#include <criterion/criterion.h>

#include "Game/Player.hpp"
#include "Game/Team.hpp"

Test(team, starts_with_given_capacity) {
  game::Team t("team1", 4);
  cr_assert_str_eq(t.getName().c_str(), "team1");
  cr_assert_eq(t.getClientMax(), 4);
  cr_assert_eq(t.getConnected(), 0);
  cr_assert_eq(t.getAvailable(), 4);
}

Test(team, connecting_reduces_availability) {
  game::Team t("team1", 2);
  t.addConnected();
  cr_assert_eq(t.getAvailable(), 1);
  t.addConnected();
  cr_assert_eq(t.getAvailable(), 0);
}

// Forking adds a slot to the team (subject section "Player Reproduction":
// "Once the egg is laid, a new slot is added to the team").
Test(team, fork_increases_available_slot) {
  game::Team t("team1", 1);
  t.addConnected();
  cr_assert_eq(t.getAvailable(), 0);
  t.addClientMax();
  cr_assert_eq(t.getAvailable(), 1);
}

Test(team, egg_pool_add_pick_remove) {
  game::Team t("team1", 1);
  cr_assert(!t.pickRandomEgg().has_value(), "no eggs yet, must be empty");

  t.addEgg(std::make_unique<Egg>(1, 2, 3, "team1", -1));
  auto egg = t.pickRandomEgg();
  cr_assert(egg.has_value());
  cr_assert_eq(egg->get().getId(), 1);
  cr_assert_eq(egg->get().getX(), 2);
  cr_assert_eq(egg->get().getY(), 3);

  t.removeEgg(1);
  cr_assert(!t.pickRandomEgg().has_value(), "egg pool must be empty after removal");
}

Test(team, removing_unknown_egg_is_a_no_op) {
  game::Team t("team1", 1);
  t.addEgg(std::make_unique<Egg>(5, 0, 0, "team1", -1));
  t.removeEgg(999);
  cr_assert_eq(t.getEggs().size(), (size_t)1);
}

Test(team, pick_random_egg_only_returns_existing_ids) {
  game::Team t("team1", 5);
  for (int i = 0; i < 5; i++)
    t.addEgg(std::make_unique<Egg>(i, 0, 0, "team1", -1));

  // Run many times: pickRandomEgg uses rand(), every draw must return one
  // of the eggs that were actually inserted.
  for (int iter = 0; iter < 500; iter++) {
    auto egg = t.pickRandomEgg();
    cr_assert(egg.has_value());
    int id = egg->get().getId();
    cr_assert(id >= 0 && id < 5, "unexpected egg id %d drawn", id);
  }
}

Test(team, add_player_and_remove_user) {
  game::Team t("team1", 2);
  auto p1 = std::make_shared<game::Player>(1, "team1");
  auto p2 = std::make_shared<game::Player>(2, "team1");
  t.addPlayer(p1);
  t.addPlayer(p2);
  cr_assert_eq(t.getPlayers().size(), (size_t)2);

  t.addConnected();
  t.addConnected();
  t.removeUser(1);
  cr_assert_eq(t.getPlayers().size(), (size_t)1);
  cr_assert_eq(t.getPlayers()[0]->getId(), 2);
  cr_assert_eq(t.getAvailable(), 1, "removeUser must free a connection slot");
}
