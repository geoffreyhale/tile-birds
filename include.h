// include.h
#pragma once

#include <vector>

enum CARDINAL_DIRECTION{ NONE, NORTH, SOUTH, EAST, WEST};

const int grid_cells_x = 16;
const int grid_cells_y = 16;

float random(float LO, float HI);
int random_int(int LO, int HI);

struct Cell {
  bool tiled;
  bool blocked;
  Cell();
  Cell(bool _tiled, bool _blocked);
};

struct Arena {
  int width; int height;
  std::vector<Cell> cell_vector;
  Arena();
  Arena(int _width, int _height);
  Cell cell(int _x, int _y);
  void set(int _x, int _y, bool _tiled);
  bool cell_exists(int _x, int _y);
  bool can_move(int _x, int _y);
  int count_tiled();
  int percent_tiled();
  bool fully_tiled();
};

struct Object {
  int x;
  int y;
  CARDINAL_DIRECTION cardinal_direction;
  Object();
  Object(int _x, int _y);
  Object(int _x, int _y, CARDINAL_DIRECTION _cardinal_direction);
  void Facing(CARDINAL_DIRECTION _cardinal_direction);
};

struct Player {
  Object obj;
  int facing_x; int facing_y;
  bool has_block;
  int coins;
  Player();
  Player(int _x, int _y, int _facing_x, int _facing_y, bool _has_block);
  void Facing(CARDINAL_DIRECTION _cardinal_direction);
};

struct Nest {
  Object obj;
  Nest();
  Nest(int _x, int _y);
};

struct Foo {
  Object obj;
  int count_since_moved;
  float distance_last_moved;
  Foo();
  Foo(int _x, int _y);
  void Update(Object _target);
};

struct Item {
  Object obj;
  Item();
  Item(int _x, int _y);
};