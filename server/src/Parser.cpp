#include <sstream>
#include <string>
#include <vector>

#include "Parser.hpp"

std::vector<std::string> ParseArgs(const std::string &line) {
  std::vector<std::string> args;
  std::istringstream iss(line);
  std::string token;

  while (iss >> token)
    args.push_back(token);

  return args;
}
