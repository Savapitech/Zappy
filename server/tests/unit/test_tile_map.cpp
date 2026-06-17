#include <criterion/criterion.h>

#include "Game/Common.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"

Test(tile, starts_empty) {
  game::Tile t;
  for (int i = 0; i < RESOURCE_COUNT; i++)
    cr_assert_eq(t.getRessource(i), 0);
}

Test(tile, add_and_remove_resource) {
  game::Tile t;
  t.addRessource(FOOD_IDX, 3);
  cr_assert_eq(t.getRessource(FOOD_IDX), 3);
  t.addRessource(FOOD_IDX, 2);
  cr_assert_eq(t.getRessource(FOOD_IDX), 5);
  t.removeRessource(FOOD_IDX, 4);
  cr_assert_eq(t.getRessource(FOOD_IDX), 1);
}

Test(tile, resources_are_independent_per_index) {
  game::Tile t;
  t.addRessource(FOOD_IDX, 1);
  t.addRessource(LINEMATE_IDX, 1);
  cr_assert_eq(t.getRessource(DERAUMERE_IDX), 0);
  cr_assert_eq(t.getRessource(FOOD_IDX), 1);
  cr_assert_eq(t.getRessource(LINEMATE_IDX), 1);
}

Test(map, in_bounds_tiles_are_independent) {
  game::Map m(5, 5);
  m.getTile(0, 0).addRessource(FOOD_IDX, 1);
  m.getTile(1, 0).addRessource(FOOD_IDX, 2);
  cr_assert_eq(m.getTile(0, 0).getRessource(FOOD_IDX), 1);
  cr_assert_eq(m.getTile(1, 0).getRessource(FOOD_IDX), 2);
}

// The world wraps around: exiting right comes back through the left, and
// likewise for top/bottom, per the subject's "Geography" section.
Test(map, wraps_around_horizontally) {
  game::Map m(5, 5);
  m.getTile(0, 0).addRessource(FOOD_IDX, 7);
  cr_assert_eq(m.getTile(5, 0).getRessource(FOOD_IDX), 7,
              "x == width must wrap back to x == 0");
  cr_assert_eq(m.getTile(-1, 0).getRessource(FOOD_IDX), 0,
              "x == -1 must wrap to x == width-1, an untouched tile");
}

Test(map, wraps_around_vertically) {
  game::Map m(5, 5);
  m.getTile(0, 0).addRessource(LINEMATE_IDX, 4);
  cr_assert_eq(m.getTile(0, 5).getRessource(LINEMATE_IDX), 4,
              "y == height must wrap back to y == 0");
}

Test(map, negative_coordinates_wrap_to_last_tile) {
  game::Map m(5, 5);
  m.getTile(4, 4).addRessource(SIBUR_IDX, 9);
  cr_assert_eq(m.getTile(-1, -1).getRessource(SIBUR_IDX), 9);
}

Test(map, large_multiple_wraps_resolve_to_same_tile) {
  game::Map m(5, 5);
  m.getTile(2, 3).addRessource(PHIRAS_IDX, 1);
  cr_assert_eq(m.getTile(2 + 5 * 10, 3 - 5 * 7).getRessource(PHIRAS_IDX), 1);
}
