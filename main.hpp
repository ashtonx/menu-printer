#ifndef MAIN_HPP
#define MAIN_HPP

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// default data
namespace defaultSettings {
constexpr std::array<std::string_view, 2> searches{ "zakupy", "menu" };

constexpr std::string_view DOWNLOAD_PATH{ "/data/Downloads/Browser" };
constexpr std::string_view WORKDIR{ "/data/Downloads/Browser/menu" }; // namespace defaultSettings

constexpr std::string_view TMP_DIR{ ".tmp" };
constexpr std::string_view ARCHIVE_DIR{ "archive" };

constexpr std::string_view FILE_EXT{ ".pdf" };
constexpr std::string_view DATE_FORMAT{ "%m.%d" };
constexpr std::string_view DELIMS{ "-." };
constexpr int OUT_OF_DATE_RANGE{ 7 };

} // namespace defaultSettings

enum FILE_FORMAT { FILE_NAME, DAY_START, MONTH_START, DAY_END, MONTH_END, FILE_EXTENSION };

struct ShoppingList {
  std::string FilePath;
  std::string MonthStart;
  std::string MonthEnd;
  std::string DayStart;
  std::string DayEnd;
  int YearStart;
  int YearEnd;
  std::filesystem::file_time_type TimeStamp;
};
// namespace date {
// enum { DAY_START, MONTH_START, YEAR_START, DAY_END, MONTH_END, YEAR_END };
// }

// functions
std::vector<std::string> getCurrentDate();
bool checkIfFileIsRecent(std::filesystem::file_time_type fileWriteTime);

std::vector<std::string> tokenizeString(std::string string);
std::string parseDate(std::string_view const &filename, bool range = false);

std::vector<std::filesystem::directory_entry> findFiles(
    std::filesystem::path const &directoryPath, std::array<std::string_view, 2> const &search,
    std::vector<std::filesystem::directory_entry> &leftoverFilesRef);
void sortFiles(std::vector<std::filesystem::directory_entry> const &files,
               std::array<std::string_view, 2> const &search,
               std::vector<std::filesystem::directory_entry> &leftoverFilesRef);

namespace debug {
bool checkIfExists(std::filesystem::path const &path, std::string function)
{
  if (!std::filesystem::exists(path)) {
    std::cerr << "ERROR::" + function + " = " << path << " does not exist\n ";
    return false;
  }
  return true;
}
} // namespace debug

#endif
