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
int main()
{
  std::vector<std::filesystem::directory_entry> leftoverFiles;
  std::filesystem::path const searchDir{ defaultSettings::DOWNLOAD_PATH };
  std::vector<std::filesystem::directory_entry> foundFiles =
      findFiles(searchDir, defaultSettings::searches, leftoverFiles);

  return 0;
}

std::vector<std::filesystem::directory_entry> findFiles(std::filesystem::path const &directoryPath,
                                                        std::array<std::string_view, 2> const &search,
                                                        std::vector<std::filesystem::directory_entry> &leftoverFilesRef)
{
  std::vector<std::filesystem::directory_entry> results;

  if (!debug::checkIfExists(directoryPath, "findFiles()")) return results;

  for (std::filesystem::directory_entry const entry : std::filesystem::directory_iterator(directoryPath)) {
    if (entry.is_regular_file()) {
      if (entry.path().extension() == defaultSettings::FILE_EXT) {
        std::string fileName = entry.path().filename();
        for (auto s : search) {
          if (fileName.find(s) != std::string::npos) {
            if (checkIfFileIsRecent(entry.last_write_time())) {
              std::cerr << "file is recent " << entry.path().filename().string() << '\n';
              results.push_back(entry);
            } else {
              std::cerr << "file is old " << entry.path().filename().string() << '\n';
              (leftoverFilesRef.push_back(entry));
            }
          }
        }
      }
    }
  }
  return results;
}

void sortFiles(std::vector<std::filesystem::directory_entry> const &files,
               std::array<std::string_view, 2> const &search,
               std::vector<std::filesystem::directory_entry> &leftoverFilesRef)
{
  // todo: consider directory entries for date comparison between files

  std::vector<std::string> const date = getCurrentDate();
  std::vector<ShoppingList> shoppingList;
  std::map<std::string, std::string> menu;
  std::vector<std::string> leftovers;
  bool yearChange = false;

  for (std::filesystem::directory_entry entry : files) {
    std::string fileName               = entry.path().filename();
    std::vector<std::string> tokenized = tokenizeString(fileName);
    if (fileName.find(defaultSettings::searches[0]) != std::string::npos) {
      // Shopping List found
      ShoppingList shop;
      shop.FilePath   = entry.path();
      shop.TimeStamp  = entry.last_write_time();
      shop.MonthStart = tokenized[FILE_FORMAT::MONTH_START];
      shop.DayStart   = tokenized[FILE_FORMAT::DAY_START];
      shop.MonthEnd   = tokenized[FILE_FORMAT::MONTH_END];
      shop.DayEnd     = tokenized[FILE_FORMAT::DAY_END];
      shop.YearEnd = shop.YearEnd = std::stoi(date[2]);
      if (shop.MonthStart == "12" && shop.MonthEnd == "01") ++shop.YearEnd;
      shoppingList.push_back(shop);
    } else {
      // todo: year issue persists... need to eliminate dupes before sorting shopping files..
    }
  }
  if (shoppingList.size() > 1) // too many files try to get right one
  {
    // later
  }
}

std::vector<std::string> getCurrentDate()
{
  std::time_t t     = std::time(nullptr);
  std::tm localtime = *std::localtime(&t);
  std::stringstream buffer;
  buffer << std::put_time(&localtime, "%d.%m.%Y");
  return tokenizeString(buffer.str());
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

std::string parseDate(std::string_view const &filename, bool range)
{
  std::vector<std::string> tokenizedString = tokenizeString(static_cast<std::string>(filename));
  std::string date{ tokenizedString[FILE_FORMAT::MONTH_START] + tokenizedString[FILE_FORMAT::DAY_START] };
  return date;
}
