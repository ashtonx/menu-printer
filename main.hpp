#ifndef MAIN_HPP
#define MAIN_HPP

#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Date {
  int year;
  int month;
  int day;

  bool operator<(const Date &b) const
  {
    if (year < b.year) return true;
    if (year == b.year) {
      if (month < b.month) return true;
      if (month == b.month && day < b.day) return true;
    }
    return false;
  }
  bool operator==(const Date &b) const { return year == b.year && month == b.month && day == b.day; }
  bool operator>(const Date &b) const { return !(*this < b) && !(*this == b); }

  bool operator!=(const Date &b) const { return !(*this == b); }
  bool operator<=(const Date &b) const { return (*this < b || *this == b); }
  bool operator>=(const Date &b) const { return (*this > b || *this == b); }
};

// File struct
struct File {
  enum struct FileType { shoppingList = 0, menu = 1, leftover = 99 };
  std::string path;
  // type of file
  FileType type;
  // pages
  int noPages;
  // date
  struct DateRange {
    Date start;
    Date end;
  } date;
  std::filesystem::file_time_type writeTime;
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
    fileParsing.fileMask      = { { "fileType", 0 }, { "monthStart", 2 }, { "dayStart", 1 },
                             { "monthEnd", 4 }, { "dayEnd", 3 },     { "fileExtension", 5 } };
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
    std::map<std::string, int> fileMask;
    std::string_view dateFormat;
    std::string_view delims;
    std::string_view fileExtension;
  } fileParsing;

  enum search { shoppingList, menu };
  std::array<std::string_view, 2> searchStrings;
  int outOfDateRange;
};

enum FOUND_FILES { LIST, MENU, LEFTOVER };

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
void findFiles(std::vector<File> &files, Data const &settings);
bool isRecent(std::filesystem::file_time_type const &time, Data const &settings);
std::vector<size_t> sortFiles(std::vector<File> &files, Data const &settings);
void processFiles(std::vector<File> &files, std::vector<size_t> const &positions, Data const &settings);
File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool yearChange = false);
// Date
Date getCurrentDate();
std::vector<std::string> tokenizeString(std::string string, std::string_view delims);

// bool checkIfFileIsRecent(std::filesystem::file_time_type fileWriteTime);
// bool checkIfYearChange(std::filesystem::directory_entry const &entry);
int getPageCount(std::filesystem::directory_entry entry);

// file management
std::string executeProcess(std::string exec, std::vector<std::string> args);
std::filesystem::path getBlankPage(Data const &settings);

// debug
void fileStatus(const std::filesystem::path &p, std::filesystem::file_status s);
#endif
