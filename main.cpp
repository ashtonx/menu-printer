#include "main.hpp"
#include <boost/process.hpp>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output
using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

Date const CURRENT_DATE = getCurrentDate();
int main()
{
  test();
  return 0;
}

void test()
{
  // init settings
  Data settings;
  // main data
  std::vector<File> files;
  findFiles(files, settings);
  std::vector<size_t> sortedFilesPos = sortFiles(files, settings);
  // for (auto a : sortedFilesPos) << std::cerr << a.first() << '\t' << a.second() << '\n';
  processFiles(files, sortedFilesPos, settings);
  // void sortFiles(&vector<File> &settings)

  // for rewriting code
  // find files
  // File files = findFiles()
}

void findFiles(std::vector<File> &files, Data const &settings)
{
  namespace fs = std::filesystem;
  assert(fs::exists(settings.paths.DownloadedFiles));
  for (fs::directory_entry const entry : fs::directory_iterator(settings.paths.DownloadedFiles)) {
    if (entry.is_regular_file() && entry.path().extension() == settings.fileParsing.fileExtension) {
      for (int i = 0; i < settings.searchStrings.size(); ++i) {
        if (entry.path().filename().string().find(settings.searchStrings[i]) != std::string::npos) {
          File newFile;
          newFile.path    = entry.path().string();
          newFile.type    = isRecent(entry.last_write_time(), settings) ? File::FileType(i) : File::FileType::leftover;
          newFile.noPages = getPageCount(entry);
          newFile.writeTime = entry.last_write_time();

          files.push_back(newFile); // make parse date return File::Date
        }
      }
    }
  }
}

std::vector<size_t> sortFiles(std::vector<File> &files, Data const &settings)
{
  int shoppingListPos = -1;
  std::map<Date, size_t> menuPos;
  // get shopping list
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].type == File::FileType::shoppingList) {
      if (shoppingListPos >= 0) {
        if (files[shoppingListPos].date.start < files[i].date.start) {
          files[shoppingListPos].type = File::FileType::leftover;
          shoppingListPos             = i;
        }
      } else {
        shoppingListPos = i;
      }
    }
  }
  std::cerr << "shopping list debug: " << files[shoppingListPos].path << '\n';
  files[shoppingListPos].date =
      parseDate(std::filesystem::path(files[shoppingListPos].path).filename().string(), settings);
  bool yearChange = (files[shoppingListPos].date.start.month == 12 && files[shoppingListPos].date.end.month == 1);

  // Get menu files
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].type == File::FileType::menu) {
      files[i].date = parseDate(std::filesystem::path(files[i].path).filename().string(), settings, yearChange);
      Date date     = files[i].date.start;
      // check if in shopping list range
      if (files[shoppingListPos].date.start > date || files[shoppingListPos].date.end < date) {
        files[i].type = File::FileType::leftover;
      } else {
        if (!menuPos.count(date)) {
          // std::cerr << "!menuPos.count(date)\n";
          menuPos[date] = i;
        } else {
          if (files[menuPos[date]].writeTime > files[i].writeTime) {
            files[i].type = File::FileType::leftover;
          } else {
            files[menuPos[date]].type = File::FileType::leftover;
            menuPos[date]             = i;
            std::cerr << "menupos[date] overwritten\n";
          }
        }
      }
    }
  }

  // send back sorted results
  std::vector<size_t> result;
  result.push_back(shoppingListPos);
  std::cerr << "menupos size: " << menuPos.size() << "\n";
  for (auto entry : menuPos) {
    std::cerr << "menupos debug (" << std::to_string(entry.second) << ")["
              << "Y:" << std::to_string(entry.first.year) << " M:" << std::to_string(entry.first.month)
              << " D:" << std::to_string(entry.first.day) << "]: " << files[entry.second].path << '\n';
    result.push_back(entry.second);
  }
  return result;
}

void processFiles(std::vector<File> &files, std::vector<size_t> const &positions, Data const &settings)
{
  std::vector<File> processed;
  for (auto pos : positions) {
    processed.push_back(files[pos]);
  }
  // check directories and make if they're needed
  assert(std::filesystem::exists(settings.paths.WorkingDir)); // working dir check
  std::filesystem::remove_all(settings.paths.TmpDir);
  std::filesystem::create_directory(settings.paths.TmpDir);
  std::filesystem::path tmp = settings.paths.TmpDir;

  std::filesystem::path processing = settings.paths.TmpDir / "processing";
  std::filesystem::create_directory(processing);

  for (auto file : processed) {
    // std::cerr << "copying filepath:" << files[pos].path << '\n';

    std::filesystem::path filePath{ file.path };

    if (file.noPages == 2) {
      std::filesystem::copy_file(filePath, (tmp / filePath.filename()));
      file.path = (tmp / filePath.filename()).string();
    } else {
      std::filesystem::copy_file(filePath, (processing / filePath.filename()));
      std::filesystem::path blankPage = getBlankPage(settings);
      std::string output              = (settings.paths.TmpDir / filePath.filename()).string();
      std::vector<std::string> args{ blankPage.string(), filePath.string(), // merge with blank page
                                     output };                              // save to tmpDir
      executeProcess("pdfunite", args);
      file.path = (output);
    }
  }
  std::filesystem::remove_all(processing);

  // merge all files
  std::vector<std::string> executionPaths;
  for (File file : processed) {
    executionPaths.push_back(file.path);
  }
  std::string mergedOut = (settings.paths.TmpDir / "merged.pdf").string();
  executionPaths.push_back(mergedOut);
  executeProcess("pdfunite", executionPaths);
  // crop
  executionPaths.clear();
  executionPaths.push_back(mergedOut);
  executionPaths.push_back((settings.paths.WorkingDir / "finished.pdf").string());
  executeProcess("pdfcrop", executionPaths);

  // cleanup
  // std::filesystem::remove_all(settings.paths.TmpDir);
  // for (auto file : files) {
  //   std::filesystem::path(file.path);
  //   std::filesystem::remove(file.path);
  // }
}
File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool yearChange)
{
  File::DateRange result;

  std::vector<std::string> tok = tokenizeString(static_cast<std::string>(filename), settings.fileParsing.delims);

  // std::cerr << "parseDate Debug: \n";
  // for (auto str : tok) std::cerr << str << " ";
  // std::cerr << '\n';
  // std::cerr << "settings.fileParsing.fileMask.at(dayStart): " << tok[settings.fileParsing.fileMask.at("dayStart")]
  //           << "\n";
  // std::cerr << "settings.fileParsing.fileMask.at(dayEnd): " << tok[settings.fileParsing.fileMask.at("dayEnd")] <<
  // "\n"; std::cerr << "settings.fileParsing.fileMask.at(monthStart): " <<
  // tok[settings.fileParsing.fileMask.at("monthStart")]
  //           << "\n";
  // std::cerr << "settings.fileParsing.fileMask.at(monthEnd): " << tok[settings.fileParsing.fileMask.at("monthEnd")]
  //           << "\n";
  // todo fix const shit
  result.start.day   = std::stoi(tok[settings.fileParsing.fileMask.at("dayStart")]);
  result.end.day     = std::stoi(tok[settings.fileParsing.fileMask.at("dayEnd")]);
  result.start.month = std::stoi(tok[settings.fileParsing.fileMask.at("monthStart")]);
  result.end.month   = std::stoi(tok[settings.fileParsing.fileMask.at("monthEnd")]);
  result.start.year = result.end.year = CURRENT_DATE.year;

  // check for year change
  if (yearChange) {
    if (result.start.month == 1) result.start.year++;
    if (result.end.month == 1) result.end.year++;
  }

  return result;
}

// helpers
Date getCurrentDate()
{
  std::time_t t     = std::time(nullptr);
  std::tm localtime = *std::localtime(&t);
  std::stringstream buffer;
  buffer << std::put_time(&localtime, "%d.%m.%Y");
  std::vector<std::string> str = tokenizeString(buffer.str(), ".");
  for (auto s : str) {
    std::cerr << s << '\n';
  }
  return Date{ std::stoi(str[2]), std::stoi(str[1]), std::stoi(str[0]) };
}

bool isRecent(std::filesystem::file_time_type const &time, Data const &settings)
{
  // todo: figure out how to convert file_time_type clock to normal date and compare days rather than hours
  // also move current date to some const variable

  std::filesystem::file_time_type range =
      std::filesystem::file_time_type::clock::now() - std::chrono::hours(settings.outOfDateRange * 24);
  if (time > range) return true;
  return false;
}

std::vector<std::string> tokenizeString(std::string string, std::string_view delims)
{
  // todo: consider (1) appended to files when dupe
  std::vector<std::string> tmp;
  size_t pos = 0;
  while ((pos = string.find_first_of(delims)) != std::string::npos) {
    tmp.push_back(string.substr(0, pos));
    string.erase(0, pos + 1);
  }
  tmp.push_back(string);
  return tmp;
}

// File operations
std::string executeProcess(std::string exec, std::vector<std::string> args)
{
  std::string command{ exec };
  for (auto arg : args) {
    command += (" " + arg);
  }

  std::cout << "executeProcess() command: " << command << '\n';
  boost::process::ipstream pipe_stream;
  boost::process::child c(command, boost::process::std_out > pipe_stream);
  c.wait();
  std::string line;
  std::string result;
  while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
    result += line + '\n';
  }
  // std::cout << "execProcess output: " << result;
  c.exit_code();

  return result;
}

int getPageCount(std::filesystem::directory_entry entry)
{
  std::vector<std::string> args;
  args.push_back(entry.path().string());
  // args.push_back(" | grep Pages:");
  std::string output = executeProcess("pdfinfo", args);
  std::smatch match;
  // std::regex pages(".+Pages.+(\\d).+");
  if (std::regex_search(output, match, std::regex("Pages.+(\\d)"))) {
    return std::stoi(match[1]);
  }

  return -1;
}

std::filesystem::path getBlankPage(Data const &settings)
{
  return settings.paths.WorkingDir / "blank.pdf";
}

void fileStatus(const std::filesystem::path &p, std::filesystem::file_status s)
{
  namespace fs = std::filesystem;
  std::cout << p;
  // alternative: switch(s.type()) { case fs::file_type::regular: ...}
  if (fs::is_regular_file(s)) std::cout << " is a regular file\n";
  if (fs::is_directory(s)) std::cout << " is a directory\n";
  if (fs::is_block_file(s)) std::cout << " is a block device\n";
  if (fs::is_character_file(s)) std::cout << " is a character device\n";
  if (fs::is_fifo(s)) std::cout << " is a named IPC pipe\n";
  if (fs::is_socket(s)) std::cout << " is a named IPC socket\n";
  if (fs::is_symlink(s)) std::cout << " is a symlink\n";
  if (!fs::exists(s)) std::cout << " does not exist\n";
}
