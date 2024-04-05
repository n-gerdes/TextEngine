#include <iostream>
#include <list>
#include "engine/headers/savable.h"
#include "engine/headers/engine.h"
#include <random>
#include "engine/headers/util/string_utils.h"

inline void launch_game_engine()
{
	srand(time(NULL));
	engine* game_engine = new engine();
	game_engine->main_menu(); //This starts the game
	delete game_engine; //After the game has finished, delete the game engine game_object & all associated resources.
}

int main(int argc, char** args)
{
	launch_game_engine();
}
