#include "main.hpp"
#include <boost/process.hpp>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output

using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

Date const CURRENT_DATE = getCurrentDate();
int main()
{
  // init settings
  Data settings;
  // main data
  std::vector<File> files;
  findFiles(files, settings);
  std::vector<size_t> sorted_files_pos = sortFiles(files, settings);
  processFiles(files, sorted_files_pos, settings);

  return 0;
}

void findFiles(std::vector<File> &files, Data const &settings)
{
  namespace fs = std::filesystem;
  assert(fs::exists(settings.paths.files_to_sort));
  for (auto entry : fs::directory_iterator(settings.paths.files_to_sort)) {
    if (entry.is_regular_file() && entry.path().extension() == settings.file_parsing.file_extension) {
      for (int i = 0; i < settings.search_strings.size(); ++i) {
        if (entry.path().filename().string().find(settings.search_strings[i]) != std::string::npos) {
          File new_file;
          new_file.path    = entry.path().string();
          new_file.type    = isRecent(entry.last_write_time(), settings) ? File::FileType(i) : File::FileType::leftover;
          new_file.no_of_pages = getPageCount(entry);
          new_file.write_time = entry.last_write_time();

          files.push_back(new_file); // make parse date return File::Date
        }
      }
    }
  }
}

std::vector<size_t> sortFiles(std::vector<File> &files, Data const &settings)
{
  int shopping_list_pos = -1;
  std::map<Date, size_t> menu_pos;
  // get shopping list
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].type == File::FileType::shopping_list) {
      if (shopping_list_pos >= 0) {
        if (files[shopping_list_pos].date.start < files[i].date.start) {
          files[shopping_list_pos].type = File::FileType::leftover;
          shopping_list_pos             = i;
        }
      } else {
        shopping_list_pos = i;
      }
    }
  }
  std::cerr << "shopping list debug: " << files[shopping_list_pos].path << '\n';
  files[shopping_list_pos].date =
      parseDate(std::filesystem::path(files[shopping_list_pos].path).filename().string(), settings);
  bool year_change = (files[shopping_list_pos].date.start.month == 12 && files[shopping_list_pos].date.end.month == 1);

  // Get menu files
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].type == File::FileType::menu) {
      files[i].date = parseDate(std::filesystem::path(files[i].path).filename().string(), settings, year_change);
      Date date     = files[i].date.start;
      // check if in shopping list range
      if (files[shopping_list_pos].date.start > date || files[shopping_list_pos].date.end < date) {
        files[i].type = File::FileType::leftover;
      } else {
        if (!menu_pos.count(date)) {
          menu_pos[date] = i;
        } else {
          if (files[menu_pos[date]].write_time > files[i].write_time) {
            files[i].type = File::FileType::leftover;
          } else {
            files[menu_pos[date]].type = File::FileType::leftover;
            menu_pos[date]             = i;
          }
        }
      }
    }
  }

  // send back sorted results
  std::vector<size_t> result;
  result.push_back(shopping_list_pos);
  for (auto entry : menu_pos) {
    result.push_back(entry.second);
  }
  return result;
}

void processFiles(std::vector<File> &files, std::vector<size_t> const &positions, Data const &settings)
{
  std::vector<File> processing;
  for (auto pos : positions) {
    processing.push_back(files[pos]);
  }
  // check directories and make if they're needed
  assert(std::filesystem::exists(settings.paths.working_directory)); // working dir check
  std::filesystem::remove_all(settings.paths.tmp_dir);
  std::filesystem::create_directory(settings.paths.tmp_dir);
  std::filesystem::path tmp_dir = settings.paths.tmp_dir;

  std::filesystem::path processing_dir = settings.paths.tmp_dir / "processing";
  std::filesystem::create_directory(processing_dir);

  for (auto file : processing) {
    std::filesystem::path file_path{ file.path }; //string to path

    if (file.no_of_pages == 2) {
      std::filesystem::copy_file(file_path, (tmp_dir / file_path.filename()));
      file.path = (tmp_dir / file_path.filename()).string();
    } else {
      std::filesystem::copy_file(file_path, (processing_dir / file_path.filename()));
      std::filesystem::path blank_page = getBlankPage(settings);
      std::string output              = (settings.paths.tmp_dir / file_path.filename()).string();
      std::vector<std::string> args{ blank_page.string(), file_path.string(), // merge with blank page
                                     output };                              // save to tmpDir
      executeProcess("pdfunite", args);
      file.path = (output);
    }
  }
  std::filesystem::remove_all(processing_dir);

  // merge all files
  std::vector<std::string> executionPaths;
  for (File file : processing) {
    executionPaths.push_back(file.path);
  }
  std::string mergedOut = (settings.paths.tmp_dir / "merged.pdf").string();
  executionPaths.push_back(mergedOut);
  executeProcess("pdfunite", executionPaths);
  // crop
  executionPaths.clear();
  executionPaths.push_back(mergedOut);
  executionPaths.push_back((settings.paths.working_directory / "finished.pdf").string());
  executeProcess("pdfcrop", executionPaths);

  // cleanup
  // std::filesystem::remove_all(settings.paths.TmpDir);
  // for (auto file : files) {
  //   std::filesystem::path(file.path);
  //   std::filesystem::remove(file.path);
  // }
}
File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool year_change)
{
  File::DateRange result;

  std::vector<std::string> tok = tokenizeString(static_cast<std::string>(filename), settings.file_parsing.delims);
  result.start.day             = std::stoi(tok[settings.file_parsing.file_mask.at("day_start")]);
  result.end.day               = std::stoi(tok[settings.file_parsing.file_mask.at("day_end")]);
  result.start.month           = std::stoi(tok[settings.file_parsing.file_mask.at("month_start")]);
  result.end.month             = std::stoi(tok[settings.file_parsing.file_mask.at("month_end")]);
  result.start.year = result.end.year = CURRENT_DATE.year;

  // check for year change
  if (year_change) {
    if (result.start.month == 1) result.start.year++;
    if (result.end.month == 1) result.end.year++;
  }

  return result;
}

Date getCurrentDate()
{
  std::time_t t     = std::time(nullptr);
  std::tm localtime = *std::localtime(&t);
  std::stringstream buffer;
  buffer << std::put_time(&localtime, "%d.%m.%Y");
  std::vector<std::string> str = tokenizeString(buffer.str(), ".");
  return Date{ std::stoi(str[2]), std::stoi(str[1]), std::stoi(str[0]) };
}

bool isRecent(std::filesystem::file_time_type const &time, Data const &settings)
{
  std::filesystem::file_time_type range =
      std::filesystem::file_time_type::clock::now() - std::chrono::hours(settings.date_range * 24);
  if (time > range) return true;
  return false;
}

std::vector<std::string> tokenizeString(std::string string, std::string_view delims)
{
  // todo: consider (1) appended to files when dupe
  std::vector<std::string> tmp;
  size_t pos = 0;
  while ((pos = string.find_first_of(delims)) != std::string::npos) {
    tmp.push_back(string.substr(0, pos));
    string.erase(0, pos + 1);
  }
  tmp.push_back(string);
  return tmp;
}

// File operations
std::string executeProcess(std::string const &exec, std::vector<std::string> const &args)
{
  std::string command{ exec };
  for (std::string arg : args) {
    command += (" " + arg);
  }
  boost::process::ipstream pipe_stream;
  boost::process::child c(command, boost::process::std_out > pipe_stream);
  c.wait();
  std::string line;
  std::string result;
  while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
    result += line + '\n';
  }
  c.exit_code();

  return result;
}

int getPageCount(std::filesystem::directory_entry entry)
{
  std::vector<std::string> args;
  args.push_back(entry.path().string());
  std::string output = executeProcess("pdfinfo", args);
  std::smatch match;
  if (std::regex_search(output, match, std::regex("Pages.+(\\d)"))) {
    return std::stoi(match[1]);
  }

  return -1;
}

std::filesystem::path getBlankPage(Data const &settings)
{
    //do it later
  return settings.paths.working_directory / "blank.pdf";
}
