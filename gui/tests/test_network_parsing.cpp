#include <criterion/criterion.h>
#include <string>
#include <vector>

#include "Network/INetworkClient.hpp"
#include "Network/NetworkManager.hpp"

using namespace Zappy;

class FakeClient : public INetworkClient {
public:
  std::vector<std::string> inbox;
  std::vector<std::string> sent;

  bool connectToServer(const std::string &, int) override { return true; }
  std::vector<std::string> fetchLines() override {
    std::vector<std::string> out = inbox;
    inbox.clear();
    return out;
  }
  void sendCommand(const std::string &cmd) override { sent.push_back(cmd); }

  void feed(const std::string &line) { inbox.push_back(line); }
};

Test(gui_net, msz_sets_map_dimensions_and_grid) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("msz 10 12");
  nm.update();

  const GameState &gs = nm.getGameState();
  cr_assert_eq(gs.map.width, 10);
  cr_assert_eq(gs.map.height, 12);
  cr_assert(gs.map.isInitialized);
  cr_assert_eq((int)gs.grid.size(), 120);
}

Test(gui_net, msz_requests_map_content) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("msz 5 5");
  nm.update();
  bool asked = false;
  for (auto &s : c.sent)
    if (s.find("mct") != std::string::npos)
      asked = true;
  cr_assert(asked, "msz should trigger a map-content request");
}

Test(gui_net, msz_too_short_is_ignored) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("msz 10");
  nm.update();
  cr_assert_not(nm.getGameState().map.isInitialized);
}

Test(gui_net, bct_fills_tile_resources) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("msz 10 10");
  c.feed("bct 2 3 1 2 3 4 5 6 7");
  nm.update();

  const GameState &gs = nm.getGameState();
  int index = 3 * 10 + 2;
  for (int i = 0; i < 7; i++)
    cr_assert_eq(gs.grid[index].resources[i], i + 1, "resource %d", i);
}

Test(gui_net, bct_out_of_range_does_not_crash) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("msz 4 4");
  c.feed("bct 99 99 1 1 1 1 1 1 1");
  nm.update(); // must simply ignore the impossible coordinate
  cr_assert(nm.getGameState().map.isInitialized);
}

Test(gui_net, bct_before_msz_is_ignored) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("bct 0 0 1 1 1 1 1 1 1");
  nm.update();
  cr_assert_eq((int)nm.getGameState().grid.size(), 0);
}

Test(gui_net, tna_accumulates_team_names) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("tna red");
  c.feed("tna blue");
  c.feed("tna green");
  nm.update();

  const auto &teams = nm.getGameState().map.teamNames;
  cr_assert_eq((int)teams.size(), 3);
  cr_assert_str_eq(teams[0].c_str(), "red");
  cr_assert_str_eq(teams[2].c_str(), "green");
}

Test(gui_net, pnw_registers_a_new_player) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw #7 4 5 2 3 red");
  nm.update();

  const auto &players = nm.getGameState().players;
  cr_assert(players.contains(7));
  const Player &p = players.at(7);
  cr_assert_eq(p.x, 4);
  cr_assert_eq(p.y, 5);
  cr_assert_eq(p.orientation, 2);
  cr_assert_eq(p.level, 3);
  cr_assert_str_eq(p.team.c_str(), "red");
}

Test(gui_net, ppo_updates_known_player_position) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw #7 0 0 1 1 red");
  c.feed("ppo #7 8 9 3");
  nm.update();

  const Player &p = nm.getGameState().players.at(7);
  cr_assert_eq(p.x, 8);
  cr_assert_eq(p.y, 9);
  cr_assert_eq(p.orientation, 3);
}

Test(gui_net, ppo_for_unknown_player_is_ignored) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("ppo #42 1 1 1");
  nm.update();
  cr_assert_not(nm.getGameState().players.contains(42));
}

Test(gui_net, plv_updates_player_level) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw #7 0 0 1 1 red");
  c.feed("plv #7 6");
  nm.update();
  cr_assert_eq(nm.getGameState().players.at(7).level, 6);
}

Test(gui_net, pin_updates_player_inventory) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw #7 1 1 1 1 red");
  c.feed("pin #7 1 1 10 9 8 7 6 5 4");
  nm.update();

  const Player &p = nm.getGameState().players.at(7);
  int expected[7] = {10, 9, 8, 7, 6, 5, 4};
  for (int i = 0; i < 7; i++)
    cr_assert_eq(p.inventory[i], expected[i], "inv[%d]", i);
}

Test(gui_net, pdi_removes_player) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw #7 1 1 1 1 red");
  c.feed("pdi #7");
  nm.update();
  cr_assert_not(nm.getGameState().players.contains(7));
}

Test(gui_net, parseId_handles_bare_and_hashed_ids) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw 3 0 0 1 1 red"); // no '#'
  nm.update();
  cr_assert(nm.getGameState().players.contains(3));
}

Test(gui_net, tile_update_emits_an_event) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("msz 5 5");
  c.feed("bct 1 1 0 0 0 0 0 0 0");
  nm.update();

  auto events = nm.consumeEvents();
  bool found = false;
  for (auto &e : events)
    if (e.type == NetworkEventType::TILE_UPDATED)
      found = true;
  cr_assert(found);
}

Test(gui_net, consume_events_clears_the_queue) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("pnw #1 0 0 1 1 red");
  nm.update();
  cr_assert_gt((int)nm.consumeEvents().size(), 0);
  cr_assert_eq((int)nm.consumeEvents().size(), 0);
}

Test(gui_net, unknown_command_is_ignored_gracefully) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("zzz garbage line");
  nm.update(); // must not throw / crash
  cr_assert_not(nm.getGameState().map.isInitialized);
}

Test(gui_net, empty_line_is_ignored) {
  FakeClient c;
  NetworkManager nm(c, "127.0.0.1", 4242);
  c.feed("");
  nm.update();
  cr_assert_eq((int)nm.getGameState().players.size(), 0);
}
