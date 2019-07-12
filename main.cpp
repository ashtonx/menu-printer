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
  sortFiles(files, settings);

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
        if (entry.path().filename().string() == settings.searchStrings[i]) {
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

void sortFiles(std::vector<File> &files, Data const &settings)
{
  std::size_t shoppingListPos;
  std::map<Date, size_t> menuPos;

  // get shopping list
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].type == File::FileType::shoppingList) {
      if (shoppingListPos) {
        if (files[shoppingListPos].date.start < files[i].date.start) {
          files[shoppingListPos].type = File::FileType::leftover;
          shoppingListPos             = i;
        }
      } else {
        shoppingListPos = i;
      }
    }
  }
  files[shoppingListPos].date =
      parseDate(std::filesystem::directory_entry(files[shoppingListPos].path).path().filename().string(), settings);
  bool yearChange = (files[shoppingListPos].date.start.month == 12 && files[shoppingListPos].date.end.month == 1);

  // Get menu files
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].type == File::FileType::menu) {
      files[i].date =
          parseDate(std::filesystem::directory_entry(files[i].path).path().filename().string(), settings, yearChange);
      Date date = files[i].date.start;
      // check if in shopping list range
      if (files[shoppingListPos].date.start > date || files[shoppingListPos].date.end < date) {
        files[i].type = File::FileType::leftover;
      } else {
        if (!menuPos[date]) {
          menuPos[date] = i;
        } else {
          if (files[menuPos[date]].writeTime > files[i].writeTime) {
            files[i].type = File::FileType::leftover;
          } else {
            files[menuPos[date]].type = File::FileType::leftover;
            menuPos[date]             = i;
          }
        }
      }
    }
  }
}

File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool yearChange)
{
  File::DateRange result;

  std::vector<std::string> tok = tokenizeString(static_cast<std::string>(filename), settings.fileParsing.delims);

  // todo fix const shit
  result.start.day   = std::stoi(tok[settings.fileParsing.fileMask["dayStart"].second]);
  result.end.day     = std::stoi(tok[settings.fileParsing.fileMask["dayEnd"].second]);
  result.start.month = std::stoi(tok[settings.fileParsing.fileMask["monthStart"].second]);
  result.end.month   = std::stoi(tok[settings.fileParsing.fileMask["monthEnd"].second]);
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
  while ((pos = string.find_first_of(delims) != std::string::npos) {
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
  std::cout << "execProcess output: " << result;
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

std::filesystem::path getBlankPage()
{
  namespace fs = std::filesystem;

  fs::path workdir{ defaultSettings::WORKDIR };
  fs::path result = workdir / "blank.pdf";
  if (fs::exists(result)) return result;
  // echo showpage | ps2pdf -sPAPERSIZE=letter - blank.pdf blank pdf
  // figure out later
  assert(fs::exists(result));
  return result;
}

void processFiles(std::filesystem::directory_entry shoppingList, std::map<Date, std::filesystem::directory_entry> menu,
                  std::vector<std::filesystem::directory_entry> leftoverFiles)
{
  namespace fs     = std::filesystem;
  fs::path workDir = fs::path(defaultSettings::WORKDIR);
  assert(fs::exists(workDir));

  fs::path tmpDir = workDir / defaultSettings::TMP_DIR;
  if (!fs::exists(tmpDir)) fs::create_directory(tmpDir);
  assert(fs::exists(tmpDir));
  fs::path archiveDir = workDir / defaultSettings::ARCHIVE_DIR;
  assert(fs::exists(archiveDir));

  // clean up tmpdir
  for (fs::directory_entry entry : fs::directory_iterator(tmpDir)) {
    fs::remove_all(entry);
  }
  // copy files
  for (auto file : menu) {
    fs::copy(file.second, tmpDir);
  }

  fs::path processed = tmpDir / "processed";
  fs::path blankPage = getBlankPage();
  fs::create_directory(processed);
  for (fs::directory_entry entry : fs::directory_iterator(tmpDir)) {
    if (fs::is_regular_file(entry)) {
      if (getPageCount(entry) == 2)
        fs::rename(entry, processed / entry.path().filename());
      else {
        std::vector<std::string> args;
        args.push_back(entry.path().string());
        args.push_back(blankPage.string());
        args.push_back(processed.string() + '/' + entry.path().filename().string());
        std::cout << executeProcess("pdfunite", args) << '\n';
        fs::remove(entry);
      }
    }
  }

  // merging files
  std::vector<std::string> filesToMerge;
  for (fs::directory_entry entry : fs::directory_iterator(processed)) {
    filesToMerge.push_back(entry.path().string());
  }
  Date min                  = parseDate(shoppingList.path().filename().string(), false);
  Date max                  = parseDate(shoppingList.path().filename().string(), true);
  std::string finalFileName = "Menu-" + std::to_string(min.month) + '.' + std::to_string(min.day) + "-" +
                              std::to_string(max.month) + '.' + std::to_string(max.day) + ".pdf";
  std::string combinedPath = tmpDir.string() + '/' + finalFileName;
  filesToMerge.push_back(combinedPath);
  executeProcess("pdfunite", filesToMerge);
  // cropping
  std::vector<std::string> cropArgs;
  cropArgs.push_back(combinedPath);
  cropArgs.push_back(combinedPath + "-cropped.pdf");
  executeProcess("pdfcrop", cropArgs);
}
// prepareSettings
