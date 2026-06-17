#include "Team.hpp"

void game::Team::removeUser(int playerId) {
  _players.erase(std::remove_if(_players.begin(), _players.end(),
                                [playerId](const std::shared_ptr<Player> &p) {
                                  return p->getId() == playerId;
                                }),
                 _players.end());
  removeConnected();
}

void game::Team::removeEgg(int eggId) {
  _eggs.erase(std::remove_if(_eggs.begin(), _eggs.end(),
                             [eggId](const std::unique_ptr<Egg> &egg) {
                               return egg->getId() == eggId;
                             }),
              _eggs.end());
}

std::optional<std::reference_wrapper<Egg>> game::Team::pickRandomEgg() {
  if (_eggs.empty())
    return std::nullopt;
  int index = rand() % _eggs.size();
  return std::ref(*_eggs[index]);
}
