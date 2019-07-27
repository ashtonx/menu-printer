#ifndef DATA_STRUCTS_HPP
#define DATA_STRUCTS_HPP

#include <filesystem>
#include <map>
#include <string>

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

struct Data {
  struct Paths {
    std::filesystem::path files_to_sort;
    std::filesystem::path working_directory;
    std::filesystem::path tmp_dir;
    std::filesystem::path archive_dir;
  } paths;

  enum Search { shopping_list, menu };
  struct FileParsing {
    std::map<std::string, int> file_mask;
    std::string delims;
    std::string file_extension;
    std::array<std::string, 2> search_strings;
  } file_parsing;

  int date_range;
  Date current_date;
};

#endif
