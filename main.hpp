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
  enum struct FileType { shopping_list = 0, menu = 1, leftover = 99 };
  std::string path;
  // type of file
  FileType type;
  // pages
  int no_of_pages;
  // date
  struct DateRange {
    Date start;
    Date end;
  } date;
  std::filesystem::file_time_type write_time;
};

// setttings struct
struct Data {
  Data()
  {
    // Setup paths
    paths.files_to_sort = std::filesystem::path("/data/Downloads/Browser");
    paths.working_directory      = std::filesystem::path("/data/Downloads/Browser/menu");
    paths.tmp_dir          = paths.working_directory / ".tmp";
    paths.archive_dir         = paths.working_directory / "archive";

    // File parsing data
    file_parsing.file_mask      = { { "file_type", 0 }, { "month_start", 2 }, { "day_start", 1 },
                             { "month_end", 4 }, { "day_end", 3 },     { "file_extension", 5 } };
    file_parsing.delims        = "-.";
    file_parsing.file_extension = ".pdf";
    // 0 shoppinglist 2 menu
    search_strings = std::array<std::string_view, 2>({ "zakupy", "menu" });
    //
    date_range = 7;
  }

  struct Paths {
    std::filesystem::path files_to_sort;
    std::filesystem::path working_directory;
    std::filesystem::path tmp_dir;
    std::filesystem::path archive_dir;
  } paths;

  struct FileParsing {
    std::map<std::string, int> file_mask;
    std::string_view delims;
    std::string_view file_extension;
  } file_parsing;

  enum Search { shopping_list, menu };
  std::array<std::string_view, 2> search_strings;
  int date_range;
};

void FindFiles(std::vector<File> &files, Data const &settings);
bool isRecent(std::filesystem::file_time_type const &time, Data const &settings);
std::vector<size_t> sortFiles(std::vector<File> &files, Data const &settings);
void processFiles(std::vector<File> &files, std::vector<size_t> const &positions, Data const &settings);
File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool year_change = false);
Date getCurrentDate();
std::vector<std::string> tokenizeString(std::string string, std::string_view delims);
int getPageCount(std::filesystem::directory_entry entry);
std::string executeProcess(std::string const &exec, std::vector<std::string> const &args);
std::filesystem::path getBlankPage(Data const &settings);

#endif
