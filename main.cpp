#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "main.hpp"

// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output

int main()
{
  // data
  std::vector<std::filesystem::directory_entry> leftoverFiles;
  std::filesystem::path const searchDir{ defaultSettings::DOWNLOAD_PATH };
  std::vector<std::string> foundFiles = findFiles(searchDir, defaultSettings::searches, leftoverFiles);

  // std::cout << "result: " << shoppingList.path().string() << '\n';
  // std::cout << "range: " << readDateFromFile(shoppingList.path().stem(),
  // true)
  // << '\n';

  // findMenuFiles(shoppingList);
  std::vector<std::string> date = getCurrentDate();
  for (std::string s : date) {
    std::cout << s << '\n';
  }
  return 0;
}

std::vector<std::string> findFiles(std::filesystem::path const &directoryPath,
                                   std::array<std::string_view, 2> const &search,
                                   std::vector<std::filesystem::directory_entry> &leftoverFilesRef)
{
  std::vector<std::string> results;

  if (!debug::checkIfExists(directoryPath, "findFiles()")) return results;

  for (std::filesystem::directory_entry const entry : std::filesystem::directory_iterator(directoryPath)) {
    if (entry.is_regular_file()) {
      if (entry.path().extension() == defaultSettings::FILE_EXT) {
        std::string fileName = entry.path().filename();
        for (auto s : search) {
          if (fileName.find(s) != std::string::npos) {
            if (checkIfFileIsRecent(entry.last_write_time())) // todo date -7
              results.push_back(fileName);
            else
              (leftoverFilesRef.push_back(entry));
          }
        }
      }
    }
  }
  return results;
}

void sortFiles(std::vector<std::string> const &files, std::array<std::string_view, 2> const &search)
{
  std::map<std::string, std::string> shoppingList;
  std::map<std::string, std::string> menu;
  std::vector<std::string> leftovers;

  for (std::string const &fileName : files) {
    if (fileName.find(search[0]) != std::string::npos) { // shoplist
      shoppingList[parseDate(fileName)] = fileName;
    } else {
      menu[parseDate(fileName)] = fileName;
    }
  }
  if (shoppingList.size() > 1) // too many files try to get right one
  {
    //
  }
}

std::vector<std::string> getCurrentDate()
{
  std::time_t t     = std::time(nullptr);
  std::tm localtime = *std::localtime(&t);
  std::stringstream buffer;
  buffer << std::put_time(&localtime, "%d.%m.%y");
  return tokenizeString(buffer.str());
}

bool checkIfFileIsRecent(std::filesystem::file_time_type fileWriteTime)
{
  // figure out how to do time
  return true;
}

std::vector<std::string> tokenizeString(std::string string)
{
  std::vector<std::string> tmp;
  size_t pos = 0;
  while ((pos = string.find_first_of(defaultSettings::DELIMS)) != std::string::npos) {
    tmp.push_back(string.substr(0, pos));
    string.erase(0, pos + 1);
  }
  tmp.push_back(string);
  return tmp;
}

std::string parseDate(std::string_view const &filename)
{
  std::vector<std::string> tokenizedString = tokenizeString(static_cast<std::string>(filename));
  std::string date{ tokenizedString[FILE_FORMAT::MONTH_START] + tokenizedString[FILE_FORMAT::DAY_START] }; // add year?
  return date;
}
