#ifndef PARSE_SETTINGS_HPP
#define PARSE_SETTINGS_HPP

#include <fstream>
#include "data_structs.hpp"
#include "iostream"
#include "json.cpp"

void loadConfig(Data &settings, std::string const &filename)
{
  using json = nlohmann::json;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "ERROR: No settings.json file present\n";
    exit(1);
  }
  json config = json::parse(file);
  file.close();

  // paths
  settings.paths.files_to_sort     = config["paths"]["files_to_sort"].get<std::string>();
  settings.paths.working_directory = config["paths"]["working_dir"].get<std::string>();
  if (config["paths"]["tmp_dir"] != nullptr) {
    settings.paths.tmp_dir = config["paths"]["tmp_dir"].get<std::string>();
  } else {
    settings.paths.tmp_dir = settings.paths.working_directory / ".tmp";
  }
  if (config["paths"]["archive_dir"] != nullptr) {
    settings.paths.archive_dir = config["paths"]["archive_dir"].get<std::string>();
  } else {
    settings.paths.archive_dir = settings.paths.working_directory / "archive";
  }
  // parse
  for (int i = 0; i < config["file_parsing"]["file_mask"].size(); ++i) {
    settings.file_parsing.file_mask[config["file_parsing"]["file_mask"][i].get<std::string>()] = i;
  }
  settings.file_parsing.delims = config["file_parsing"]["delims"].get<std::string>();
  if (config["file_parsing"]["file_extension"] != nullptr) {
    settings.file_parsing.file_extension = config["file_parsing"]["file_extension"].get<std::string>();
  } else {
    settings.file_parsing.file_extension = ".pdf";
  }
  settings.file_parsing.search_strings[Data::Search::shopping_list] =
      config["file_parsing"]["file_type_strings"]["shopping_list"].get<std::string>();
  settings.file_parsing.search_strings[Data::Search::menu] =
      config["file_parsing"]["file_type_strings"]["menu"].get<std::string>();

  settings.date_range = config["date_range"].get<int>();
}
#endif
