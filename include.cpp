// include.cpp
#include "include.h"

#include <random>
#include <vector>

//class implementation

float random(float LO, float HI) {
	float randomfloat = LO + (float)rand()/((float)RAND_MAX/(HI-LO)); // This will generate a number from some arbitrary LO to some arbitrary HI.
	return randomfloat;
}

//http://stackoverflow.com/questions/7560114/random-number-c-in-some-range
int random_int(int LO, int HI) {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(LO-1, HI); // define the range

    return distr(eng);
}

Cell::Cell()
{
  exists = false;
  blocked = false;
}
Cell::Cell(bool _exists, bool _blocked)
{
  exists = _exists;
  blocked = _blocked;
}
Arena::Arena()
{
}
Arena::Arena(int _width, int _height)
{
  width = _width; height = _height;
  for ( int i = 0; i < _width*_height ; i++ )
    cell_vector.push_back(Cell(0,0));
}
Cell Arena::cell(int _x, int _y)
{
  return cell_vector[_x+_y*width];
}
void Arena::set(int _x, int _y, bool _exists)
{
  cell_vector[_x+_y*width].exists = _exists;
}

Object::Object()
{
  x = 0; y = 0;
  cardinal_direction = NONE;
}
Object::Object(int _x, int _y)
{
  x = _x; y = _y;
  cardinal_direction = NONE;
}
Object::Object(int _x, int _y, CARDINAL_DIRECTION _cardinal_direction)
{
  x = _x; y = _y;
  cardinal_direction = _cardinal_direction;
}
void Object::Facing(CARDINAL_DIRECTION _cardinal_direction)
{
  cardinal_direction = _cardinal_direction;
}

Player::Player()
{
  obj = Object(0,0);
  facing_x = 0;
  facing_y = 0;
  has_block = 0;
  coins = 0;
}
Player::Player(int _x, int _y, int _facing_x, int _facing_y, bool _has_block)
{
  obj = Object(_x,_y);
  facing_x = _facing_x;
  facing_y = _facing_y;
  has_block = _has_block;
  coins = 0;
}

Nest::Nest()
{
  obj = Object(0,0);
}
Nest::Nest(int _x, int _y)
{
  obj = Object(_x,_y);
}

Foo::Foo()
{
  obj = Object(0,0);
  count_since_moved = 0;
}
Foo::Foo(int _x, int _y)
{
  obj = Object(_x,_y);
  count_since_moved = 0;
}
void Foo::Update(Object _target)
{
  count_since_moved++;

  // MOVE:
  if ( count_since_moved > 120 ) //move every 2 seconds
        {
          int x_move = random_int(-1,1) + (_target.x-obj.x)/abs((_target.x-obj.x)+.1);//AI and +.1 hack for not div by 0
          int y_move = random_int(-1,1) + (_target.y-obj.y)/abs((_target.y-obj.y)+.1);//AI and +.1 hack for not div by 0
          if (  obj.x+x_move >= 0 &&  obj.x+x_move < grid_cells_x
              &&  obj.y+y_move >= 0 &&  obj.y+y_move < grid_cells_y 
              && abs(x_move + y_move) < 2 )
          {
             obj.x += x_move;
             obj.y += y_move;
             count_since_moved = 0;
          }
        }
}

Item::Item()
{
  obj = Object(0,0);
}
Item::Item(int _x, int _y)
{
  obj = Object(_x, _y);
}