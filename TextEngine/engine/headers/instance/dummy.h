#ifndef GAME_OBJ_VARIABLE_H
#define GAME_OBJ_VARIABLE_H
#include "engine/headers/instance/game_obj.h"

//Basically a dummy class to give objects extra variables / containers when they need them.
class dummy : public game_obj
{
	GAME_OBJ_SUBCLASS(dummy, 4)

public:
	dummy(const std::string& dummy_name);
};

#endif