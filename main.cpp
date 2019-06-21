#include <filesystem>
#include <iostream>
#include <string>

// #include <map>
// overkill app to format and merge downloaded diet pdfs into printable version
// to learn/practice some c++17 and spend less time preparing printable output

constexpr auto DEFAULT_PATH = "/data/Downloads/Browser";
constexpr auto WORKDIR = "/data/Downloads/Browser/menu";
// constexpr auto TMP_DIR = ".tmp";
// constexpr auto ARCHIVE_DIR = "archive";
constexpr auto DEFAULT_SHOPPINGLIST = "zakupy";
constexpr auto DEFAULT_EXTENSION = ".pdf";
constexpr auto STRING_DELIM = "-";
constexpr auto DATE_FORMAT = "%m.%d";
// date data

std::string getCurrentDate();
std::filesystem::directory_entry
FindShoppingList(std::filesystem::path directoryPath = DEFAULT_PATH,
                 std::string searchString = DEFAULT_SHOPPINGLIST,
                 std::string extension = DEFAULT_EXTENSION);

int main() {
  // data
  std::filesystem::directory_entry shoppingList;
  shoppingList = FindShoppingList();

  std::cout << "result: " << shoppingList.path().string() << '\n';

  return 0;
}

std::filesystem::directory_entry
FindShoppingList(std::filesystem::path directoryPath, std::string searchString,
                 std::string extension) {
  std::filesystem::directory_entry result;
  std::string currDate = getCurrentDate();

  if (!std::filesystem::exists(directoryPath)) {
    std::cerr << "ERROR::FindShoppingList()::path = " << directoryPath
              << " does not exist\n";
    return result;
  }

  for (const auto &entry : std::filesystem::directory_iterator(directoryPath)) {
    if (entry.is_regular_file()) {
      if (entry.path().extension() == extension) {
        std::string filename = entry.path().stem();
        if (filename.find(searchString) != std::string::npos)
          if (!result.exists()) {
            size_t begin = filename.find(STRING_DELIM) + 1;
            size_t end = filename.find(STRING_DELIM, begin);
            std::string date = filename.substr(begin, end);
            std::cerr << "DEBUG: " << date << '\n';
            if (date >= currDate)
              result = entry;
          } else { // if newer file exists
            if (entry.last_write_time() > result.last_write_time())
              result = entry;
            std::cerr << "DEBUG: " << entry.path().string() << " >? "
                      << result.path().string() << '\n';
          }
      }
    }
  }
  return result;
}

std::string getCurrentDate() {
  std::time_t t = std::time(nullptr);
  std::tm localtime = *std::localtime(&t);
  std::stringstream buffer;
  buffer << std::put_time(&localtime, DATE_FORMAT);
  return buffer.str();
}
