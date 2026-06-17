#include <criterion/criterion.h>
#include <sstream>

#include "Game/Common.hpp"
#include "Game/GameLogic.hpp"

Test(getIndexByName, maps_all_known_resource_names) {
  game::GameLogic g(10, 10, 1, 1, {"team1"});

  std::string food = "food", linemate = "linemate", deraumere = "deraumere",
              sibur = "sibur", mendiane = "mendiane", phiras = "phiras",
              thystame = "thystame", bogus = "lead";

  cr_assert_eq(g.getIndexByName(food), FOOD_IDX);
  cr_assert_eq(g.getIndexByName(linemate), LINEMATE_IDX);
  cr_assert_eq(g.getIndexByName(deraumere), DERAUMERE_IDX);
  cr_assert_eq(g.getIndexByName(sibur), SIBUR_IDX);
  cr_assert_eq(g.getIndexByName(mendiane), MENDIANE_IDX);
  cr_assert_eq(g.getIndexByName(phiras), PHIRAS_IDX);
  cr_assert_eq(g.getIndexByName(thystame), THYSTAME_IDX);
  cr_assert_eq(g.getIndexByName(bogus), -1, "unknown resource name must return -1");
}

Test(formatBct, matches_protocol_shape) {
  game::GameLogic g(10, 10, 1, 1, {"team1"});
  std::string msg = g.formatBct(3, 4);

  cr_assert(msg.ends_with("\n"));
  std::istringstream iss(msg);
  std::string cmd;
  int x, y;
  iss >> cmd >> x >> y;
  cr_assert_str_eq(cmd.c_str(), "bct");
  cr_assert_eq(x, 3);
  cr_assert_eq(y, 4);

  int values[RESOURCE_COUNT];
  for (int i = 0; i < RESOURCE_COUNT; i++)
    cr_assert(static_cast<bool>(iss >> values[i]),
              "bct must carry exactly %d resource quantities", RESOURCE_COUNT);
  std::string leftover;
  cr_assert(!(iss >> leftover), "bct must not carry extra tokens");
}

Test(formatBct, reflects_actual_tile_state) {
  game::GameLogic g(10, 10, 1, 1, {"team1"});
  game::Tile &tile = g.getMap().getTile(1, 1);

  for (int i = 0; i < RESOURCE_COUNT; i++)
    while (tile.getRessource(i) > 0)
      tile.removeRessource(i, 1);
  tile.addRessource(LINEMATE_IDX, 3);

  std::string msg = g.formatBct(1, 1);
  cr_assert_eq(msg, std::string("bct 1 1 0 3 0 0 0 0 0\n"));
}

// "the resource quantity can be found with: map_width * map_height *
// density" and "on Trantor you must find at least one of each resource".
Test(initRessources, matches_density_formula_on_a_known_map_size) {
  // 10x10 -> exactly the example given in the subject: 50 food, 5 thystame.
  for (int iter = 0; iter < 100; iter++) {
    game::GameLogic g(10, 10, 1, 1, {"team1"});
    int totals[RESOURCE_COUNT] = {0};
    for (int y = 0; y < 10; y++)
      for (int x = 0; x < 10; x++)
        for (int i = 0; i < RESOURCE_COUNT; i++)
          totals[i] += g.getMap().getTile(x, y).getRessource(i);

    cr_assert_eq(totals[FOOD_IDX], 50, "expected 50 food on a 10x10 map (iter %d)",
                iter);
    cr_assert_eq(totals[THYSTAME_IDX], 5,
                "expected 5 thystame on a 10x10 map (iter %d)", iter);
  }
}

Test(initRessources, never_spawns_zero_of_a_resource_even_on_tiny_maps) {
  // "on Trantor you must find at least one of each ressource and food".
  for (int iter = 0; iter < 100; iter++) {
    game::GameLogic g(2, 2, 1, 1, {"team1"});
    int totals[RESOURCE_COUNT] = {0};
    for (int y = 0; y < 2; y++)
      for (int x = 0; x < 2; x++)
        for (int i = 0; i < RESOURCE_COUNT; i++)
          totals[i] += g.getMap().getTile(x, y).getRessource(i);

    for (int i = 0; i < RESOURCE_COUNT; i++)
      cr_assert_geq(totals[i], 1,
                    "resource %d must have at least 1 unit on the map (iter %d)", i,
                    iter);
  }
}

Test(initEggs, every_non_gui_team_gets_clientMax_eggs) {
  for (int iter = 0; iter < 50; iter++) {
    game::GameLogic g(10, 10, 1, 3, {"team1", "team2"});
    for (const auto &team : g.getTeams()) {
      cr_assert_eq(team->getEggs().size(), (size_t)3,
                  "team %s should start with exactly clientMax eggs",
                  team->getName().c_str());
      cr_assert_eq(team->getAvailable(), 3);
    }
  }
}

Test(initEggs, eggs_spawn_within_map_bounds) {
  game::GameLogic g(7, 13, 1, 5, {"team1"});
  for (const auto &team : g.getTeams()) {
    for (const auto &egg : team->getEggs()) {
      cr_assert(egg->getX() >= 0 && egg->getX() < 7);
      cr_assert(egg->getY() >= 0 && egg->getY() < 13);
    }
  }
}
