//http://wiki.allegro.cc/index.php?title=Windows,_Visual_Studio_2010_and_Allegro_5 //TODO: Setup Project For Allegro 5
// MSVS2010 > Solution Explorer > R-Click Project > Properties (alt+Enter) > Configuration >
// All Configurations > Debugging > PATH > "PATH=c:\allegro\bin;%PATH%"
// C/C++ > General > Additional Include Directories > "c:\allegro\include"
// Linker > General > Additional Library Directories > "c:\allegro\lib"
// Linker > Input > Additional Dependencies > append: ";allegro-5.0.7-monolith-md-debug.lib"
// ***SOMETHING ELSE HERE I FORGET, LOOK IT UP AND FIX!!!!

/* TO DO LIST:
- baddies generated even after tiles full wtf?
- let put tile down on tile!
- 1 tile item per cell?
- foo is moving more than a single space equivalent per step, sometimes up to 3 spaces!?
- foo drawn over player is bad? idea: set occupancy state of cells?
- sprites!
- easter-egg tiles =)
- pass a world variable with things like "game time"...
- AI for helper chick
- clean up facing and cardinal direction
- change exists to tiled
*/


#include "include.h"

#include <vector>
#include <cmath>
#include <iostream>

#include <allegro5\allegro.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_native_dialog.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_image.h> // image add-on library

//https://www.allegro.cc/manual/5/keyboard.html
enum KEYS{ UP, DOWN, LEFT, RIGHT, W, A, S, D, LSHIFT, SPACE, RCTRL, END, P};



int main(void)
{

  Cell testcell(0,0);
  std::cout << testcell.exists << "; " << std::endl;

  Arena arena(2,3);
  for ( int i = 0; i < arena.cell_vector.size(); i++ )
    std::cout << arena.cell_vector[i].exists;
  arena.set(0,1,1);
  arena.set(0,2,0);
  std::cout << std::endl << std::endl << arena.cell(0,1).exists << arena.cell(0,2).exists;

  srand(time(0));

  int width = 800;
	int height = 600;

  bool pause = true;
	bool done = false;
  bool redraw = true;

  bool game_over = false;
  bool game_win = false;

  int FPS = 60;

  bool keys[13] = {false, false, false, false, false, false, false, false, false, false, false, false, false};

	ALLEGRO_DISPLAY *display = NULL;
  //http://wiki.allegro.cc/index.php?title=Basic_tutorial_on_loading_and_showing_images
  ALLEGRO_BITMAP *image_angrybird   = NULL;

  ALLEGRO_BITMAP *image_bird_north   = NULL;
  ALLEGRO_BITMAP *image_bird_south   = NULL;
  ALLEGRO_BITMAP *image_bird_west   = NULL;
  ALLEGRO_BITMAP *image_bird_east   = NULL;

  ALLEGRO_EVENT_QUEUE *event_queue = NULL;
  ALLEGRO_TIMER *timer = NULL;
  int game_time = 0; //my variable

  const int grid_offset_x = 10;
  const int grid_offset_y = 10;

  const int grid_cells_size = 64;
  const int grid_cells_x = 7;
  const int grid_cells_y = 7;

  Cell grid[grid_cells_x][grid_cells_y];
  for ( int x = 0; x < grid_cells_x; x++ ) {
    for ( int y = 0; y < grid_cells_y; y++ ) {
      grid[x][y] = Cell();
    }
  }

  Nest nest(grid_cells_x/2,grid_cells_y/2);
  grid[nest.obj.x][nest.obj.y] = Cell(1,0);
  grid[nest.obj.x-1][nest.obj.y] = Cell(1,0);
  grid[nest.obj.x+1][nest.obj.y] = Cell(1,0);
  grid[nest.obj.x][nest.obj.y-1] = Cell(1,0);
  grid[nest.obj.x][nest.obj.y+1] = Cell(1,0);
  grid[nest.obj.x-2][nest.obj.y] = Cell(1,0);
  grid[nest.obj.x+2][nest.obj.y] = Cell(1,0);
  grid[nest.obj.x][nest.obj.y-2] = Cell(1,0);
  grid[nest.obj.x][nest.obj.y+2] = Cell(1,0);
  grid[nest.obj.x-1][nest.obj.y-1] = Cell(1,0);
  grid[nest.obj.x-1][nest.obj.y+1] = Cell(1,0);
  grid[nest.obj.x+1][nest.obj.y-1] = Cell(1,0);
  grid[nest.obj.x+1][nest.obj.y+1] = Cell(1,0);

  std::vector<Player> player;
  player.reserve(2);
  player.push_back(Player(nest.obj.x-1,nest.obj.y,0,0,0));
  player.push_back(Player(nest.obj.x+1,nest.obj.y,0,0,0));

  std::vector<Foo> foo;
  int count_since_last_foo_spawned = 0;
  
  std::vector<Item> item;
  for ( int i = 0; i < 10; i++ )
    item.push_back(Item(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1)));
  int count_since_last_item_spawned = 0;

  //initialize allegro
	if(!al_init()) {
		al_show_native_message_box(NULL, NULL, NULL, "failed to initialize allegro!", NULL, NULL);                   
		return -1;
	}

  if(!al_init_image_addon()) {
    al_show_native_message_box(NULL, NULL, NULL, "failed to initialize image addon!", NULL, NULL);                   
		return -1;
  }

  //create our display object
	display = al_create_display(width, height);

  //test display object
	if(!display) {
		al_show_native_message_box(NULL, NULL, NULL, "failed to initialize display!", NULL, NULL);
		return -1;
	}

  //If you load an image before the creation of a display
  //you would be loading it as a Memory Bitmap which is extremely slow,
  //for that reason remember always to load bitmaps after the creation of a display.
  image_angrybird = al_load_bitmap("google-eyes-black_32x32.png"/*"angrybirds_32x32.png"*/);
  if(!image_angrybird) {
    al_show_native_message_box(display, "Error", "Error", "Failed to load image!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    al_destroy_display(display);
    return 0;
  }
  //TODO:Should do a check like above on all of the following image loads?
  image_bird_north = al_load_bitmap("bird-north_32x32.png"); al_convert_mask_to_alpha(image_bird_north,al_map_rgb(255,0,255));
  image_bird_south = al_load_bitmap("bird-south_32x32.png"); al_convert_mask_to_alpha(image_bird_south,al_map_rgb(255,0,255));
  image_bird_east = al_load_bitmap("bird-east_32x32.png"); al_convert_mask_to_alpha(image_bird_east,al_map_rgb(255,0,255));
  image_bird_west = al_load_bitmap("bird-west_32x32.png"); al_convert_mask_to_alpha(image_bird_west,al_map_rgb(255,0,255));
  

	al_init_font_addon();
	al_init_ttf_addon();

  ALLEGRO_FONT *font12 = al_load_font("arial.ttf", 12, 0);
	ALLEGRO_FONT *font18 = al_load_font("arial.ttf", 18, 0);
	ALLEGRO_FONT *font24 = al_load_font("arial.ttf", 24, 0);
	ALLEGRO_FONT *font36 = al_load_font("arial.ttf", 36, 0);
  ALLEGRO_FONT *font48 = al_load_font("arial.ttf", 48, 0);
  ALLEGRO_FONT *font60 = al_load_font("arial.ttf", 60, 0);
  ALLEGRO_FONT *font72 = al_load_font("arial.ttf", 72, 0);


  al_init_primitives_addon();
  al_install_keyboard();
  al_install_mouse();


  event_queue = al_create_event_queue();
  timer = al_create_timer(1.0 / FPS);

  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));
  

  al_hide_mouse_cursor(display);
  al_start_timer(timer);
  while(!done)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

    if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true;
		}
    else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{
			if(ev.mouse.button & 1)
      {
      }
			else if (ev.mouse.button & 2)
      {
      }
		}
		else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES)
		{
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
        // PAUSE
        case ALLEGRO_KEY_P:
          keys[P] = true;
            pause = true;
          break;

				case ALLEGRO_KEY_UP:
					keys[UP] = true;
          player[0].obj.Facing(NORTH);
          player[0].facing_x = 0;
          player[0].facing_y = -1;
          if ( grid[player[0].obj.x][player[0].obj.y-1].exists && !grid[player[0].obj.x][player[0].obj.y-1].blocked && player[0].obj.y-1 >= 0 )
            player[0].obj.y--;
					break;
				case ALLEGRO_KEY_DOWN:
					keys[DOWN] = true;
          player[0].obj.Facing(SOUTH);
          player[0].facing_x = 0;
          player[0].facing_y = 1;
          if ( grid[player[0].obj.x][player[0].obj.y+1].exists && !grid[player[0].obj.x][player[0].obj.y+1].blocked && player[0].obj.y+1 < grid_cells_y )
            player[0].obj.y++;
					break;
				case ALLEGRO_KEY_RIGHT:
					keys[RIGHT] = true;
          player[0].obj.Facing(EAST);
          player[0].facing_x = 1;
          player[0].facing_y = 0;
          if ( grid[player[0].obj.x+1][player[0].obj.y].exists && !grid[player[0].obj.x+1][player[0].obj.y].blocked && player[0].obj.x+1 < grid_cells_x )
            player[0].obj.x++;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[LEFT] = true;
          player[0].obj.Facing(WEST);
          player[0].facing_x = -1;
          player[0].facing_y = 0;
          if ( grid[player[0].obj.x-1][player[0].obj.y].exists && !grid[player[0].obj.x-1][player[0].obj.y].blocked && player[0].obj.x-1 >= 0 )
            player[0].obj.x--;
					break;

        case ALLEGRO_KEY_RCTRL:
          keys[RCTRL] = true;
          if ( !player[0].has_block && grid[player[0].obj.x+player[0].facing_x][player[0].obj.y+player[0].facing_y].exists
            && !grid[player[0].obj.x+player[0].facing_x][player[0].obj.y+player[0].facing_y].blocked
            && player[0].obj.x+player[0].facing_x >= 0 && player[0].obj.x+player[0].facing_x < grid_cells_x
            && player[0].obj.y+player[0].facing_y >= 0 && player[0].obj.y+player[0].facing_y < grid_cells_y )
          {
            player[0].has_block = true;
            grid[player[0].obj.x+player[0].facing_x][player[0].obj.y+player[0].facing_y].exists = false;
            for ( int i = 0; i < (signed)foo.size(); i++ )
            {
              if ( foo[i].obj.x == player[0].obj.x+player[0].facing_x && foo[i].obj.y == player[0].obj.y+player[0].facing_y )
              {
                foo.erase(foo.begin()+i);
                item.push_back(Item(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1))); // BONUS TILES!
              }
            }
          }
          else if ( player[0].has_block && !grid[player[0].obj.x+player[0].facing_x][player[0].obj.y+player[0].facing_y].exists
            && player[0].obj.x+player[0].facing_x >= 0 && player[0].obj.x+player[0].facing_x < grid_cells_x
            && player[0].obj.y+player[0].facing_y >= 0 && player[0].obj.y+player[0].facing_y < grid_cells_y )
          {
            player[0].has_block = false;
            grid[player[0].obj.x+player[0].facing_x][player[0].obj.y+player[0].facing_y].exists = true;
            for ( int i = 0; i < (signed)foo.size(); i++ )
            {
              if ( foo[i].obj.x == player[0].obj.x+player[0].facing_x && foo[i].obj.y == player[0].obj.y+player[0].facing_y )
              {
                foo.erase(foo.begin()+i);
                item.push_back(Item(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1))); // BONUS TILES!
              }
            }
          }
          else if ( player[0].has_block && grid[player[0].obj.x+player[0].facing_x][player[0].obj.y+player[0].facing_y].exists
            && player[0].obj.x+player[0].facing_x >= 0 && player[0].obj.x+player[0].facing_x < grid_cells_x
            && player[0].obj.y+player[0].facing_y >= 0 && player[0].obj.y+player[0].facing_y < grid_cells_y )
          {
            player[0].has_block = false;
            item.push_back(Item(player[0].obj.x+player[0].facing_x,player[0].obj.y+player[0].facing_y));
            for ( int i = 0; i < (signed)foo.size(); i++ )
            {
              if ( foo[i].obj.x == player[0].obj.x+player[0].facing_x && foo[i].obj.y == player[0].obj.y+player[0].facing_y )
                foo.erase(foo.begin()+i);
            }
          }
          break;

        case ALLEGRO_KEY_W:
					keys[W] = true;
          player[1].obj.Facing(NORTH);
          player[1].facing_x = 0;
          player[1].facing_y = -1;
          if ( grid[player[1].obj.x][player[1].obj.y-1].exists && !grid[player[1].obj.x][player[1].obj.y-1].blocked && player[1].obj.y-1 >= 0 )
            player[1].obj.y--;
					break;
				case ALLEGRO_KEY_S:
					keys[S] = true;
          player[1].obj.Facing(SOUTH);
          player[1].facing_x = 0;
          player[1].facing_y = 1;
          if ( grid[player[1].obj.x][player[1].obj.y+1].exists && !grid[player[1].obj.x][player[1].obj.y+1].blocked && player[1].obj.y+1 < grid_cells_y )
            player[1].obj.y++;
					break;
				case ALLEGRO_KEY_D:
					keys[D] = true;
          player[1].obj.Facing(EAST);
          player[1].facing_x = 1;
          player[1].facing_y = 0;
          if ( grid[player[1].obj.x+1][player[1].obj.y].exists && !grid[player[1].obj.x+1][player[1].obj.y].blocked && player[1].obj.x+1 < grid_cells_x )
            player[1].obj.x++;
					break;
				case ALLEGRO_KEY_A:
					keys[A] = true;
          player[1].obj.Facing(WEST);
          player[1].facing_x = -1;
          player[1].facing_y = 0;
          if ( grid[player[1].obj.x-1][player[1].obj.y].exists && !grid[player[1].obj.x-1][player[1].obj.y].blocked && player[1].obj.x-1 >= 0 )
            player[1].obj.x--;
					break;

        case ALLEGRO_KEY_LSHIFT:
          keys[LSHIFT] = true;
          if ( !player[1].has_block && grid[player[1].obj.x+player[1].facing_x][player[1].obj.y+player[1].facing_y].exists
            && !grid[player[1].obj.x+player[1].facing_x][player[1].obj.y+player[1].facing_y].blocked
            && player[1].obj.x+player[1].facing_x >= 0 && player[1].obj.x+player[1].facing_x < grid_cells_y
            && player[1].obj.y+player[1].facing_y >= 0 && player[1].obj.y+player[1].facing_y < grid_cells_y )
          {
            player[1].has_block = true;
            grid[player[1].obj.x+player[1].facing_x][player[1].obj.y+player[1].facing_y].exists = false;
            for ( int i = 0; i < (signed)foo.size(); i++ )
            {
              if ( foo[i].obj.x == player[1].obj.x+player[1].facing_x && foo[i].obj.y == player[1].obj.y+player[1].facing_y )
              {
                foo.erase(foo.begin()+i);
                item.push_back(Item(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1))); // BONUS BLOCK!
              }
            }
          }
          else if ( player[1].has_block && !grid[player[1].obj.x+player[1].facing_x][player[1].obj.y+player[1].facing_y].exists
            && player[1].obj.x+player[1].facing_x >= 0 && player[1].obj.x+player[1].facing_x < grid_cells_y
            && player[1].obj.y+player[1].facing_y >= 0 && player[1].obj.y+player[1].facing_y < grid_cells_y )
          {
            player[1].has_block = false;
            grid[player[1].obj.x+player[1].facing_x][player[1].obj.y+player[1].facing_y].exists = true;
            for ( int i = 0; i < (signed)foo.size(); i++ )
            {
              if ( foo[i].obj.x == player[1].obj.x+player[1].facing_x && foo[i].obj.y == player[1].obj.y+player[1].facing_y )
              {
                foo.erase(foo.begin()+i);
                item.push_back(Item(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1))); // BONUS BLOCK!
              }
            }
          }
          else if ( player[1].has_block && grid[player[1].obj.x+player[1].facing_x][player[1].obj.y+player[1].facing_y].exists
            && player[1].obj.x+player[1].facing_x >= 0 && player[1].obj.x+player[1].facing_x < grid_cells_y
            && player[1].obj.y+player[1].facing_y >= 0 && player[1].obj.y+player[1].facing_y < grid_cells_y )
          {
            player[1].has_block = false;
            item.push_back(Item(player[0].obj.x+player[0].facing_x,player[0].obj.y+player[0].facing_y));
            for ( int i = 0; i < (signed)foo.size(); i++ )
            {
              if ( foo[i].obj.x == player[1].obj.x+player[1].facing_x && foo[i].obj.y == player[1].obj.y+player[1].facing_y )
                foo.erase(foo.begin()+i);
            }
          }
          break;
			}
		}//ALLEGRO_EVENT_KEY_DOWN

		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			switch(ev.keyboard.keycode){
        case ALLEGRO_KEY_P: keys[P] = false; break;
				case ALLEGRO_KEY_UP: keys[UP] = false; break;
				case ALLEGRO_KEY_DOWN: keys[DOWN] = false; break;
				case ALLEGRO_KEY_RIGHT: keys[RIGHT] = false; break;
				case ALLEGRO_KEY_LEFT: keys[LEFT] = false; break;
        case ALLEGRO_KEY_RCTRL: keys[RCTRL] = false; break;
        case ALLEGRO_KEY_END: keys[END] = false; break;
        case ALLEGRO_KEY_W: keys[W] = false; break;
				case ALLEGRO_KEY_A: keys[A] = false; break;
				case ALLEGRO_KEY_S: keys[S] = false; break;
				case ALLEGRO_KEY_D: keys[D] = false; break;
        case ALLEGRO_KEY_LSHIFT: keys[LSHIFT] = false; break;
        case ALLEGRO_KEY_SPACE: keys[SPACE] = false; break;
				case ALLEGRO_KEY_ESCAPE: done = true; break;
			}
		}


    // TIMER EVENT
    else if(ev.type == ALLEGRO_EVENT_TIMER)
		{
      game_time++;
			redraw = true;

      bool grid_fully_tiled = true;
      for ( int x = 0; x < grid_cells_x; x++ ) {
        for ( int y = 0; y < grid_cells_y; y++ ) {
          if ( !grid[x][y].exists )
            grid_fully_tiled = false;
        }
      }
      if ( grid_fully_tiled && !game_win )
        game_win = true;


      for ( int i = 0; i < (signed)foo.size(); i++ )
      {
        foo[i].Update(nest.obj);
        if ( grid[foo[i].obj.x][foo[i].obj.y].blocked )
        {
          grid[foo[i].obj.x][foo[i].obj.y].blocked = false;
          foo.erase(foo.begin()+i);
        }
      }

      count_since_last_foo_spawned++;
      if ( !game_win && count_since_last_foo_spawned > 4800 / log((double)game_time) ) //spawn every 60 seconds and increasingly frequently //TODO: CHECK THIS FREQUENCY
      {
        Foo new_foo(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1));
        while ( grid[new_foo.obj.x][new_foo.obj.y].exists && !grid_fully_tiled ) // Foos won't spawn on existing tiles. And won't try if all tiled.
          new_foo = Foo(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1));
        foo.push_back(new_foo);

        count_since_last_foo_spawned = 0;
      }

      count_since_last_item_spawned++;
      if ( count_since_last_item_spawned > 300 ) //spawn every 5sec
      {
        item.push_back(Item(random_int(0,grid_cells_x-1),random_int(0,grid_cells_y-1)));
        count_since_last_item_spawned = 0;
      }

      //COLLISION DETECTION:
      
      // FOO + NEST
      for ( int j = 0; j < (signed)foo.size(); j++ )
      {
        if ( nest.obj.x == foo[j].obj.x && nest.obj.y == foo[j].obj.y )
        {
          game_over = true;
        }
      }
      
      // PLAYER + ...
      for ( int i = 0; i < (signed)player.size(); i++ )
      {
        // PLAYER + FOO
        for ( int j = 0; j < (signed)foo.size(); j++ )
        {
          if ( player[i].obj.x == foo[j].obj.x && player[i].obj.y == foo[j].obj.y )
          {
            player[i].coins--;
          }
        }

        //PLAYER + ITEM
        for ( int j = 0; j < (signed)item.size(); j++ )
        {
          if ( player[i].obj.x == item[j].obj.x && player[i].obj.y == item[j].obj.y )
          {
            if ( player[i].has_block == false )
            {
              player[i].has_block = true;
              item.erase(item.begin()+j);
            }
          }
        }
      }
		}

    while ( pause && !done )
    {
      al_stop_timer(timer);

      // DRAW PAUSE SCREEN
      al_draw_text( font48, al_map_rgb(200, 200, 200), width/2, 10, ALLEGRO_ALIGN_CENTRE, "COOPERATIVE SURVIVAL GAME" );
      al_draw_text( font18, al_map_rgb(0, 102, 255), 10, 70, 0, "You and your friend are 2 birds building and protecting your nest from baddies." );
      al_draw_text( font12, al_map_rgb(0, 102, 255), 20, 90, 0, "PLAYER 1: ARROW KEYS and RCTRL (action button)" );
      al_draw_text( font12, al_map_rgb(0, 102, 255), 20, 105, 0, "PLAYER 2: WASD and LSHIFT (action button)" );
      al_draw_text( font12, al_map_rgb(0, 102, 255), 20, 120, 0, "You are both invulnerable to baddies, but your nest is not." );
      al_draw_text( font18, al_map_rgb(102, 51, 0), 10, 135, 0, "You both lose if baddies reach your nest (brown tile in center of arena)." );
      al_draw_text( font12, al_map_rgb(102, 51, 0), 20, 155, 0, "Baddies can be anywhere but they spawn where there are no tiles." );
      al_draw_text( font18, al_map_rgb(127, 127, 127), 10, 175, 0, "You may only stand where there is a tile." );
      al_draw_text( font18, al_map_rgb(127, 127, 127), 10, 195, 0, "You can pickup or drop a tile in the direction you are facing with your action button." );
      al_draw_text( font12, al_map_rgb(127, 127, 127), 20, 215, 0, "Pickup occurs if you are facing a tile and aren't already holding a tile." );
      al_draw_text( font12, al_map_rgb(127, 127, 127), 20, 230, 0, "Drop occurs if you are holding a tile and facing an empty space." );
      al_draw_text( font18, al_map_rgb(255, 0, 255), 10, 245, 0, "Kill baddies by picking or dropping a tile where they lurk." );
      
      al_draw_text( font18, al_map_rgb(255, 255, 0), 10, 285, 0, "This is actually the PAUSE screen..." );
      al_draw_text( font12, al_map_rgb(255, 255, 0), 20, 300, 0, "Press P to begin the game, and press P at any time to PAUSE or UNPAUSE the game." );

      al_flip_display();
		  al_clear_to_color(al_map_rgb(0,0,0));
      al_wait_for_event(event_queue, &ev);

      //al_wait_for_event(event_queue, &ev);
		  if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		  {
			  switch(ev.keyboard.keycode)
			  {
          // PAUSE
          case ALLEGRO_KEY_P:
            keys[P] = true;
              pause = false;
              al_start_timer(timer);
            break;
        }
      }
      else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			  switch(ev.keyboard.keycode)
        {
				  case ALLEGRO_KEY_ESCAPE: done = true; break;
			  }
      }

    } 



    if( redraw && al_is_event_queue_empty(event_queue) )
    {
      redraw = false;

      // DRAW GRID CELLS
      for ( int y = 0; y < grid_cells_y; y++ ) {
        for ( int x = 0; x < grid_cells_x; x++ ) {
          if ( grid[x][y].blocked )
            al_draw_filled_rounded_rectangle( grid_offset_x+x*grid_cells_size, grid_offset_y+y*grid_cells_size, grid_offset_x+x*grid_cells_size+grid_cells_size, grid_offset_y+y*grid_cells_size+grid_cells_size, grid_cells_size/4, grid_cells_size/4, al_map_rgb(255,127,0) );
          else if ( grid[x][y].exists )
            al_draw_filled_rounded_rectangle( grid_offset_x+x*grid_cells_size, grid_offset_y+y*grid_cells_size, grid_offset_x+x*grid_cells_size+grid_cells_size, grid_offset_y+y*grid_cells_size+grid_cells_size, grid_cells_size/4, grid_cells_size/4, al_map_rgb(96+16*((x+y+1)%2),96+16*((x+y+1)%2),96+16*((x+y+1)%2)) );
          else
            al_draw_filled_rounded_rectangle( grid_offset_x+x*grid_cells_size, grid_offset_y+y*grid_cells_size, grid_offset_x+x*grid_cells_size+grid_cells_size, grid_offset_y+y*grid_cells_size+grid_cells_size, grid_cells_size/4, grid_cells_size/4, al_map_rgb(127,16,127) );
        }
      }

      // DRAW NEST
      int x = nest.obj.x;
      int y = nest.obj.y;
      al_draw_filled_rounded_rectangle( grid_offset_x+( x)*grid_cells_size+2, grid_offset_y+( y)*grid_cells_size+2,
        grid_offset_x+( x)*grid_cells_size+grid_cells_size-2, grid_offset_y+( y)*grid_cells_size+grid_cells_size-2,
        grid_cells_size/4, grid_cells_size/4, al_map_rgb(102,51,0) );
      al_draw_text( font12, al_map_rgb(0, 0, 0), grid_offset_x+( x+0.5)*grid_cells_size, grid_offset_y+( y+.25)*grid_cells_size, ALLEGRO_ALIGN_CENTRE, "nest" );

      // DRAW PLAYERS
      for ( int i = 0; i < (signed)player.size(); i++ )
      {
        int x = player[i].obj.x;
        int y = player[i].obj.y;

        // Draw block if PLAYER has one, bottom-most image layer.
        if ( player[i].has_block )
          al_draw_filled_rounded_rectangle( grid_offset_x+(x+.1)*grid_cells_size, grid_offset_y+(y+.1)*grid_cells_size,
          grid_offset_x+(x+.9)*grid_cells_size, grid_offset_y+(y+.9)*grid_cells_size,
          grid_cells_size/8, grid_cells_size/8, al_map_rgb(255,255,255) );
        //al_draw_textf( font12, al_map_rgb(0, 0, 0), grid_offset_x+(x+0.5)*grid_cells_size, grid_offset_y+(y+.25)*grid_cells_size, ALLEGRO_ALIGN_CENTRE, "%d", i+1 );

        // Draw PLAYER birds.
        if ( player[i].obj.cardinal_direction == EAST )
          al_draw_bitmap(image_bird_east,grid_offset_x+(x)*grid_cells_size, grid_offset_y+(y)*grid_cells_size,0);
        else if ( player[i].obj.cardinal_direction == WEST )
          al_draw_bitmap(image_bird_west,grid_offset_x+(x)*grid_cells_size, grid_offset_y+(y)*grid_cells_size,0);
        else if ( player[i].obj.cardinal_direction == NORTH )
          al_draw_bitmap(image_bird_north,grid_offset_x+(x)*grid_cells_size, grid_offset_y+(y)*grid_cells_size,0);
        else if ( player[i].obj.cardinal_direction == SOUTH )
          al_draw_bitmap(image_bird_south,grid_offset_x+(x)*grid_cells_size, grid_offset_y+(y)*grid_cells_size,0);
        else
        {
          al_draw_filled_rounded_rectangle( grid_offset_x+(x+.25)*grid_cells_size, grid_offset_y+(y+.25)*grid_cells_size, grid_offset_x+(x+.75)*grid_cells_size, grid_offset_y+(y+.75)*grid_cells_size, grid_cells_size/4, grid_cells_size/4, al_map_rgb(64+i*191,64,255-i*128) );
          al_draw_textf( font12, al_map_rgb(0, 0, 0), grid_offset_x+(x+0.5)*grid_cells_size, grid_offset_y+(y+.25)*grid_cells_size, ALLEGRO_ALIGN_CENTRE, "NONE" );
        }
      }

      // DRAW ITEMS
      for ( int i = 0; i < (signed)item.size(); i++ )
        al_draw_filled_rounded_rectangle( grid_offset_x+(item[i].obj.x+.25)*grid_cells_size, grid_offset_y+(item[i].obj.y+.25)*grid_cells_size, grid_offset_x+(item[i].obj.x-.25)*grid_cells_size+grid_cells_size, grid_offset_y+(item[i].obj.y-.25)*grid_cells_size+grid_cells_size, grid_cells_size/4, grid_cells_size/4, al_map_rgb(255,255,255) );
      
      // DRAW FOO
      for ( int i = 0; i < (signed)foo.size(); i++ )
      {
        //al_draw_filled_rounded_rectangle( grid_offset_x+(foo[i].obj.x+.25)*grid_cells_size, grid_offset_y+(foo[i].obj.y+.25)*grid_cells_size, grid_offset_x+(foo[i].obj.x-.25)*grid_cells_size+grid_cells_size, grid_offset_y+(foo[i].obj.y-.25)*grid_cells_size+grid_cells_size, grid_cells_size/4, grid_cells_size/4, al_map_rgb(255,0,255) );
        al_draw_bitmap(image_angrybird,grid_offset_x+(foo[i].obj.x)*grid_cells_size, grid_offset_y+(foo[i].obj.y)*grid_cells_size,0);
      }

      // DRAW TEXT
      al_draw_textf( font18, al_map_rgb(255, 255, 255), 600, 0, 0, "time: %d", game_time );
      al_draw_textf( font18, al_map_rgb(255, 255, 255), 600, 20, 0, "log(time): %f", log((double)game_time) );
      al_draw_textf( font18, al_map_rgb(255, 255, 255), 600, 40, 0, "Foo period: %f", 2400 / log((double)game_time) );
      if ( game_win )
        al_draw_text( font72, al_map_rgb(255, 255, 0), width / 2, height / 2, ALLEGRO_ALIGN_CENTRE, "YOU WIN!" );
      if ( game_over )
        al_draw_text( font72, al_map_rgb(255, 0, 0), width / 2, height / 2, ALLEGRO_ALIGN_CENTRE, "GAME OVER" );

		  al_flip_display();
		  al_clear_to_color(al_map_rgb(0,0,0));
    }



	}

  /*
	al_clear_to_color(al_map_rgb(0,0,0));

  al_draw_line(100, 100, width - 100, 100, al_map_rgb(255, 0, 0), 1);
	al_draw_line(50, 200, width - 50, 200, al_map_rgb(0, 0, 255), 5);
	al_draw_line(0, 300, width, 300, al_map_rgb(255, 0, 255), 10);
	al_draw_line(50, 50, 50, 400, al_map_rgb(0, 255, 0), 3);
	al_draw_line(50, 200, width - 50, 400, al_map_rgb(0, 0, 255), 5);

  al_draw_triangle(10, 200, 100, 10, 190, 200, al_map_rgb(255, 0, 255), 5);
	al_draw_filled_triangle(300, 400, 400, 200, 500, 400, al_map_rgb(0, 0, 255));

  al_draw_rectangle(10, 10, 250, 250, al_map_rgb(255, 0, 255), 5);
	al_draw_rounded_rectangle(width - 200, 10, width - 10, 50, 5, 5, al_map_rgb(0, 0, 255), 15);
	al_draw_filled_rectangle(10, 280, 250, height - 10, al_map_rgb(255, 255, 255));
	al_draw_filled_rounded_rectangle(width - 200, 180, width - 10, height - 10, 10, 10,  al_map_rgb(0, 255, 0));

  al_draw_circle(100, 100, 50, al_map_rgb(255, 255, 0), 7);
	al_draw_filled_circle(400, 400, 70, al_map_rgb(255, 255, 255));

  al_draw_ellipse(150, 100, 100, 50, al_map_rgb(127, 3, 34), 7);
	al_draw_filled_ellipse(400, 250, 100, 200, al_map_rgb(0, 255, 255));

  float points[] = {0, 0, 400, 100, 50, 200, width, height};
	al_draw_spline(points, al_map_rgb(255, 0, 255), 0);

	float points2[] = {0, height, 200, 100, 400, 200, width, height};
	al_draw_spline(points2, al_map_rgb(0, 0, 255), 3);

	al_draw_text(font24, al_map_rgb(255, 0, 255), 50, 50, 0, "Hello World, this is 24 point");
	al_draw_text(font36, al_map_rgb(255, 127, 127), width / 2, height / 2, ALLEGRO_ALIGN_CENTRE, "This is Centered and 36 point");
	al_draw_text(font18, al_map_rgb(15, 240, 18), width - 20, 350, ALLEGRO_ALIGN_RIGHT, "This is right aligned and 18 point");

	int screen_w = al_get_display_width(display);
	int screen_h = al_get_display_height(display);

	al_draw_textf(font18, al_map_rgb(255, 255, 255), screen_w/2, 400, ALLEGRO_ALIGN_CENTRE,
		"TEXT with variable output (textf): Screen width and height = %i / %i" , screen_w, screen_h);

	al_flip_display();

	al_rest(5.0);
  */



	al_destroy_font(font18);
	al_destroy_font(font24);
	al_destroy_font(font36);

  al_destroy_event_queue(event_queue);
  al_destroy_timer(timer);
	al_destroy_display(display); //destroy our display object
	
	return 0;
}