#ifndef DEBUG_CPP
#define DEBUG_CPP
#include <iostream>
#include "data_structs.hpp"

void DebugConfig(Data config)
{
  std::cout << "== CONFIG DEBUG ==\n";

  std::cout << "config.paths: \n"
            << "\t.files_to_sort: " << config.paths.files_to_sort << '\n'
            << "\t.working_directory: " << config.paths.working_directory << "\n"
            << "\t.tmp_dir: " << config.paths.tmp_dir << '\n'
            << "\t.archive_dir: " << config.paths.archive_dir << '\n';

  std::cout << "config.file_parsing: \n";
  for (auto e : config.file_parsing.file_mask) {
    std::cout << "\t.file_mask: [" << e.first << "] == " << e.second << '\n';
  }
  std::cout << "\t.delims: " << config.file_parsing.delims << '\n'
            << "\t.file_extension: " << config.file_parsing.file_extension << '\n';

  std::cout << "search_strings: " << config.search_strings[0] << "\t" << config.search_strings[1] << '\n';
  std::cout << "date_range: " << config.date_range << '\n';
}

#endif
