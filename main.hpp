#ifndef MAIN_HPP
#define MAIN_HPP

#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// File struct
enum struct FileType { shoppingList, menu, leftover };
struct File {
  std::string path;
  // type of file
  FileType type;
  bool outOfRange;
  // pages
  int noPages;
  // date
  struct Date {
    int year;
    int month;
    int day;
  } date;

  // sort by date
  bool operator>(const File &b) const
  {
    if (date.year > b.date.year) return true;
    if (date.year == b.date.year) {
      if (date.month > b.date.month) return true;
      if (date.month == b.date.month && date.day > b.date.day) return true;
    }
    return false;
  }
  bool operator<(const File &b) const { return !(*this > b); }
  bool operator==(const File &b) const
  {
    return (date.year == b.date.year && date.month == b.date.month && date.day == b.date.day);
  }
  bool operator!=(const File &b) const { return !(*this == b); }
  bool operator<=(const File &b) const { return (*this < b || *this == b); }
  bool operator>=(const File &b) const { return (*this > b || *this == b); }
};

// setttings struct
struct Data {
  Data()
  {
    // Setup paths
    paths.DownloadedFiles = std::filesystem::path("/data/Downloads/Browser");
    paths.WorkingDir      = std::filesystem::path("/data/Downloads/Browser/menu");
    paths.TmpDir          = paths.WorkingDir / ".tmp";
    paths.Archive         = paths.WorkingDir / "archive";

    // File parsing data
    fileParsing.dateFormat    = "%m.%d";
    fileParsing.delims        = "-.";
    fileParsing.fileExtension = ".pdf";
    // 0 shoppinglist 2 menu
    searchStrings = std::array<std::string_view, 2>({ "zakupy", "menu" });
    //
    outOfDateRange = 7;
  }

  struct Paths {
    std::filesystem::path DownloadedFiles;
    std::filesystem::path WorkingDir;
    std::filesystem::path TmpDir;
    std::filesystem::path Archive;
  } paths;

  struct FileParsing {
    std::string_view dateFormat;
    std::string_view delims;
    std::string_view fileExtension;
  } fileParsing;

  enum search { shoppingList, menu };
  std::array<std::string_view, 2> searchStrings;
  int outOfDateRange;
};

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
void test();
// Date
Date getCurrentDate();
std::vector<std::string> tokenizeString(std::string string);
Date parseDate(std::string_view const &filename, bool yearChange = false, bool max = false);

std::vector<std::vector<std::filesystem::directory_entry>> findFiles(std::filesystem::path const &directoryPath,
                                                                     std::array<std::string_view, 2> const &search);
bool checkIfFileIsRecent(std::filesystem::file_time_type fileWriteTime);
std::tuple<std::filesystem::directory_entry, std::map<Date, std::filesystem::directory_entry>,
           std::vector<std::filesystem::directory_entry>>
sortFiles(std::vector<std::vector<std::filesystem::directory_entry>> filesToSort);
bool checkIfYearChange(std::filesystem::directory_entry const &entry);
int getPageCount(std::filesystem::directory_entry entry);
void processFiles(std::filesystem::directory_entry shoppingList, std::map<Date, std::filesystem::directory_entry> menu,
                  std::vector<std::filesystem::directory_entry> leftoverFiles);

// file management
std::string executeProcess(std::string exec, std::vector<std::string> args);
std::filesystem::path getBlankPage();

#endif
