#pragma once
#ifndef MAIN_HPP
#define MAIN_HPP
#include <filesystem>
#include <string>
#include <vector>
// #include "boost_process.hpp"

#include "data_structs.hpp"

// main
void findFiles(std::vector<File> &files, Data const &settings);
std::vector<size_t> sortFiles(std::vector<File> &files, Data const &settings);
void processFiles(std::vector<File> const &files, std::vector<size_t> const &positions, Data const &settings);
void archiveFiles(std::vector<File> const &files, Data const &settings);
// helpers
bool isRecent(std::filesystem::file_time_type const &time, Data const &settings);
File::DateRange parseDate(std::string_view const &filename, Data const &settings, bool year_change = false);
Date getCurrentDate();
std::vector<std::string> tokenizeString(std::string string, std::string_view delims);
int getPageCount(std::filesystem::directory_entry entry);
std::string executeProcess(std::string const &exec, std::vector<std::string> const &args);
std::filesystem::path getBlankPage(Data const &settings);

#endif
