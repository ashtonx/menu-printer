#ifndef MAIN_HPP
#define MAIN_HPP

#include <filesystem>
// #include <iostream>
#include <map>
#include <string>
// #include <unordered_map>
// #include <utility>
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
  struct Paths {
    std::filesystem::path files_to_sort;
    std::filesystem::path working_directory;
    std::filesystem::path tmp_dir;
    std::filesystem::path archive_dir;
  } paths;

  struct FileParsing {
    std::map<std::string, int> file_mask;
    std::string delims;
    std::string file_extension;
  } file_parsing;

  enum Search { shopping_list, menu };
  std::array<std::string, 2> search_strings;
  int date_range;
};

void findFiles(std::vector<File> &files, Data const &settings);
bool isRecent(std::filesystem::file_time_type const &time, Data const &settings);
std::vector<size_t> sortFiles(std::vector<File> &files, Data const &settings);
void processFiles(std::vector<File> &files, std::vector<size_t> const &positions, Data const &settings);
File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool year_change = false);
Date getCurrentDate();
std::vector<std::string> tokenizeString(std::string string, std::string_view delims);
int getPageCount(std::filesystem::directory_entry entry);
std::string executeProcess(std::string const &exec, std::vector<std::string> const &args);
std::filesystem::path getBlankPage(Data const &settings);

void DebugConfig(Data config);
#endif
