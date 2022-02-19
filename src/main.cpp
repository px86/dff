#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <chrono>

#include "include/argparser.hpp"
#include "include/sha256.hpp"

int main(int argc, char **argv) {

  auto start = std::chrono::high_resolution_clock::now();

  const char *rootpath = ".";

  auto ap = pr::ArgParser("dff");
  ap.add_argument(rootpath, "directory to be searched recursively", "<rootdir>");
  ap.parse(argc, argv);

  auto db = std::unordered_map<std::string_view,
                               std::vector<std::filesystem::path>>();

  auto rd_itr = std::filesystem::recursive_directory_iterator(rootpath);

  for (const auto &file: rd_itr) {

    if (!std::filesystem::is_regular_file(file) ||
        std::filesystem::file_size(file) == 0)
      continue;

    auto [ itr, inserted ] = db.insert({hash_file(file.path().c_str()), {file.path()}});
    if (!inserted)
      itr->second.push_back(file.path());
  }

  for (const auto& ent: db) {
    if (ent.second.size() > 1) {
      std::cout << '\n' << "\x1b[31m";
      for (const auto& path: ent.second) std::cout << path.c_str() << '\n';
      std::cout << "\x1b[m"
                << "\n---------------------------------------------------\n";


    }
  }

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

  std::cout << '\n'
	    << "Finished...\n"
	    << "took " << duration.count() << " milliseconds" << std::endl;

  return 0;
}
