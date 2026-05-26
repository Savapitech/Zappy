#pragma once

#include <memory>
#include <vector>

#include "Common.hpp"
#include "Team.hpp"

namespace game {
  class GameLogic
  {
    private:
      std::vector<std::unique_ptr<Team>>_teams;
    public:
      const std::vector<std::unique_ptr<Team>> &getTeams() const { return _teams; }
  };
} //namespace game
