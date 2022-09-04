#include <chrono>
#include <cstdlib>
#include <iostream>

#include "include/dff.hpp"
#include "include/argparser.hpp"

int main(int argc, char **argv) {

  const char *rootpath = ".";
  bool follow_symlinks = false;
  const char *outfile = nullptr;

  auto ap = pr::ArgParser("dff");

  ap.add_option(follow_symlinks, "follow symbolic links", "follow-sym", 0);
  ap.add_option(outfile, "export output to csv file", "export", 'e');
  ap.add_argument(rootpath, "directory to be searched recursively",
                  "<rootdir>");

  ap.parse(argc, argv);

  auto start = std::chrono::high_resolution_clock::now();

  auto dff = pr::dff(rootpath, follow_symlinks);
  dff.find_dups();

  if (outfile != nullptr) {
    dff.export_dups(outfile);
  }
  else dff.print_dups();

  auto stop = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

  std::cout << '\n'
            << "finished...\n"
            << "took " << duration.count() << " milliseconds" << std::endl;

  if (outfile != nullptr) {
    std::cout << "Output exported to \"" << outfile << "\"\n";
  }

  return EXIT_SUCCESS;
}
