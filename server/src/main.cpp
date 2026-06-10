#include <csignal>
#include <cstdint>
#include <iostream>
#include <limits.h>
#include <string>

#include <termios.h>
#include <unistd.h>

#include "Game/GameLogic.hpp"
#include "Logger.hpp"
#include "Server.hpp"

static Server *g_server = nullptr;
static struct termios saved_term_settings;

static void initTerm() {
  if (isatty(STDIN_FILENO))
    tcgetattr(STDIN_FILENO, &saved_term_settings);

  struct termios new_settings = saved_term_settings;

  if (isatty(STDIN_FILENO)) {
    new_settings.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
  }
}

static void restoreTerm() {
  if (isatty(STDIN_FILENO))
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_term_settings);
}

static void handleSignal(int signum) {
  if (g_server != nullptr) {
    LOG_DEBUG("Signal received, shutting down server gracefully...");
    g_server->stop();
  }
}

static void printHelp() {
  std::cout << "USAGE: ./zappy_server\n"
            << "\t-p port -x width -y height -n name1 name2 ... -c clientsNb "
               "-f freq\n";
}

static bool checkargs(int argc, char **argv) {
  int pIdx = -1;
  int xIdx = -1;
  int yIdx = -1;
  int nIdx = -1;
  int cIdx = -1;
  int fIdx = -1;

  for (int i = 1; i < argc; i++) {
    std::string act(argv[i]);
    if (act == "-p") {
      if (pIdx != -1)
        return LOG_ERROR("Duplicate option"), false;
      pIdx = i;
    } else if (act == "-x") {
      if (xIdx != -1)
        return LOG_ERROR("Duplicate option"), false;
      xIdx = i;
    } else if (act == "-y") {
      if (yIdx != -1)
        return LOG_ERROR("Duplicate option"), false;
      yIdx = i;
    } else if (act == "-n") {
      if (nIdx != -1)
        return LOG_ERROR("Duplicate option"), false;
      nIdx = i;
    } else if (act == "-c") {
      if (cIdx != -1)
        return LOG_ERROR("Duplicate option"), false;
      cIdx = i;
    } else if (act == "-f") {
      if (fIdx != -1)
        return LOG_ERROR("Duplicate option"), false;
      fIdx = i;
    }
  }
  if (pIdx == -1 || xIdx == -1 || yIdx == -1 || nIdx == -1 || cIdx == -1 ||
      fIdx == -1)
    return LOG_ERROR("Missing one of the arguments"), false;

  if (!(pIdx == 1 && xIdx == 3 && yIdx == 5 && nIdx == 7 && nIdx < cIdx &&
        cIdx + 2 == fIdx))
    return LOG_ERROR("Invalid order"), false;

  if (pIdx + 1 >= argc || xIdx + 1 >= argc || yIdx + 1 >= argc ||
      cIdx + 1 >= argc || fIdx + 1 >= argc)
    return LOG_ERROR("Missing value after an option"), false;

  try {
    std::size_t pos;
    if (std::stoi(argv[pIdx + 1], &pos) <= 0 ||
        pos != std::string(argv[pIdx + 1]).size())
      return LOG_ERROR("Invalid number in parameters"), false;
    if (std::stoi(argv[xIdx + 1], &pos) <= 0 ||
        pos != std::string(argv[xIdx + 1]).size())
      return LOG_ERROR("Invalid number in parameters"), false;
    if (std::stoi(argv[yIdx + 1], &pos) <= 0 ||
        pos != std::string(argv[yIdx + 1]).size())
      return LOG_ERROR("Invalid number in parameters"), false;
    if (std::stoi(argv[cIdx + 1], &pos) <= 0 ||
        pos != std::string(argv[cIdx + 1]).size())
      return LOG_ERROR("Invalid number in parameters"), false;
    if (std::stoi(argv[fIdx + 1], &pos) <= 0 ||
        pos != std::string(argv[fIdx + 1]).size())
      return LOG_ERROR("Invalid number in parameters"), false;
  } catch (const std::exception &) {
    return LOG_ERROR("Invalid number in parameters"), false;
  }

  if (nIdx + 1 == cIdx)
    return LOG_ERROR("Missing at least one teamname"), false;

  return true;
}

int main(int argc, char **argv) {
  if (argc == 2 && std::string(argv[1]) == "--help")
    return printHelp(), 0;

  if (!checkargs(argc, argv))
    return printHelp(), 84;

  uint16_t port;
  try {
    int parsedPort = std::stoi(argv[2]);
    if (!parsedPort || parsedPort > UINT16_MAX)
      throw std::out_of_range("Port out of range");
    port = static_cast<uint16_t>(parsedPort);
  } catch (const std::exception &) {
    return LOG_ERROR("Invalid port number."), 84;
  }

  int x;
  int y;
  int clientMax;
  int freq;
  std::vector<std::string> teamnames;
  try {
    x = std::stoi(argv[4]);
    y = std::stoi(argv[6]);
    int index = 8;
    while (std::string(argv[index]) != "-c") {
      teamnames.push_back(std::string(argv[index]));
      index++;
    }
    index++;
    clientMax = std::stoi(argv[index]);
    index += 2;
    freq = std::stoi(argv[index]);
  } catch (const std::exception &) {
    return LOG_ERROR("Invalid arguments."), 84;
  }

  try {
    Server server(port);
    g_server = &server;
    game::GameLogic game(x, y, freq, clientMax, teamnames);

    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    initTerm();

    server.run(game);
    g_server = nullptr;
  } catch (const std::exception &e) {
    restoreTerm();
    handleSignal(0);
    return LOG_FATAL("Server fatal error: " + std::string(e.what())), 84;
  }

  restoreTerm();
  return 0;
}
