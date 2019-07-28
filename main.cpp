#include "main.hpp"
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
#include "data_structs.hpp"
#include "parse_settings.hpp"
// #include "debug.cpp"

// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output

using file_time_type =
    std::chrono::time_point<std::chrono::system_clock>; // is it needed ? ugh i used it when fighting clocks but i'm not
                                                        // sure now, too much work to debug.

int main()
{
  // init settings
  Data settings;
  loadConfig(settings, ".resources/settings.json");
  settings.current_date = getCurrentDate();
  // DebugConfig(settings);

  // main data
  std::vector<File> files;
  findFiles(files, settings);
  std::vector<size_t> sorted_files_pos = sortFiles(files, settings);
  processFiles(files, sorted_files_pos, settings);
  archiveFiles(files, settings);

  return 0;
}

void findFiles(std::vector<File> &files, Data const &settings)
{
  if (!std::filesystem::exists(settings.paths.files_to_sort)) {
    std::cerr << "ERROR: files_to_sort directory does not exist\n";
    exit(1);
  };

  for (auto entry : std::filesystem::directory_iterator(settings.paths.files_to_sort)) {
    if (entry.is_regular_file() && entry.path().extension() == settings.file_parsing.file_extension) {
      for (int i = 0; i < settings.file_parsing.search_strings.size(); ++i) {
        if (entry.path().filename().string().find(settings.file_parsing.search_strings[i]) != std::string::npos) {
          File new_file;
          new_file.path = entry.path().string();
          new_file.type = isRecent(entry.last_write_time(), settings) ? File::FileType(i) : File::FileType::leftover;
          new_file.no_of_pages = getPageCount(entry);
          new_file.write_time  = entry.last_write_time();

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

  if (shopping_list_pos == -1) {
    std::cerr << "ERROR: Did not find any recent shopping list\n";
    exit(1);
  }

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

void processFiles(std::vector<File> const &files, std::vector<size_t> const &positions, Data const &settings)
{
  std::vector<File> processing;
  for (auto pos : positions) {
    processing.push_back(files[pos]);
  }
  // check directories and make if they're needed
  if (!std::filesystem::exists(settings.paths.working_directory)) {
    std::cerr << "ERROR working_dir does not exist\n";
    exit(1);
  }
  std::filesystem::remove_all(settings.paths.tmp_dir);
  std::filesystem::create_directory(settings.paths.tmp_dir);
  std::filesystem::path tmp_dir = settings.paths.tmp_dir;

  std::filesystem::path processing_dir = settings.paths.tmp_dir / "processing";
  std::filesystem::create_directory(processing_dir);
  std::filesystem::path blank_page = getBlankPage(settings);

  for (auto &file : processing) {
    std::filesystem::path file_path{ file.path }; // paths stored as strings
    if (file.no_of_pages == 2) {
      std::filesystem::copy_file(file_path, (tmp_dir / file_path.filename()));
      file.path = (tmp_dir / file_path.filename()).string();
    } else {
      std::filesystem::copy_file(file_path, (processing_dir / file_path.filename()));
      std::string output = (settings.paths.tmp_dir / file_path.filename()).string();
      std::vector<std::string> args{ file_path.string(), blank_page.string(), // merge with blank page
                                     output };                                // save to tmpDir
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
  // prepare final filename
  std::vector<std::string> shopping_list_tokens =
      tokenizeString(files[positions[0]].path, settings.file_parsing.delims);
  // clang-format off
  std::string finished_file_name { "merged-menu-"
                                  + shopping_list_tokens[settings.file_parsing.file_mask.at("day_start")] + '.'
                                  + shopping_list_tokens[settings.file_parsing.file_mask.at("month_start")] + '-'
                                  + shopping_list_tokens[settings.file_parsing.file_mask.at("day_end")] + '.'
                                  + shopping_list_tokens[settings.file_parsing.file_mask.at("month_end")] + ".pdf"
};
  // clang-format on

  // crop
  executionPaths.clear();
  executionPaths.push_back(mergedOut);
  executionPaths.push_back((settings.paths.working_directory / finished_file_name).string());
  executeProcess("pdfcrop", executionPaths);

  // cleanup
  std::filesystem::remove_all(settings.paths.tmp_dir);
}
void archiveFiles(std::vector<File> const &files, Data const &settings)
{
  for (File file : files) {
    std::filesystem::path file_path{ file.path };
    std::filesystem::rename(file_path, settings.paths.archive_dir / file_path.filename());
  }
}

//=======
// UTILS
//=======

File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool year_change)
{
  File::DateRange result;

  std::vector<std::string> tok = tokenizeString(static_cast<std::string>(filename), settings.file_parsing.delims);
  result.start.day             = std::stoi(tok[settings.file_parsing.file_mask.at("day_start")]);
  result.end.day               = std::stoi(tok[settings.file_parsing.file_mask.at("day_end")]);
  result.start.month           = std::stoi(tok[settings.file_parsing.file_mask.at("month_start")]);
  result.end.month             = std::stoi(tok[settings.file_parsing.file_mask.at("month_end")]);
  result.start.year = result.end.year = settings.current_date.year;

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
  std::vector<std::string> tmp;
  size_t pos = 0;
  while ((pos = string.find_first_of(delims)) != std::string::npos) {
    tmp.push_back(string.substr(0, pos));
    string.erase(0, pos + 1);
  }
  tmp.push_back(string);
  return tmp;
}

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
  // do it later .. someday ? maybe ? honestly i really dislike idea playing with boost pipelines for something this
  // simple. unnecessary function but i already overdid it more than i planned.
  std::filesystem::path blank_page_path = settings.paths.working_directory / ".resources/blank.pdf";
  if (!std::filesystem::exists(blank_page_path)) {
    std::cerr << "ERROR: didn't find blank.pdf in working directory, creation of blank pages has not implemented";
    exit(2);
  }
  return blank_page_path;
}
