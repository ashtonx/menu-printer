#include "main.hpp"
#include <boost/process.hpp>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output
using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

Date const CURRENT_DATE = getCurrentDate();
int main()
{
  test();
  namespace fs = std::filesystem;
  std::vector<std::vector<fs::directory_entry>> files =
      findFiles(defaultSettings::DOWNLOAD_PATH, defaultSettings::searches);
  fs::directory_entry shoppingList;
  std::map<Date, fs::directory_entry> menuFiles;
  std::vector<fs::directory_entry> leftoverFiles;
  std::tie(shoppingList, menuFiles, leftoverFiles) = sortFiles(files);
  processFiles(shoppingList, menuFiles, leftoverFiles);
  return 0;
}

void test()
{
  // for rewriting code
}

std::vector<std::vector<std::filesystem::directory_entry>> findFiles(std::filesystem::path const &directoryPath,
                                                                     std::array<std::string_view, 2> const &search)
{
  std::vector<std::vector<std::filesystem::directory_entry>> foundFiles;
  for (int i = 0; i < 3; ++i) {
    std::vector<std::filesystem::directory_entry> tmp;
    foundFiles.push_back(tmp);
  }
  assert(std::filesystem::exists(directoryPath));

  // find all files that mach search strings
  for (std::filesystem::directory_entry const entry : std::filesystem::directory_iterator(directoryPath)) {
    if (entry.is_regular_file() && entry.path().extension() == defaultSettings::FILE_EXT) {
      for (int i = 0; i < search.size(); ++i) {
        if (entry.path().filename().string().find(search[i]) != std::string::npos) { // 0 shopping list
          if (!checkIfFileIsRecent(entry.last_write_time())) {                       // if file is older than week
            (foundFiles[FOUND_FILES::LEFTOVER].push_back(entry));
          } else {
            if (i == 0) { // 0 == shopping list file
              foundFiles[FOUND_FILES::LIST].push_back(entry);
            } else { // menu files
              foundFiles[FOUND_FILES::MENU].push_back(entry);
            }
          }
        }
      }
    }
  }
  return foundFiles;
}

std::tuple<std::filesystem::directory_entry, std::map<Date, std::filesystem::directory_entry>,
           std::vector<std::filesystem::directory_entry>>
sortFiles(std::vector<std::vector<std::filesystem::directory_entry>> filesToSort)
{
  namespace fs = std::filesystem;

  fs::directory_entry shoppingList;
  std::map<Date, fs::directory_entry> menuFiles;
  std::vector<fs::directory_entry> leftoverFiles;
  bool yearChange = false;

  // move leftovers first
  for (auto leftover : filesToSort[FOUND_FILES::LEFTOVER]) {
    leftoverFiles.push_back(leftover);
  }

  // find menu;
  // todo optimize ifs
  shoppingList = filesToSort[0][0];
  for (auto entry : filesToSort[0]) {
    if (entry.last_write_time() > shoppingList.last_write_time()) {
      leftoverFiles.push_back(shoppingList);
      shoppingList = entry;
    } else if (entry.last_write_time() < shoppingList.last_write_time()) {
      leftoverFiles.push_back(entry);
    }
  }

  yearChange = checkIfYearChange(shoppingList);

  Date max = parseDate(shoppingList.path().filename().string(), yearChange, true);
  Date min = parseDate(shoppingList.path().filename().string(), yearChange, false);

  for (auto entry : filesToSort[FOUND_FILES::MENU]) {
    Date tmpDate = parseDate(entry.path().filename().string(), yearChange, false);

    if (tmpDate >= min || tmpDate <= max) {
      if (menuFiles.count(tmpDate) == 0) {
        menuFiles[tmpDate] = entry;
      } else {
        if (menuFiles[tmpDate].last_write_time() < entry.last_write_time()) {
          leftoverFiles.push_back(menuFiles[tmpDate]);
          menuFiles[tmpDate] = entry;
        } else {
          leftoverFiles.push_back(entry);
        }
      }
    } else
      leftoverFiles.push_back(entry);
  }
  return std::make_tuple(shoppingList, menuFiles, leftoverFiles);
}

Date getCurrentDate()
{
  std::time_t t     = std::time(nullptr);
  std::tm localtime = *std::localtime(&t);
  std::stringstream buffer;
  buffer << std::put_time(&localtime, "%d.%m.%Y");
  std::vector<std::string> str = tokenizeString(buffer.str());
  return Date{ std::stoi(str[2]), std::stoi(str[1]), std::stoi(str[0]) };
}

bool checkIfFileIsRecent(std::filesystem::file_time_type fileWriteTime)
{
  // todo: figure out how to convert file_time_type clock to normal date and compare days rather than hours
  // also move current date to some const variable
  std::filesystem::file_time_type timeRange =
      std::filesystem::file_time_type::clock::now() - std::chrono::hours(7 * 24);
  if (timeRange > fileWriteTime) return false;
  return true;
}

bool checkIfYearChange(std::filesystem::directory_entry const &entry)
{
  std::vector<std::string> tok = tokenizeString(entry.path().filename());
  return (tok[FILE_FORMAT::MONTH_START] == "12" && tok[FILE_FORMAT::MONTH_END] == "01");
}

std::vector<std::string> tokenizeString(std::string string)
{
  // todo: consider (1) appended to files when dupe
  std::vector<std::string> tmp;
  size_t pos = 0;
  while ((pos = string.find_first_of(defaultSettings::DELIMS)) != std::string::npos) {
    tmp.push_back(string.substr(0, pos));
    string.erase(0, pos + 1);
  }
  tmp.push_back(string);
  return tmp;
}

Date parseDate(std::string_view const &filename, bool yearChange, bool max)
{
  Date result;
  std::vector<std::string> tok = tokenizeString(static_cast<std::string>(filename));
  int month                    = max ? FILE_FORMAT::MONTH_END : FILE_FORMAT::MONTH_START;
  int day                      = max ? FILE_FORMAT::DAY_END : FILE_FORMAT::DAY_START;
  result.month                 = std::stoi(tok[month]);
  result.day                   = std::stoi(tok[day]);
  result.year                  = (result.month == 1 && yearChange) ? CURRENT_DATE.year + 1 : CURRENT_DATE.year;
  return result;
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
