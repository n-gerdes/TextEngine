#include "engine/headers/engine.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/dummy.h"
#include "engine/headers/instance/res_file.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/instance/scene.h"
#include "engine/headers/procedure/file_reader.h"
#include <thread>

game::game()
{
	initialize();
	set_name("UNNAMED GAME");
}

void game::game_loop()
{
	//scene* current_scene = get_current_scene();
	int threads_launched = 0;
	int* ptr = &threads_launched;
	while (game_going)
	{
		const auto& entities = get(this, "entities")->get_children();

		/*
		auto last_entity = entities.end();
		--last_entity;
		if ((*last_entity) != get_perspective_entity())
		{
			auto perspective = entities.begin();
			for (perspective = entities.begin(); perspective != entities.end(); ++perspective)
			{
				if ((*perspective) == get_perspective_entity())
					break;
			}
			std::swap(last_entity, perspective);
		}
		*/
		const auto& scenes = get(this, "scenes")->get_children();
		for (auto scene_iterator = scenes.begin(); scene_iterator != scenes.end(); ++scene_iterator)
		{
			scene* current_scene = dynamic_cast<scene*>(*scene_iterator);
			current_scene->launch(this, ptr);
		}

		/*
		const auto& entities = get("entities")->get_children();
		
		for (auto entity_iterator = entities.begin(); entity_iterator != entities.end(); ++entity_iterator)
		{
			entity* current_entity = dynamic_cast<entity*>(*entity_iterator);
			if (current_entity)
			{
				bool success = current_entity->take_turn(this);
				while (!success && current_entity == get_perspective_entity())
				{
					get_engine()->println("Vague or unclear.");
					success = current_entity->take_turn(this);
				}

				if (!game_going)
					return;
			}
		}
		*/
	}
	get_engine()->println("Game has ended.");
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	while (threads_launched > 0)
	{
		//std::cout << "Threads: " << threads_launched << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void game::end_game()
{
	game_going = false;
}

void game::initialize()
{
	add_child(new dummy("entities"));
	add_child(new dummy("scenes"));
}

void game::save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const
{
	save_uint32_t(file, game_version);
	save_bool(file, save_any_time);
	save_bool(file, clear_on_scene_change);
	save_bool(file, custom_input_substitution_overrides_engine);
	save_string(file, perspective_entity);
}

void game::load_variables(std::ifstream& file, const std::string& scenario_name, engine* engine)
{
	uint32_t loaded_game_version = 0;
	load_uint32_t(file, loaded_game_version);
	load_bool(file, save_any_time);
	load_bool(file, clear_on_scene_change);
	load_bool(file, custom_input_substitution_overrides_engine);
	load_string(file, perspective_entity);
}

bool game::entity_exists(const std::string& name)
{
	const auto& l = get_entities();
	for (size_t i = 0; i < l.size(); ++i)
	{
		if (l[i]->get_name() == name)
			return true;
	}
	return false;
}

bool game::game_is_active() const
{
	return game_going;
}

entity* game::get_any_entity(std::string name, const std::string& source)
{
	string_utils string_utils;
	name = string_utils.replace_all(name, variable_value_header, "", false);
	name = string_utils.replace_all(name, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (c->get_name() == name || c->has_alias(this, name))
				possible_entities.push_back(c);
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return load_entity_from_file(name, source, source);
	}
	else
	{
		return possible_entities[rand() % possible_entities.size()];
	}
}

entity* game::get_any_entity_in_scene(std::string entity_name, const std::string& scene_name, const std::string& source)
{
	string_utils string_utils;
	entity_name = string_utils.replace_all(entity_name, variable_value_header, "", false);
	entity_name = string_utils.replace_all(entity_name, var_val_space, " ", false);
	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (  (c->get_name() == entity_name || c->has_alias(this, entity_name)  )  && (   c->get_scene_name() == scene_name) && !c->in_transfer_queue()  )
				possible_entities.push_back(c);
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return possible_entities[rand() % possible_entities.size()];
	}
}

std::vector<entity*> game::get_entities()
{
	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		throw;
	}

	const auto& entities = entity_folder->get_children();

	std::vector<entity*> return_val;
	return_val.reserve(entities.size());

	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* ent = dynamic_cast<entity*>(*i);
		if (ent)
			return_val.push_back(ent);
	}

	return return_val;
}

std::vector<scene*> game::get_scenes()
{
	game_obj* entity_folder = find_first_child(this, "scenes");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		throw;
	}

	const auto& entities = entity_folder->get_children();

	std::vector<scene*> return_val;
	return_val.reserve(entities.size());

	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		scene* ent = dynamic_cast<scene*>(*i);
		if (ent)
			return_val.push_back(ent);
	}

	return return_val;
}

entity* game::get_entity(std::string name, bool allow_alias, const std::string& source)
{
	string_utils string_utils;
	name = string_utils.replace_all(name, variable_value_header, "", false);
	name = string_utils.replace_all(name, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (c->get_name() == name || (allow_alias && c->has_alias(this, name)))
				possible_entities.push_back(c);
			if (possible_entities.size() == 2)
				return get_engine()->AMBIGUOUS_CHARACTER();
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return load_entity_from_file(name, source, true);
	}
	else
	{
		return possible_entities[0];
	}
}

entity* game::get_entity_in_scene(std::string entity_name, const std::string& scene_name, bool allow_alias, const std::string& source)
{
	string_utils string_utils;
	entity_name = string_utils.replace_all(entity_name, variable_value_header, "", false);
	entity_name = string_utils.replace_all(entity_name, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if ((c->get_name() == entity_name || (allow_alias && c->has_alias(this, entity_name))) && (c->get_scene_name() == scene_name) && !c->in_transfer_queue())
			{
				possible_entities.push_back(c);
				if (!allow_alias)
					return c;
			}
			
			if (possible_entities.size() == 2)
			{
				return get_engine()->AMBIGUOUS_CHARACTER();
			}
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return possible_entities[0];
	}
}

entity* game::get_first_entity(std::string name, const std::string& source)
{
	string_utils string_utils;
	name = string_utils.replace_all(name, variable_value_header, "", false);
	name = string_utils.replace_all(name, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (c->get_name() == name || c->has_alias(this, name))
				return c;
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	return load_entity_from_file(name, source, true);
}

entity* game::get_first_entity_in_scene(std::string entity_name, const std::string& scene_name, const std::string& source)
{
	string_utils string_utils;
	entity_name = string_utils.replace_all(entity_name, variable_value_header, "", false);
	entity_name = string_utils.replace_all(entity_name, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if ((c->get_name() == entity_name || c->has_alias(this, entity_name)) && (c->get_scene_name() == scene_name) && !c->in_transfer_queue())
				return c;
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	return nullptr;
}

std::string game::get_scenario_directory() const
{
	return "Scenarios/" + get_name() + "/";
}

entity* game::get_any_entity(std::string alias, entity* seeker, const std::string& source)
{
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	alias = string_utils.replace_all(alias, var_val_space, " ", false);
	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (seeker->knows_alias(alias, c))
				possible_entities.push_back(c);
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return possible_entities[rand() % possible_entities.size()];
	}
}

entity* game::get_any_entity_in_scene(std::string alias, const std::string& scene_name, entity* seeker, const std::string& source)
{
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	alias = string_utils.replace_all(alias, var_val_space, " ", false);
	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if ((seeker->knows_alias(alias, c)) && (c->get_scene_name() == scene_name) && !c->in_transfer_queue())
				possible_entities.push_back(c);
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return possible_entities[rand() % possible_entities.size()];
	}
}

entity* game::get_entity(std::string alias, entity* seeker, const std::string& source)
{
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	alias = string_utils.replace_all(alias, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (seeker->knows_alias(alias, c))
				possible_entities.push_back(c);
			if (possible_entities.size() == 2)
				return get_engine()->AMBIGUOUS_CHARACTER();
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return possible_entities[0];
	}
}

entity* game::get_entity_in_scene(std::string alias, const std::string& scene_name, entity* seeker, const std::string& source)
{
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	alias = string_utils.replace_all(alias, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	std::vector<entity*> possible_entities;
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if ((seeker->knows_alias(alias, c)) && (c->get_scene_name() == scene_name) && !c->in_transfer_queue())
				possible_entities.push_back(c);

			if (possible_entities.size() == 2)
			{
				return get_engine()->AMBIGUOUS_CHARACTER();
			}
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	if (possible_entities.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return possible_entities[0];
	}
}

entity* game::get_entity_by_name(std::string name, const std::string& source)
{
	string_utils string_utils;
	name = string_utils.replace_all(name, variable_value_header, "", false);
	name = string_utils.replace_all(name, var_val_space, " ", false);

	auto found = name_to_entity.find(name);
	if (found == name_to_entity.end())
	{
		game_obj* entity_folder = find_first_child(this, "entities");
		if (entity_folder == nullptr)
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}

		const auto& entities = entity_folder->get_children();
		std::vector<entity*> possible_entities;
		for (auto i = entities.begin(); i != entities.end(); ++i)
		{
			entity* c = dynamic_cast<entity*>(*i);
			if (c)
			{
				if (c->get_name() == name)
				{
					name_to_entity_mutex.lock();
					name_to_entity.insert_or_assign(name, c);
					name_to_entity_mutex.unlock();
					return c;
				}
			}
			else
			{
				get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
				return nullptr;
			}
		}
		entity* loaded_from_file = load_entity_from_file(name, source, true);
		if (loaded_from_file != nullptr)
		{
			name_to_entity_mutex.lock();
			name_to_entity.insert_or_assign(name, loaded_from_file);
			name_to_entity_mutex.unlock();
		}
		//std::cout << "ATTEMTPING TO LOAD " << name << " FROM FILE\n";
		return loaded_from_file;
	}
	else
	{
		return found->second;
	}
}

uint64_t game::get_current_turn()
{
	if (get_perspective_entity() == nullptr)
	{
		return 10;
	}
	else
	{
		return get_perspective_entity()->get_turn_number();
	}
}

entity* game::get_first_entity(std::string alias, entity* seeker, const std::string& source)
{
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	alias = string_utils.replace_all(alias, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if (seeker->knows_alias(alias, c))
				return c;
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	return nullptr;
}

entity* game::get_first_entity_in_scene(std::string alias, const std::string& scene_name, entity* seeker, const std::string& source)
{
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	alias = string_utils.replace_all(alias, var_val_space, " ", false);

	game_obj* entity_folder = find_first_child(this, "entities");
	if (entity_folder == nullptr)
	{
		get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
		return nullptr;
	}

	const auto& entities = entity_folder->get_children();
	for (auto i = entities.begin(); i != entities.end(); ++i)
	{
		entity* c = dynamic_cast<entity*>(*i);
		if (c)
		{
			if ((seeker->knows_alias(alias, c)) && (c->get_scene_name() == scene_name) && !c->in_transfer_queue())
				return c;
		}
		else
		{
			get_engine()->println("A grave error has been encountered in game.cpp on line ", __LINE__);
			return nullptr;
		}
	}
	return nullptr;
}


scene* game::get_current_scene()
{
	return get_perspective_entity()->get_scene();
}

engine* game::get_engine() const
{
	return game_engine;
}

void game::describe_scene(scene* s)
{
	game* game_instance = s->get_game_instance();
	s->call_function(game_instance, "describe");
	auto siblings = s->get_entities_in_scene();
	for (auto i = siblings.begin(); i != siblings.end(); ++i)
	{
		entity* s = *i;
		if (s != game_instance->get_perspective_entity())
			s->call_function(game_instance, "describe");
	}
}

std::string game::get_meta_directory(const std::string& variable_name)
{
	return get_engine()->get_scenario_directory(get_name()) + "meta/" + variable_name + ".meta";
}

std::string game::get_meta_value(const std::string& variable_name)
{
	meta_variable_mutex.lock();
	std::string val;
	std::ifstream file;
	file.open(get_meta_directory(variable_name));
	if (file.is_open())
	{
		std::getline(file, val);
		file.close();
	}
	else
	{
		val = "0";
	}
	meta_variable_mutex.unlock();
	return val;
}

scene* game::get_scene(std::string scene_name)
{
	string_utils string_utils;
	scene_name = string_utils.replace_all(scene_name, variable_value_header, "", false);
	scene_name = string_utils.replace_all(scene_name, var_val_space, " ", false);
	if (scene_name == "" || scene_name == "NULL")
		return nullptr;
	scene* found_scene = dynamic_cast<scene*>(get(this, "scenes/" + scene_name));
	if (found_scene == nullptr)
		found_scene = load_scene_from_file(scene_name);
	return found_scene;
}

entity* game::get_perspective_entity()
{
	if (perspective_entity == "")
		return nullptr;
	if (perspective_entity_ptr == nullptr)
	{
		perspective_entity_ptr = dynamic_cast<entity*>(get(this,"entities/" + perspective_entity)); //dynamic_cast<entity*>(find_first_child(const_cast<game*>(this), "entity")->find_first_child(const_cast<game*>(this), perspective_entity, true, false));
		if (perspective_entity_ptr == nullptr)
		{
			get_engine()->println("ENCOUNTERED FATAL ERROR: PERSPECTIVE ENTITY IS NOT AN ENTITY.");
		}
	}
	return perspective_entity_ptr;
}

entity* game::load_entity_from_file(const std::string& entity_name, const std::string& source, bool dummy)
{
	return load_entity_from_file(entity_name, game_engine->get_scenario_directory(get_name()) + "entities/" + entity_name + ".entity", source);
}

entity* game::load_entity_from_file(const std::string& entity_name, const std::string& file, const std::string& source)
{
	const std::string& filename = file;
	//std::ifstream peek;
	file_reader peek;
	peek.open(filename);
	if (peek.is_open())
	{
		std::string first_line;
		peek.getline(first_line);
		peek.close();

		entity* loaded_entity;
		loaded_entity = new entity();
		loaded_entity->set_name(entity_name);
		loaded_entity->set_turn_number(get_current_turn());
		bool did_get_data = loaded_entity->read(get_engine(), get_name(), filename, this);
		if (get(this, "entities")->find_first_child(this, entity_name, true, false) != nullptr)
		{
			loaded_entity->destroy();
			//get_engine()->println("Error: Already loaded/created entity '", entity_name, "' from file.");
			return nullptr;
		}
		get(this, "entities")->add_child(loaded_entity);
		loaded_entity->call_function(this, "initialize");
		if (did_get_data)
			return loaded_entity;
		else
		{
			loaded_entity->destroy();
			return nullptr;
		}
	}
	else
	{
		get_engine()->println("Error: Could not read ", filename, "(",source,")");
		return nullptr;
	}
}

scene* game::load_scene_from_file(const std::string& scene_name)
{
	/*
	std::string filename = game_engine->get_scenario_directory(get_name()) + "scenes/" + scene_name + ".scene";
	std::ifstream peek;
	peek.open(filename);
	if (peek.is_open())
	{
		std::string first_line;
		std::getline(peek, first_line);
		peek.close();
		scene* loaded_scene = new scene();
		if (get(this, "scenes")->find_first_child(this, scene_name) != nullptr)
		{
			get_engine()->println("Error: already loaded ", scene_name, " from file.");
			loaded_scene->destroy();
			return nullptr;
		}
		loaded_scene->set_name(scene_name);
		bool did_get_data = loaded_scene->read(get_engine(), get_name(), filename, this);
		get(this, "scenes")->add_child(loaded_scene);
		loaded_scene->call_function(this, "initialize");
		if (did_get_data)
			return loaded_scene;
		else
		{
			loaded_scene->destroy();
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
	*/
	std::string filename = game_engine->get_scenario_directory(get_name()) + "scenes/" + scene_name + ".scene";
	return load_scene_from_file(scene_name, filename);
}

scene* game::load_scene_from_file(const std::string& scene_name, const std::string& file)
{
	std::string filename = file;
	//std::ifstream peek;
	file_reader peek;
	peek.open(filename);
	if (peek.is_open())
	{
		std::string first_line;
		peek.getline(first_line);
		peek.close();
		scene* loaded_scene = new scene();
		if (get(this, "scenes")->find_first_child(this, scene_name) != nullptr)
		{
			get_engine()->println("Error: already loaded ", scene_name, " from file.");
			loaded_scene->destroy();
			return nullptr;
		}
		loaded_scene->set_name(scene_name);
		bool did_get_data = loaded_scene->read(get_engine(), get_name(), filename, this);
		get(this, "scenes")->add_child(loaded_scene);
		loaded_scene->call_function(this, "initialize");
		if (did_get_data)
		{
			return loaded_scene;
		}
		else
		{
			loaded_scene->destroy();
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

bool game::resolve_input(game* game_instance, entity* user, const std::string& input_, std::string& return_val)
{
	if (user == get_perspective_entity())
	{
		string_utils string_utils;
		std::string input = input_;

		/*
		std::string input = string_utils.replace_all(input_, ", ", "", false);
		input = string_utils.replace_all(input, ".", "",false);
		string_utils.make_lowercase(input);
		input = string_utils.replace_all(input, "i'd like you to", "",false);
		input = string_utils.replace_all(input, "i'd love you to", "", false);
		input = string_utils.replace_all(input, "i'd like for you to", "", false);
		input = string_utils.replace_all(input, "i'd love for you to", "", false);
		input = string_utils.replace_all(input, "if you will", "", false);
		input = string_utils.replace_all(input, "can you", "", false);
		string_utils.strip(input);
		*/
		std::vector<std::string> dummy;
		if (
			string_utils.matches_command("quit", input) || 
			string_utils.matches_command("exit", input) ||
			string_utils.matches_command("end", input) ||
			string_utils.matches_command("quit game", input) ||
			string_utils.matches_command("exit game", input) ||
			string_utils.matches_command("stop game", input) ||
			string_utils.matches_command("end game", input) ||
			string_utils.matches_command("close game", input) ||
			string_utils.matches_command("end scenario", input) ||
			string_utils.matches_command("quit scenario", input) ||
			string_utils.matches_command("exit scenario", input) ||
			string_utils.matches_command("stop scenario", input) ||
			string_utils.matches_command("close scenario", input) ||
			string_utils.matches_command("end program", input) ||
			string_utils.matches_command("quit program", input) ||
			string_utils.matches_command("stop program", input) ||
			string_utils.matches_command("exit program", input) ||
			string_utils.matches_command("close program", input) ||

			string_utils.matches_command("computer quit", input) ||
			string_utils.matches_command("computer exit", input) ||
			string_utils.matches_command("computer end", input) ||
			string_utils.matches_command("computer quit game", input) ||
			string_utils.matches_command("computer exit game", input) ||
			string_utils.matches_command("computer stop game", input) ||
			string_utils.matches_command("computer end game", input) ||
			string_utils.matches_command("computer close game", input) ||
			string_utils.matches_command("computer end scenario", input) ||
			string_utils.matches_command("computer quit scenario", input) ||
			string_utils.matches_command("computer exit scenario", input) ||
			string_utils.matches_command("computer stop scenario", input) ||
			string_utils.matches_command("computer close scenario", input) ||
			string_utils.matches_command("computer end program", input) ||
			string_utils.matches_command("computer quit program", input) ||
			string_utils.matches_command("computer stop program", input) ||
			string_utils.matches_command("computer exit program", input) ||
			string_utils.matches_command("computer close program", input) ||

			string_utils.matches_command("quit my game", input) ||
			string_utils.matches_command("exit my game", input) ||
			string_utils.matches_command("stop my game", input) ||
			string_utils.matches_command("end my game", input) ||
			string_utils.matches_command("close my game", input) ||
			string_utils.matches_command("end my scenario", input) ||
			string_utils.matches_command("quit my scenario", input) ||
			string_utils.matches_command("exit my scenario", input) ||
			string_utils.matches_command("stop my scenario", input) ||
			string_utils.matches_command("close my scenario", input) ||
			string_utils.matches_command("end my program", input) ||
			string_utils.matches_command("quit my program", input) ||
			string_utils.matches_command("stop my program", input) ||
			string_utils.matches_command("exit my program", input) ||
			string_utils.matches_command("close my program", input) ||

			string_utils.matches_command("computer quit my game", input) ||
			string_utils.matches_command("computer exit my game", input) ||
			string_utils.matches_command("computer stop my game", input) ||
			string_utils.matches_command("computer end my game", input) ||
			string_utils.matches_command("computer close my game", input) ||
			string_utils.matches_command("computer end my scenario", input) ||
			string_utils.matches_command("computer quit my scenario", input) ||
			string_utils.matches_command("computer exit my scenario", input) ||
			string_utils.matches_command("computer stop my scenario", input) ||
			string_utils.matches_command("computer close my scenario", input) ||
			string_utils.matches_command("computer end my program", input) ||
			string_utils.matches_command("computer quit my program", input) ||
			string_utils.matches_command("computer stop my program", input) ||
			string_utils.matches_command("computer exit my program", input) ||
			string_utils.matches_command("computer close my program", input)
			)
		{
			return_val = "DONE";
			game_going = false;
			get_engine()->clear_screen(false);
			return true;
		}
		else if (
			save_any_time && 
			(string_utils.matches_command("save", input) ||
			string_utils.matches_command("save game", input) ||
			string_utils.matches_command("save scenario", input) ||
			string_utils.matches_command("save program", input)
			))
		{
			save_game_to_file();
			describe_scene(get_perspective_entity()->get_scene());
			return_val = "DONE";
			return true;
		}
		else if (
			input == "describe"
			|| input == "help"
			|| input == "options"
			|| string_utils.matches_command("describe the surroundings", input)
			|| string_utils.matches_command("describe my surroundings", input)
			|| string_utils.matches_command("describe everything", input)
			|| string_utils.matches_command("describe it all", input)
			|| string_utils.matches_command("describe surroundings", input)
			|| string_utils.matches_command("describe area", input)
			|| string_utils.matches_command("describe the area", input)
			|| string_utils.matches_command("describe my area", input)
			|| string_utils.matches_command("describe my surrounding area", input)
			|| string_utils.matches_command("describe the surrounding area", input)

			|| input == "review"
			|| string_utils.matches_command("review the surroundings", input)
			|| string_utils.matches_command("review my surroundings", input)
			|| string_utils.matches_command("review surroundings", input)
			|| string_utils.matches_command("review area", input)
			|| string_utils.matches_command("review the area", input)
			|| string_utils.matches_command("review my area", input)
			|| string_utils.matches_command("review my surrounding area", input)
			|| string_utils.matches_command("review the surrounding area", input)

			|| input == "display"
			|| string_utils.matches_command("display the surroundings", input)
			|| string_utils.matches_command("display my surroundings", input)
			|| string_utils.matches_command("display surroundings", input)
			|| string_utils.matches_command("display area", input)
			|| string_utils.matches_command("display the area", input)
			|| string_utils.matches_command("display my area", input)
			|| string_utils.matches_command("display my surrounding area", input)
			|| string_utils.matches_command("display the surrounding area", input)

			|| input == "look"
			|| string_utils.matches_command("look around", input)
			|| string_utils.matches_command("look all around", input)
			|| string_utils.matches_command("look at the surroundings", input)
			|| string_utils.matches_command("look at my surroundings", input)
			|| string_utils.matches_command("look at surroundings", input)
			|| string_utils.matches_command("look at area", input)
			|| string_utils.matches_command("look at the area", input)
			|| string_utils.matches_command("look at my area", input)
			|| string_utils.matches_command("look at my surrounding area", input)
			|| string_utils.matches_command("look at the surrounding area", input)

			|| string_utils.matches_command("look around the surroundings", input)
			|| string_utils.matches_command("look around my surroundings", input)
			|| string_utils.matches_command("look around surroundings", input)
			|| string_utils.matches_command("look around area", input)
			|| string_utils.matches_command("look around the area", input)
			|| string_utils.matches_command("look around my area", input)
			|| string_utils.matches_command("look around my surrounding area", input)
			|| string_utils.matches_command("look around the surrounding area", input)
			|| string_utils.matches_command("look around surrounding area", input)

			|| string_utils.matches_command("look all around the surroundings", input)
			|| string_utils.matches_command("look all around my surroundings", input)
			|| string_utils.matches_command("look all around surroundings", input)
			|| string_utils.matches_command("look all around area", input)
			|| string_utils.matches_command("look all around the area", input)
			|| string_utils.matches_command("look all around my area", input)
			|| string_utils.matches_command("look all around my surrounding area", input)
			|| string_utils.matches_command("look all around the surrounding area", input)
			|| string_utils.matches_command("look all around surrounding area", input)

			|| string_utils.matches_command("look around at all the surroundings", input)
			|| string_utils.matches_command("look around at all my surroundings", input)
			|| string_utils.matches_command("look around at all surroundings", input)
			|| string_utils.matches_command("look around at all area", input)
			|| string_utils.matches_command("look around at all the area", input)
			|| string_utils.matches_command("look around at all my area", input)
			|| string_utils.matches_command("look around at all my surrounding area", input)
			|| string_utils.matches_command("look around at all the surrounding area", input)
			|| string_utils.matches_command("look around at all surrounding area", input)
			)
		{
			scene* cur_scene = get_perspective_entity()->get_scene();
			describe_scene(cur_scene);
			return_val = "DONE";
			return true;
		}
		else if (
			string_utils.matches_command("clear", input) ||
			string_utils.matches_command("cls", input) ||
			string_utils.matches_command("clr", input) ||
			string_utils.matches_command("clear screen", input) ||
			string_utils.matches_command("clr screen", input) ||
			string_utils.matches_command("clear display", input) ||
			string_utils.matches_command("clr screen", input) ||

			string_utils.matches_command("clear the screen", input) ||
			string_utils.matches_command("clr the screen", input) ||
			string_utils.matches_command("clear the display", input) ||
			string_utils.matches_command("clr the screen", input) ||

			string_utils.matches_command("clear output", input) ||
			string_utils.matches_command("clr output", input) ||
			string_utils.matches_command("clear output", input) ||
			string_utils.matches_command("clr output", input) ||

			string_utils.matches_command("clear the output", input) ||
			string_utils.matches_command("clr the output", input) ||
			string_utils.matches_command("clear the output", input) ||
			string_utils.matches_command("clr the outpu", input) ||

			string_utils.matches_command("clear the console", input) ||
			string_utils.matches_command("clr the console", input) ||
			string_utils.matches_command("clear the console", input) ||
			string_utils.matches_command("clr the console", input) ||
			string_utils.matches_command("clear console", input) ||
			string_utils.matches_command("clr console", input) ||
			string_utils.matches_command("clear console", input) ||
			string_utils.matches_command("clr console", input)
			)
		{
			get_engine()->clear_screen();

			scene* cur_scene = get_perspective_entity()->get_scene();

			describe_scene(cur_scene);

			return_val = "DONE";
			return true;
		}
		else
		{
			std::vector<std::string> wildcards;
			bool matches_desc = false;

			std::vector<std::string> options = {"describe $ent", "examine $ent", "look at $ent", "I look at $ent", "I look at the $ent", "I look more closely at $ent"};

			for (int i = 0; i < options.size(); ++i)
			{
				options[i] = get_engine()->extra_text_processing(options[i], this);
				if (string_utils.matches_command(options[i], input, wildcards))
				{
					matches_desc = true;
					break;
				}
				else
				{
					continue;
				}
			}

			if (matches_desc)
			{
				std::string ent_alias = wildcards[0];
				entity* found = get_entity_in_scene(ent_alias, get_perspective_entity()->get_scene_name(), get_perspective_entity(), "game.cpp");
				if (found != nullptr)
				{
					if (found->has_function_named("command:describe"))
					{
						found->call_function(this, "command:describe");
					}
					else
					{
						found->call_function(this, "describe");
					}
					return true;
				}
			}
		}
	}
	return false;
}

void game::save_game_to_file()
{
	std::string scenario_directory = game_engine->get_scenario_directory(get_name());
	std::string saves_directory = game_engine->get_saves_directory(get_name());

	std::string save_file_name = "save.dat";
	std::string save_file_list_name = "files.dat";

	res_file list_of_current_saves;

	std::ofstream saves_list_file;
	std::string full_saves_list_directory = saves_directory + save_file_list_name;

	bool successfully_read_save_files = list_of_current_saves.read_raw(full_saves_list_directory);
	
	auto ent_list = get_entities();
	for (auto i = ent_list.begin(); i != ent_list.end(); ++i)
	{
		entity* c = *i;
		if (c == get_perspective_entity())
			continue;
		while ((!c->idle()) && (c->get_scene()!=get_perspective_entity()->get_scene()))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	if (successfully_read_save_files)
	{
		game_engine->print_lines("Choose save file:", "1. New File");
		for (res_file::line_num i = 1; i < list_of_current_saves.lines() - 1; ++i)
		{
			game_engine->println(i + 1, ". ", list_of_current_saves.get_line(i));
		}

		int save_file_choice = game_engine->get_integer_input();

		while (save_file_choice < 1 && save_file_choice > list_of_current_saves.lines() - 1)
		{
			game_engine->println("You must enter a number from 1 to ", list_of_current_saves.lines() + 1);
			save_file_choice = game_engine->get_integer_input();
		}

		int chosen_index = save_file_choice - 1;
		if (chosen_index == 0)
		{
			game_engine->println("Enter the name of the new save file:");
			save_file_name = game_engine->get_input();
		}
		else
		{
			save_file_name = list_of_current_saves.get_line(chosen_index);
		}

		save_file_name[0] = std::toupper(save_file_name[0]);

		if (list_of_current_saves.find_match(0, save_file_name) == RES_FILE_NO_MATCH)
		{
			saves_list_file.open(full_saves_list_directory, std::ios::app);
			if (saves_list_file.is_open())
			{
				saves_list_file << save_file_name << std::endl;
				saves_list_file.close();
			}
			else
			{
				game_engine->println("Error: Could not open file ", full_saves_list_directory);
				return;
			}

		}

		std::string full_save_file_directory = saves_directory + save_file_name;

		const auto& ents = get_entities();
		const auto& scenes = get_scenes();

		for (size_t i = 0; i < ents.size(); ++i)
		{
			ents[i]->call_function(this, "before_save");
		}

		for (size_t i = 0; i < scenes.size(); ++i)
		{
			scenes[i]->call_function(this, "before_save");
		}

		std::ofstream main_save_file;
		main_save_file.open(full_save_file_directory, std::ios::binary);

		try
		{
			if (main_save_file.is_open())
				save_to_file(main_save_file, get_name(), get_engine());
			else
				game_engine->println("Error: Could not open ", full_save_file_directory);
		}
		catch (const std::exception& E)
		{
			game_engine->println("Error: encountered problem while writing to file.");
		}
	}
	else
	{
		game_engine->println("Error: Could not open ", full_saves_list_directory);
	}

}

void game::set_meta_value(const std::string& var_name, const std::string& var_value)
{
	meta_variable_mutex.lock();
	std::ofstream file;
	file.open(var_name);
	if (file.is_open())
	{
		file << var_value;
		file.close();
	}
	meta_variable_mutex.unlock();
}

void game::set_perspective_entity(entity* pc)
{
	if (pc != nullptr)
	{
		perspective_entity = pc->get_name();
		perspective_entity_ptr = pc;
	}
}