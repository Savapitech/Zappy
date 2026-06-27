#include <criterion/criterion.h>
#include <set>
#include <string>
#include <vector>

#include "Game/Common.hpp"
#include "Game/GameLogic.hpp"

using game::GameLogic;

// Construction-time invariants and resource seeding across a range of map
// sizes and team layouts. Everything here stays on the Client-free surface of
// GameLogic so it can run as a pure unit test.

Test(setup, stores_map_dimensions_for_many_sizes) {
  int sizes[][2] = {{10, 10}, {30, 30}, {42, 7}, {1, 1}, {100, 50}, {5, 80}};
  for (auto &s : sizes) {
    GameLogic g(s[0], s[1], 100, 1, {"team1"});
    cr_assert_eq(g.getMapX(), s[0]);
    cr_assert_eq(g.getMapY(), s[1]);
    cr_assert_eq(g.getMap().getWidth(), s[0]);
    cr_assert_eq(g.getMap().getHeigth(), s[1]);
  }
}

Test(setup, frequency_is_stored_and_mutable) {
  GameLogic g(10, 10, 100, 1, {"team1"});
  cr_assert_eq(g.getFreq(), 100);
  for (int f : {1, 2, 50, 1000, 10000}) {
    g.setFreq(f);
    cr_assert_eq(g.getFreq(), f);
  }
}

Test(setup, client_max_is_stored) {
  for (int c : {1, 2, 5, 10, 50}) {
    GameLogic g(10, 10, 100, c, {"team1"});
    cr_assert_eq(g.getClientMax(), c);
  }
}

Test(setup, every_requested_team_is_created) {
  std::vector<std::string> names = {"red", "green", "blue", "yellow"};
  GameLogic g(10, 10, 100, 3, names);
  cr_assert_eq((int)g.getTeams().size(), (int)names.size());
  std::set<std::string> found;
  for (auto &t : g.getTeams())
    found.insert(t->getName());
  for (auto &n : names)
    cr_assert(found.count(n) == 1, "team %s missing", n.c_str());
}

Test(setup, single_team_is_supported) {
  GameLogic g(10, 10, 100, 1, {"solo"});
  cr_assert_eq((int)g.getTeams().size(), 1);
  cr_assert_str_eq(g.getTeams()[0]->getName().c_str(), "solo");
}

Test(setup, each_team_starts_with_capacity_equal_to_client_max) {
  GameLogic g(10, 10, 100, 4, {"a", "b"});
  for (auto &t : g.getTeams())
    cr_assert_eq(t->getClientMax(), 4);
}

Test(setup, next_id_is_non_negative_at_start) {
  GameLogic g(10, 10, 100, 1, {"team1"});
  cr_assert(g.getNextId() >= 0);
}

Test(setup, getIndexByName_is_the_inverse_of_the_resource_order) {
  GameLogic g(10, 10, 100, 1, {"team1"});
  struct {
    const char *name;
    int idx;
  } table[] = {{"food", FOOD_IDX},         {"linemate", LINEMATE_IDX},
               {"deraumere", DERAUMERE_IDX}, {"sibur", SIBUR_IDX},
               {"mendiane", MENDIANE_IDX},   {"phiras", PHIRAS_IDX},
               {"thystame", THYSTAME_IDX}};
  for (auto &e : table) {
    std::string n = e.name;
    cr_assert_eq(g.getIndexByName(n), e.idx, "%s -> %d", e.name, e.idx);
  }
}

Test(setup, initRessources_seeds_every_resource_on_a_standard_map) {
  // The GameLogic constructor already seeds the map once, so we measure the
  // tiles directly without re-seeding.
  GameLogic g(20, 20, 100, 1, {"team1"});
  long totals[RESOURCE_COUNT] = {0};
  for (int x = 0; x < g.getMapX(); x++)
    for (int y = 0; y < g.getMapY(); y++)
      for (int i = 0; i < RESOURCE_COUNT; i++)
        totals[i] += g.getMap().getTile(x, y).getRessource(i);
  for (int i = 0; i < RESOURCE_COUNT; i++)
    cr_assert(totals[i] >= 1, "resource %d was never spawned", i);
}

Test(setup, initRessources_respects_the_density_formula_for_food) {
  // map_width * map_height * density, with the constructor's single seeding.
  GameLogic g(20, 20, 100, 1, {"team1"});
  long food = 0;
  for (int x = 0; x < 20; x++)
    for (int y = 0; y < 20; y++)
      food += g.getMap().getTile(x, y).getRessource(FOOD_IDX);
  // 20 * 20 * 0.5 = 200 expected; seeding may use ceil/round, so allow a
  // small tolerance around the formula.
  cr_assert(food >= 180 && food <= 220, "food total %ld off formula", food);
}

Test(setup, getPlayerById_returns_null_when_empty) {
  GameLogic g(10, 10, 100, 1, {"team1"});
  cr_assert_null(g.getPlayerById(123).get());
}

Test(setup, gui_team_is_accepted_alongside_regular_teams) {
  GameLogic g(10, 10, 100, 1, {"team1", GUI_TEAM});
  bool hasGui = false;
  for (auto &t : g.getTeams())
    if (t->getName() == GUI_TEAM)
      hasGui = true;
  cr_assert(hasGui);
}
