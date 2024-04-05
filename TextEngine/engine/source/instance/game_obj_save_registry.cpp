#include "engine/headers/instance/game_obj_save_registry.h"
#include "engine/headers/instance/game_obj.h"

#include "engine/headers/instance/game.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/instance/dummy.h"
#include "engine/headers/instance/scene.h"
//#include "engine/headers/instance/scene/event_scene.h"

void game_obj_save_registry::register_save_id(game_obj* class_instance)
{
	uint32_t id = class_instance->get_save_id();
	if (save_id_registry.count(id) != 0)
	{
		std::cout << "Error: Tried to register more than one class with save ID " << id << std::endl;
	}
	else
	{
		save_id_registry.insert_or_assign(id, class_instance);
	}
}

void game_obj_save_registry::cleanup_registry()
{
	for (auto i = save_id_registry.begin(); i != save_id_registry.end(); ++i)
	{
		(i->second)->destroy();
	}
}

game_obj* game_obj_save_registry::get_instance(uint32_t id)
{
	if (save_id_registry.count(id) == 0)
		return nullptr;
	game_obj* instance = save_id_registry.at(id)->instance();
	instance->was_instanced_from_file = true;
	return instance;
}

game_obj_save_registry::game_obj_save_registry()
{
	register_save_id(new game());
	//register_save_id(new event_scene());
	register_save_id(new scene());
	register_save_id(new entity());
	register_save_id(new dummy());
	
}
