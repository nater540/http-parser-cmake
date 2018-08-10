#include <iostream>
#include <fstream>
#include <string>
#include <regex>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "ERR: Must specify Makefile path." << std::endl;
    std::cerr << "EG: `" << argv[0] << " http-parser/http-parser-VERS/Makefile`" << std::endl;
    return 1;
  }

  std::string makefile_path = argv[1];
  std::ifstream input(makefile_path);

  std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

  std::regex rgx("SOMAJOR\\s+=\\s+([0-9]*)\nSOMINOR\\s+=\\s+([0-9]*)\nSOREV\\s+=\\s+([0-9]*)");
  std::smatch matches;

  if (std::regex_search(contents, matches, rgx)) {
    std::cout << "MATCH COUNT = " << matches.size() << std::endl;
    for (size_t i = 1; i < matches.size(); ++i) {
      std::cout << "`" << matches[i].str() << "`" << std::endl;
    }
  }
  else {
    std::cerr << "Unable to extract version information from '" << makefile_path << "'" << std::endl;\
    return 1;
  }

  return 0;
}
