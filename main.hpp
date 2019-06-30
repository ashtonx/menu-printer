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
enum FOUND_FILES { LIST, MENU, LEFTOVER };
struct Date {
  int year;
  int month;
  int day;

  bool operator>(const Date &b) const
  {
    if (year > b.year) return true;
    if (year == b.year) {
      if (month > b.month) return true;
      if (month == b.month && day > b.day) return true;
    }
    return false;
  }
  bool operator<(const Date &b) const { return !(*this > b); }
  bool operator==(const Date &b) const { return (year == b.year && month == b.month && day == b.day); }
  bool operator!=(const Date &b) const { return !(*this == b); }
  bool operator<=(const Date &b) const { return (*this < b || *this == b); }
  bool operator>=(const Date &b) const { return (*this > b || *this == b); }
};

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
Date getCurrentDate();
bool checkIfFileIsRecent(std::filesystem::file_time_type fileWriteTime);
bool checkIfYearChange(std::filesystem::directory_entry const &entry);
std::vector<std::string> tokenizeString(std::string string);
Date parseDate(std::string_view const &filename, bool yearChange = false, bool max = false);
std::vector<std::vector<std::filesystem::directory_entry>> findFiles(std::filesystem::path const &directoryPath,
                                                                     std::array<std::string_view, 2> const &search);
void sortFiles(std::vector<std::vector<std::filesystem::directory_entry>> filesToSort);

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
