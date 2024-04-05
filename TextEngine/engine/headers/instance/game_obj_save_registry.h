#pragma once
#include <map>
#include <iostream>
#include "game_obj.h"

class game_obj_save_registry
{
	std::map<uint32_t, game_obj*> save_id_registry;

public:
	void register_save_id(game_obj* class_instance);

	void cleanup_registry();
	game_obj* get_instance(uint32_t id);
	game_obj_save_registry();
};
