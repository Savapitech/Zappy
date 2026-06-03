#pragma once

namespace Zappy {
enum class EventType {
  WindowClosed,
  KeyPressed,
  KeyReleased,
  MousePressed,
  MouseReleased,
  MouseMoved
};

enum class Key {
  Num0 = 48,
  Num1 = 49,
  Num2 = 50,
  Num3 = 51,
  Num4 = 52,
  Num5 = 53,
  Num6 = 54,
  Num7 = 55,
  Num8 = 56,
  Num9 = 57,

  A = 97,
  B = 98,
  C = 99,
  D = 100,
  E = 101,
  F = 102,
  G = 103,
  H = 104,
  I = 105,
  J = 106,
  K = 107,
  L = 108,
  M = 109,
  N = 110,
  O = 111,
  P = 112,
  Q = 113,
  R = 114,
  S = 115,
  T = 116,
  U = 117,
  V = 118,
  W = 119,
  X = 120,
  Y = 121,
  Z = 122,

  Space = 32,
  Escape = 65307,
  Enter = 65293,
  Up = 65362,
  Down = 65364,
  Left = 65361,
  Right = 65363,

  NONE = 0
};

struct Event {
  EventType type;

  int mouseX = 0;
  int mouseY = 0;
  int button = 0;
  Key keyCode = Key::NONE;
};
} // namespace Zappy
