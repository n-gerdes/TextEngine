#include "engine/headers/instance/scene.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/instance/dummy.h"
#include <thread>
#include <mutex>

scene::scene()
{
	add_child(new dummy("entities")); //These will store dummies containing the names of entities & items.

	register_innate_function("transfer");
	register_innate_function("say");
	register_innate_function("set_global_value");
	register_innate_function("set_meta_value");
	register_innate_function("set_value");
}

void transfer_func(game* game_instance, scene* s, std::vector<std::string>& args, std::string& err)
{
	scene* new_scene = s;
	entity* c = game_instance->get_entity_by_name(args[0]);
	if (c)
	{
		const auto& followers = c->get_attached_entity_names();
		for (auto i = followers.begin(); i != followers.end(); ++i)
		{
			entity* follower = game_instance->get_entity_by_name(*i);
			if (follower)
			{
				follower->remove_from_scene();
				new_scene->queue_transfer(follower);
			}
		}
		c->remove_from_scene();
		new_scene->queue_transfer(c);
	}
	else
	{
		err = "Error: No entity found by the name of '" + args[0] + "'";
	}
}

void say_func(game* game_instance, scene* s, std::vector<std::string>& args, std::string& err)
{
	if (game_instance->get_perspective_entity() == nullptr)
		return;
	if (game_instance->get_perspective_entity()->get_scene() == s)
	{
		game_instance->get_perspective_entity()->println(game_instance, game_instance->get_engine()->correct_tokenizer_bug(args[0]));
	}
}

void set_global_value_func(game* game_instance, scene* s, std::vector<std::string>& args, std::string& err)
{
	game_instance->set_value(args[0], args[1]);
}


void set_meta_value_func(game* game_instance, scene* s, std::vector<std::string>& args, std::string& err)
{
	game_instance->set_meta_value(args[0], args[1]);
}

void set_value_func(game* game_instance, scene* s, std::vector<std::string>& args, std::string& err)
{
	const std::string& var_name = args[0];
	const std::string& var_val = args[1];
	s->set_value(var_name, var_val);
}

std::string scene::call_innate_function(game* game_instance, const std::string& function_name, std::vector<std::string>& args)
{
	std::string err;

	pair_innate_function(&transfer_func, function_name, "transfer", args, game_instance, err, 1);
	pair_innate_function(&set_global_value_func, function_name, "set_global_value", args, game_instance, err, 2);
	pair_innate_function(&say_func, function_name, "say", args, game_instance, err, 1);
	pair_innate_function(&set_meta_value_func, function_name, "set_meta_value", args, game_instance, err, 2);
	pair_innate_function(&set_value_func, function_name, "set_value", args, game_instance, err, 2);

	return err;
}


void scene::display_text()
{
	
}

entity* scene::get_entity(game* game_instance, const std::string& name, bool allow_alias) const
{
	return game_instance->get_entity_in_scene(name, get_name(), allow_alias);
}

entity* scene::get_first_entity(game* game_instance, const std::string& name) const
{
	return game_instance->get_first_entity_in_scene(name, get_name());
}

const std::vector<entity*> scene::get_entities_in_scene() const
{
	std::vector<entity*> list;
	game* game_instance = dynamic_cast<game*>(get_parent()->get_parent());
	entities_in_scene = 0;
	if (game_instance != nullptr)
	{
		auto& children = (*(get_children().begin()))->get_children();
		list.reserve(children.size());
		if (children.size() == 0)
		{
			return list;
		}
		else if (children.size() == 1)
		{
			list.push_back(  game_instance->get_entity((*children.begin())->get_name(), false)  );
			entities_in_scene = 1;
			return list;
		}
		else
		{
			for (auto i = children.begin(); i != children.end(); ++i) //crashing error caused here. 'cannot increment value-initialized list iterator'. Only child was the mercant.
			{
				std::string child_name = (*i)->get_name();
				entity* found = game_instance->get_entity(child_name, false);
				list.push_back(found);
				++entities_in_scene;
			}
		}
	}
	return list;
}

void scene::finalize_entity_removal()
{
	--entities_in_scene;
}

game* scene::get_game_instance()
{
	return static_cast<game*>(get_parent()->get_parent());
}

int scene::count_entities_in_scene() const
{
	return entities_in_scene;
}

static std::mutex threads_launched_mutex;

void scene_friend_funcs::game_loop(game* game_instance, scene* this_scene, int* threads_launched)
{
	srand(time(NULL));
	while (game_instance->game_is_active())
	{
		this_scene->transferred_perspective_character = false;
		this_scene->load_transfer_entities(game_instance);
		if (this_scene->loaded_from_file && !this_scene->has_read_description_after_loading_from_file)
		{
			this_scene->has_read_description_after_loading_from_file = true;
			this_scene->transferred_perspective_character = true;
		}
		const std::vector<entity*> children = this_scene->get_entities_in_scene(); //Crashing error comes from here

		if (this_scene->transferred_perspective_character)
		{
			this_scene->call_function(game_instance, "describe");
			for (auto entity_iterator = children.begin(); entity_iterator != children.end(); ++entity_iterator)
			{
				entity* current_entity = dynamic_cast<entity*>(*entity_iterator);
				if (current_entity)
				{
					if (current_entity->get_turn_number() >= game_instance->get_current_turn() - 1 && this_scene != game_instance->get_perspective_entity()->get_scene())
					{
						continue;
					}
					else
					{
						current_entity->call_function(game_instance, "describe");
					}
				}
			}
		}

		for (auto entity_iterator = children.begin(); entity_iterator != children.end(); ++entity_iterator)
		{
			entity* current_entity = dynamic_cast<entity*>(*entity_iterator);
			if (current_entity)
			{
				if (current_entity->get_turn_number() >= game_instance->get_current_turn() - 1 && this_scene != game_instance->get_perspective_entity()->get_scene())
				{
					continue;
				}
				else
				{
					std::string dummy_return_val;
					this_scene->call_function(game_instance, "before_turn", { current_entity->get_name() }, dummy_return_val);

					current_entity->call_function(game_instance, "before_turn");

					std::string reason_for_failure;
					bool success = current_entity->take_turn(game_instance, reason_for_failure);
					while (!success && current_entity == game_instance->get_perspective_entity())
					{
						if (reason_for_failure == "")
						{
							//current_entity->println(game_instance, current_entity->get_last_command());
							current_entity->println(game_instance, "That doesn't seem to be an option here.");
						}
						else
						{
							current_entity->println(game_instance, reason_for_failure);
						}
						current_entity->interrupt();
						success = current_entity->take_turn(game_instance, reason_for_failure);
					}
					current_entity->advance_turn_count();
				}
			}
		}
	}
	
	threads_launched_mutex.lock();
	--(*threads_launched);
	threads_launched_mutex.unlock();
}

void scene::launch(game* game_instance, int* threads_launched_ptr)
{
	if (!thread_active)
	{
		threads_launched_mutex.lock();
		++(*threads_launched_ptr);
		threads_launched_mutex.unlock();
		thread_active = true;
		std::thread my_thread(scene_friend_funcs::game_loop, game_instance, this, threads_launched_ptr);
		my_thread.detach();
	}
}

void scene::load_transfer_entities(game* game_instance)
{
	//std::cout << "Checking for transfer entities\n";
	queuetex.lock();
	const auto& entities = game_instance->get_entities();
	for (size_t i = 0; i < entities.size(); ++i)
	{
		entity* ent = entities[i];
		//std::cout << "Loading transfer entity '" << ent->get_name() << "'\n";
		if (std::find(transfer_queue.begin(), transfer_queue.end(), ent) != transfer_queue.end()) //If this entity was waiting in the transfer queue
		{
			if (ent == game_instance->get_perspective_entity())
			{
				transferred_perspective_character = true; //This is so when transferring the PC to a scene, it knows to read off a description to them.
				if (!has_read_description_after_loading_from_file)
					has_read_description_after_loading_from_file = true;
			}
			ent->set_to_scene(get_name());
			++entities_in_scene;
		}
	}
	transfer_queue.clear();
	queuetex.unlock();
}

void scene::on_destroyed()
{

}

template <typename T>
void scene::pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int number_of_args)
{
	if (checked_func_name == innate_function_name)
	{
		if (args.size() == number_of_args)
		{
			err = "";
			internal_func(game_instance, this, args, err);
		}
		else if (args.size() < number_of_args)
		{
			std::string args_string;
			for (int i = 0; i < args.size(); ++i)
				args_string += "'" + args[i] + "', ";

			if (args_string.size() > 1)
				args_string.resize(args_string.size() - 2);

			err = "Too few arguments supplied for function '" +
				innate_function_name + "'; expected " +
				std::to_string(number_of_args) +
				", got " +
				std::to_string(args.size()) +
				" (" +
				args_string +
				")";
		}
		else
		{
			std::string args_string;
			for (int i = 0; i < args.size(); ++i)
				args_string += "'" + args[i] + "', ";
			args_string.resize(args_string.size() - 2);

			err = "Too many arguments supplied for function '" +
				innate_function_name +
				"'; expected " +
				std::to_string(number_of_args) +
				", got " +
				std::to_string(args.size()) +
				" (" + args_string + ")";
		}
	}
}

template <typename T>
void scene::pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int min_args, int max_args)
{
	if (checked_func_name == innate_function_name)
	{
		if (args.size() >= min_args && args.size() <= max_args)
		{
			err = "";
			internal_func(game_instance, this, args, err);
		}
		else if (args.size() < min_args)
		{
			std::string args_string;
			for (int i = 0; i < args.size(); ++i)
				args_string += "'" + args[i] + "', ";

			if (args_string.size() > 1)
				args_string.resize(args_string.size() - 2);

			err = "Too few arguments supplied for function " +
				innate_function_name + "; expected " +
				std::to_string(min_args) + " to " + std::to_string(max_args) +
				", got " +
				std::to_string(args.size()) +
				" (" +
				args_string +
				")";
		}
		else
		{
			std::string args_string;
			for (int i = 0; i < args.size(); ++i)
				args_string += "'" + args[i] + "', ";
			args_string.resize(args_string.size() - 2);

			err = "Too many arguments supplied for function " +
				innate_function_name +
				"; expected " +
				std::to_string(min_args) + " to " + std::to_string(max_args) +
				", got " +
				std::to_string(args.size()) +
				" (" + args_string + ")";
		}
	}
}


void scene::process_line_from_file(const std::string& line)
{
	string_utils string_utils;
	/*
	std::vector<std::string> wildcards;
	if (string_utils.matches_command("register_npc_option( $ai , $opt , $weight )", line, wildcards))
	{
		try {
			int weight = std::stoi(wildcards[2]);
			if (weight < 1)
				throw;
			register_npc_option(wildcards[0], wildcards[1], weight);
		}
		catch (const std::exception& E)
		{
			std::cout << "Error loading scene: \"" << wildcards[2] << "\" is not a valid npc option weight (it must be a positive integer)" << std::endl;
		}
	}
	*/
}

bool scene::resolve_input(game* game_instance, entity* user, const std::string& input, std::string& return_val)
{
	//std::cout << "SCENE PROCESSING " << input << std::endl;
	line_num custom_func_line = 0;
	string_utils string_utils;
	std::vector<std::string> dummy_var_names;
	std::vector<std::string> dummy_var_values;
	if (get_command_func_lines().size() != 0)
	{
		for (size_t custom_func_line_index = 0; custom_func_line_index < get_command_func_lines().size(); ++custom_func_line_index)
		{
			custom_func_line = get_command_func_lines()[custom_func_line_index];
			std::string line = get_line(custom_func_line);
			std::vector<std::string> wildcards;
			if (string_utils.matches_command("function command: $func ( $args )", line, wildcards, " ():"))
			{
				//std::cout << "MATCHED " << line << " AGAINST " << "function command: $func ( $args )" << std::endl;
				std::string actual_func_name = wildcards[0];
				std::string expected_args = wildcards[1];

				std::string check = "function command:" + input + "( " + expected_args + " )";

				if (string_utils.matches_command(line, check, wildcards, " ():"))
				{
					std::vector<std::string> args;
					args.push_back(user->get_name());
					for (size_t i = 0; i < wildcards.size(); ++i)
						args.push_back(wildcards[i]);

					std::string err = call_function(game_instance, "command:" + actual_func_name, args, return_val);
					if (err == "")
						return true;
				}
				else
				{
					//std::cout << "HERE (  " << input << " / " << line << "  /  " << check << "  )" << std::endl;
				}
			}
		}
	}
	return false;
}

void scene::queue_transfer(entity* ent)
{
	queuetex.lock();
	if ((ent->get_scene_name() == "" || ent->get_scene_name() == "NULL") && !ent->in_transfer_queue()) //It can only transfer entities that aren't currently in a scene
	{
		transfer_queue.push_back(ent);
		ent->set_in_transfer_queue(true);
	}
	queuetex.unlock();
}

void scene::load_variables(std::ifstream& file, const std::string& scenario_name, engine* engine) {
	adjudicator::load_variables(file, scenario_name, engine);
	std::string loaded_file;
	load_string(file, loaded_file);
	load_uint32_t(file, entities_in_scene);
	read(engine, scenario_name, loaded_file, get_game_instance());
	loaded_from_file = true;
}

void scene::save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const {
	adjudicator::save_variables(file, scenario_name, engine);
	save_string(file, get_filename());
	save_uint32_t(file, entities_in_scene);
}
