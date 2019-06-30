#include "main.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output
using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

Date const CURRENT_DATE = getCurrentDate();
int main()
{
  std::filesystem::path const searchDir{ defaultSettings::DOWNLOAD_PATH };

  std::vector<std::vector<std::filesystem::directory_entry>> files = findFiles(searchDir, defaultSettings::searches);
  sortFiles(files);

  return 0;
}

std::vector<std::vector<std::filesystem::directory_entry>> findFiles(std::filesystem::path const &directoryPath,
                                                                     std::array<std::string_view, 2> const &search)
{
  std::vector<std::vector<std::filesystem::directory_entry>> foundFiles;
  for (int i = 0; i < 3; ++i) {
    std::vector<std::filesystem::directory_entry> tmp;
    foundFiles.push_back(tmp);
  }

  if (!debug::checkIfExists(directoryPath, "findFiles()")) return foundFiles;

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

void sortFiles(std::vector<std::vector<std::filesystem::directory_entry>> filesToSort)
{
  std::filesystem::directory_entry shoppingList;
  std::map<Date, std::filesystem::directory_entry> menuFiles;
  std::vector<std::filesystem::directory_entry> leftoverFiles;
  bool yearChange = false;

  // move leftovers first
  // for (auto leftover : filesToSort[FOUND_FILES::LEFTOVER]) {
  //   leftoverFiles.push_back(leftover);
  // }

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

  // debug sort

  for (auto file : filesToSort[FOUND_FILES::LIST]) {
    std::cout << "Shopping List Files: " << file.path().filename().string() << '\n';
  }
  std::cout << "\n-----\n"
            << "Shopping List File PICKED:  " << shoppingList.path().filename().string() << '\n'
            << "-----\n";

  std::cout << "\n\n";
  for (auto file : filesToSort[FOUND_FILES::MENU]) {
    std::cout << "Menu Files: " << file.path().filename().string() << '\n';
  }
  std::cout << "\n-----\n"
            << "MENU FILES MATCHED: \n"
            << "-----\n";
  for (auto file : menuFiles) {
    std::cout << '\t' << file.second.path().filename().string() << '\n';
  }

  std::cout << "\n\n";
  for (auto file : filesToSort[FOUND_FILES::LEFTOVER]) {
    std::cout << "Leftover Files: " << file.path().filename().string() << '\n';
  }
  std::cout << "-----\n"
            << "LEFTOVER FILES AFTER SORTING:\n"
            << "-----\n";
  for (auto file : leftoverFiles) {
    std::cout << "leftover picked: " << file.path().filename().string() << '\n';
  }
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
