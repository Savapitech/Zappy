#pragma once

class Egg {
private:
  int _id;
  int _posX;
  int _posY;
  std::string _teamName;
  int _playerId;

public:
  Egg(int id, int x, int y, const std::string &teamName, int playerId)
      : _id(id), _posX(x), _posY(y), _teamName(teamName), _playerId(playerId) {}

  int getId() { return _id; }
  int getX() { return _posX; }
  int getY() { return _posY; }
  int getPlayerId() { return _playerId; }
  std::string getTeamName() { return _teamName; }
};
