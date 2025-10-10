#include "engine/headers/instance/entity.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/dummy.h"
#include "engine/headers/instance/scene.h"
#include "engine/headers/util/string_utils.h"
#include <thread>
#include <mutex>

entity::entity()
{
	register_innate_function("tell");
	register_innate_function("set_value");
	register_innate_function("add_alias");
	register_innate_function("add_title");
	register_innate_function("learn_alias");
	register_innate_function("learn_title");
	register_innate_function("transfer");
	register_innate_function("say");
	register_innate_function("damage");
	register_innate_function("attach");
	register_innate_function("attach_to");
	register_innate_function("unattach");
	register_innate_function("clone");
	register_innate_function("set_global_value");
	register_innate_function("wipe_aliases");
	register_innate_function("set_meta_value");
	register_innate_function("recover");
	register_innate_function("set_max_hp");
	register_innate_function("set_hp");

	register_innate_function("set_clear_on_scene_change");
	register_innate_function("set_save_any_time");

	register_innate_function("describe_scene");
	register_innate_function("clear");

	register_innate_function("pause");
}

void entity::save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const
{
	this;
	while (
		(!idle()) &&
		(!(get_game_instance()->get_perspective_entity()->get_scene() == get_scene()))
		)
	{
		//std::cout << get_name() << ": " << idle() << " / " << ((get_game_instance()->get_perspective_entity()->get_scene() == get_scene())) << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	adjudicator::save_variables(file, scenario_name, engine);
	save_uint64_t(file, current_turn);
	save_bool(file, interrupted);
	save_string(file, name_of_interrupter);
	save_uint64_t(file, command_repeats);
	save_string(file, current_scene);
	save_int32_t(file, max_hp);
	save_int32_t(file, hp);
	save_string(file, narrative_queue);
	save_string(file, current_command);
	save_string(file, last_command_return_value);
	save_string(file, get_filename());

	save_uint64_t(file, static_cast<uint64_t>(attached_entity_names.size()));
	for (auto attached_entities_iterator = attached_entity_names.begin();
		attached_entities_iterator != attached_entity_names.end();
		++attached_entities_iterator)
	{
		const std::string& attached_entity_name = *attached_entities_iterator;
		save_string(file, attached_entity_name);
	}

	save_uint64_t(file, static_cast<uint64_t>(known_aliases.size()));
	auto i = known_aliases.begin();
	//string_utils string_utils;
	while (i != known_aliases.end())
	{
		const std::string& true_name = i->first;
		const std::map<std::string, std::vector<std::string>>& categories_to_aliases_map = i->second;
		save_string(file, true_name);
		uint64_t number_of_categories = categories_to_aliases_map.size();
		save_uint64_t(file, number_of_categories);
		for (auto j = categories_to_aliases_map.begin(); j != categories_to_aliases_map.end(); ++j)
		{
			const std::string& category_name = j->first;
			const std::vector<std::string>& aliases = j->second;
			save_string(file, category_name);
			save_string_vector(file, aliases);
		}
		++i;
	}
}

void entity::load_variables(std::ifstream& file, const std::string& scenario_name, engine* engine)
{
	adjudicator::load_variables(file, scenario_name, engine);
	load_uint64_t(file, current_turn);
	load_bool(file, interrupted);
	load_string(file, name_of_interrupter);
	load_uint64_t(file, command_repeats);
	//load_string(file, ai_controller);
	load_string(file, current_scene);
	load_int32_t(file, max_hp);
	load_int32_t(file, hp);
	load_string(file, narrative_queue);
	load_string(file, current_command);
	load_string(file, last_command_return_value);
	std::string loaded_file_name;
	load_string(file, loaded_file_name);
	read(engine, scenario_name, loaded_file_name, get_game_instance());
	attached_entity_names.clear();
	uint64_t number_of_attached_entities = 0;
	load_uint64_t(file, number_of_attached_entities);
	std::string attached_entity_name;
	uint64_t i;
	for (i = 0; i < number_of_attached_entities; ++i)
	{
		load_string(file, attached_entity_name);
		attached_entity_names.push_back(attached_entity_name);
	}

	known_aliases.clear();
	uint64_t& number_of_known_aliases = number_of_attached_entities; //Reusing the same memory for efficiency
	load_uint64_t(file, number_of_known_aliases);
	for (i = 0; i < number_of_known_aliases; ++i)
	{
		std::string& true_name = attached_entity_name; //Reusing memory again
		load_string(file, true_name);
		uint64_t number_of_categories;
		load_uint64_t(file, number_of_categories);
		known_aliases.insert_or_assign(true_name, std::map<std::string, std::vector<std::string>>());
		for (size_t i = 0; i < number_of_categories; ++i)
		{
			std::string category;
			std::vector<std::string> aliases;
			load_string(file, category);
			load_string_vector(file, aliases);
			known_aliases.at(true_name).insert_or_assign(category, aliases);
		}
	}
}

void tell_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	if (args.size() == 0)
	{
		c->println(game_instance, "");
		return;
	}
	std::string disp;
	for (int i = 0; i < args.size(); ++i)
	{
		disp += args[i];
	}
	std::string text = game_instance->get_engine()->correct_tokenizer_bug(disp);

	c->println(game_instance, text); //Should I correct_tokenizer_bug on the whole thing or each individual one?
}

void set_value_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string var_name = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	std::string var_val = string_utils.replace_all(args[1], { variable_value_header_char }, "", false);
	var_val = string_utils.replace_all(var_val, var_val_space, " ", false);
	var_name = string_utils.replace_all(var_name, var_val_space, " ", false);
	c->set_value(var_name, var_val);
}

void set_scene_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string scene_name = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	scene_name = string_utils.replace_all(scene_name, var_val_space, " ", false);

	scene* found_scene = c->set_to_scene(scene_name);

	if (!found_scene)
	{
		err = "Invalid scene";
	}
}

void add_alias_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	std::string alias = args[0];
	string_utils string_utils;
	alias = string_utils.replace_all(alias, variable_value_header, "", false);
	std::string& vsub = alias;
	vsub = string_utils.replace_all(vsub, var_val_space, " ", false);
	c->add_alias(alias);
}

void add_title_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	std::string title = args[0];
	string_utils string_utils;
	title = string_utils.replace_all(title, variable_value_header, "", false);
	std::string& vsub = title;
	vsub = string_utils.replace_all(vsub, " ", var_val_space, false);
	c->add_title(title);
}

void say_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	if (c->get_scene_name()!="NULL" && (c->get_scene() == game_instance->get_perspective_entity()->get_scene()))
	{
		for (int i = 0; i < args.size(); ++i)
		{
			game_instance->get_perspective_entity()->print(game_instance, game_instance->get_engine()->correct_tokenizer_bug(args[i]));
		}
		game_instance->get_perspective_entity()->print(game_instance, "\n");
		game_instance->get_engine()->set_text_color(color::DEFAULT);
	}
}

void learn_alias_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string ent_name = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	std::string& vsub = ent_name;
	vsub = string_utils.replace_all(vsub, var_val_space, " ", false);
	std::string alias_category = string_utils.get_lowercase(string_utils.replace_all(args[1], { variable_value_header_char }, "", false));
	alias_category = string_utils.replace_all(alias_category, " ", var_val_space, false);
	std::string alias = string_utils.get_lowercase(string_utils.replace_all(args[2], { variable_value_header_char }, "", false));
	alias = string_utils.replace_all(alias, var_val_space, " ", false);

	c->learn_alias_for_entity(ent_name, alias_category, alias);
}

void learn_title_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string ent_name = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	ent_name = string_utils.replace_all(ent_name, var_val_space, " ", false);
	std::string alias_category = string_utils.get_lowercase(string_utils.replace_all(args[1], { variable_value_header_char }, "", false));
	alias_category = string_utils.replace_all(alias_category, var_val_space, " ", false);
	std::string alias = string_utils.get_lowercase(string_utils.replace_all(args[2], { variable_value_header_char }, "", false));
	alias = string_utils.replace_all(alias, var_val_space, " ", false);
	c->learn_alias_for_entity(ent_name, alias_category, alias);
	c->learn_alias_for_entity(ent_name, alias_category, "the "+alias);
}

void transfer_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string new_scene_name = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	new_scene_name = string_utils.replace_all(new_scene_name, var_val_space, " ", false);

	if (new_scene_name == c->get_scene_name())
	{
		return;
	}
	scene* new_scene = game_instance->get_scene(new_scene_name);
	if (new_scene)
	{
		std::string dummy_return_value;
		const auto& followers = c->get_attached_entity_names();
		for (auto i = followers.begin(); i != followers.end(); ++i)
		{
			entity* follower = game_instance->get_entity_by_name(*i, source);
			if (follower)
			{
				follower->call_function(game_instance, "before_leave", { new_scene_name }, dummy_return_value);
				follower->remove_from_scene();
				new_scene->queue_transfer(follower);
			}
		}
		c->call_function(game_instance, "before_leave", {new_scene_name}, dummy_return_value);
		c->remove_from_scene();
		new_scene->queue_transfer(c);
	}
	else
	{
		err = "Found no scene by the name of'" + new_scene_name + "'";
	}
}

void damage_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& code_source)
{
	string_utils string_utils;
	std::string dmg_arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	dmg_arg = string_utils.replace_all(dmg_arg, var_val_space, " ", false);
	if (string_utils.is_integer(dmg_arg))
	{
		entity::hp_t amount = std::stoi(dmg_arg);
		entity* source = nullptr;
		if (args.size() == 1)
		{
			c->damage(game_instance, amount);
		}
		else if (args.size() == 2)
		{
			std::string ent_name = string_utils.replace_all(args[1], { variable_value_header_char }, "", false);
			ent_name = string_utils.replace_all(ent_name, var_val_space, " ", false);

			source = game_instance->get_entity_by_name(ent_name, code_source);
			if (source)
			{
				c->damage(game_instance, source, amount);
			}
			else
			{
				err = "Error: Found no entity by the name of " + ent_name;
			}
		}
		else
		{
			err = "Error: this message should never appear";
		}
	}
	else
	{
		err = "Error: Damage must be an integer";
	}
}

void heal_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& code_source)
{
	string_utils string_utils;
	std::string heal_arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	heal_arg = string_utils.replace_all(heal_arg, var_val_space, " ", false);
	if (string_utils.is_integer(heal_arg))
	{
		entity::hp_t amount = std::stoi(heal_arg);
		entity* source = nullptr;
		if (args.size() == 1)
		{
			c->recover(game_instance, amount);
		}
		else if (args.size() == 2)
		{
			std::string ent_name = string_utils.replace_all(args[1], { variable_value_header_char }, "", false);
			ent_name = string_utils.replace_all(ent_name, var_val_space, " ", false);
			source = game_instance->get_entity_by_name(ent_name, code_source);
			if (source)
			{
				c->recover(source, amount);
			}
			else
			{
				err = "Error: Found no entity by the name of " + ent_name + "(" + code_source + ")";
			}
		}
		else
		{
			err = "Error: this message should never appear";
		}
	}
	else
	{
		err = "Error: Amount of HP recovered must be an integer (" + code_source + ")";
	}
}

void attach_to_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	arg = string_utils.replace_all(arg, var_val_space, " ", false);
	entity* target = game_instance->get_entity_by_name(arg, source);
	if (target)
	{
		c->attach_to(target);
	}
	else
	{
		err = "Could not attach " + c->get_name() + " to entity '" + arg + "' because '" + arg + "' does not exist (" + source + ")";
	}
}

void attach_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	arg = string_utils.replace_all(arg, var_val_space, " ", false);
	entity* target = game_instance->get_entity_by_name(arg, source);
	if (target)
	{
		c->attach(target);
	}
	else
	{
		err = "Could not attach " + arg + " to entity '" + c->get_name() + "' because '" + arg + "' does not exist";
	}
}

void unattach_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	if (args.size() == 0)
	{
		c->unattach();
	}
	else if (args.size() == 1)
	{
		string_utils string_utils;
		std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
		arg = string_utils.replace_all(arg, var_val_space, " ", false);
		c->unattach(arg);
	}
	else
	{
		err = "Error: This message should never appear.";
	}
}

void clone_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string ent_name = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	ent_name = string_utils.replace_all(ent_name, var_val_space, " ", false);
	entity* new_ent = game_instance->load_entity_from_file(ent_name, c->get_filename(), source);
	if (new_ent != nullptr)
	{
		new_ent->copy_data_from(c);
	}
}

void set_global_value_func(game* game_instance, entity* s, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string a = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	a = string_utils.replace_all(a, var_val_space, " ", false);
	std::string b = string_utils.replace_all(args[1], { variable_value_header_char }, "", false);
	b = string_utils.replace_all(b, var_val_space, " ", false);
	game_instance->set_value(a, b);
}

void wipe_aliases_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	c->wipe_aliases(game_instance);
}

void set_meta_value_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string a = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	a = string_utils.replace_all(a, var_val_space, " ", false);
	std::string b = string_utils.replace_all(args[1], { variable_value_header_char }, "", false);
	b = string_utils.replace_all(b, var_val_space, " ", false);
	game_instance->set_meta_value(a, b);
}

void set_max_hp_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	arg = string_utils.replace_all(arg, var_val_space, " ", false);
	if (string_utils.is_integer(arg))
	{
		entity::hp_t val = std::stoi(arg);
		c->set_max_hp(val);
	}
	else
	{
		err = "Max HP cannot be set to any non-integer value.";
	}
}

void set_hp_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	arg = string_utils.replace_all(arg, var_val_space, " ", false);
	if (string_utils.is_integer(arg))
	{
		entity::hp_t new_hp = std::stoi(arg);
		c->set_hp(new_hp);
	}
	else
	{
		err = "Error: Can only set HP to an integer value";
	}
}

void set_clear_on_scene_change_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	arg = string_utils.replace_all(arg, var_val_space, " ", false);
	std::vector<std::string> variable_names;
	std::vector<std::string> variable_values;
	bool val = c->evaluate_condition(game_instance, arg, err, variable_names, variable_values);
	if (err == "")
		game_instance->set_clear_on_scene_change(val);
}

void set_save_any_time_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	string_utils string_utils;
	std::string arg = string_utils.replace_all(args[0], { variable_value_header_char }, "", false);
	arg = string_utils.replace_all(arg, var_val_space, " ", false);
	std::vector<std::string> variable_names;
	std::vector<std::string> variable_values;
	bool val = c->evaluate_condition(game_instance, arg, err, variable_names, variable_values);
	if (err == "")
		game_instance->set_save_any_time(val);
}

void describe_scene_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	if(c == game_instance->get_perspective_entity())
		game_instance->describe_scene(c->get_scene());
}

void clear_func(game* game_instance, entity* c, std::vector<std::string>& args, std::string& err, const std::string& source)
{
	if (game_instance->get_perspective_entity()->get_scene() == c->get_scene())
		game_instance->get_engine()->clear_screen();
}

std::string entity::call_innate_function(game* game_instance, const std::string& function_name, std::vector<std::string>& args, const std::string& source)
{
	std::string err;
	
	pair_innate_function(&tell_func, function_name, "tell", args, game_instance, err, 0, 99, source);
	pair_innate_function(&set_value_func, function_name, "set_value", args, game_instance, err, 2, source);
	pair_innate_function(&add_alias_func, function_name, "add_alias", args, game_instance, err, 1, source);
	pair_innate_function(&add_title_func, function_name, "add_title", args, game_instance, err, 1, source);
	pair_innate_function(&learn_alias_func, function_name, "learn_alias", args, game_instance, err, 3, source);
	pair_innate_function(&learn_title_func, function_name, "learn_title", args, game_instance, err, 3, source);
	pair_innate_function(&transfer_func, function_name, "transfer", args, game_instance, err, 1, source);
	pair_innate_function(&say_func, function_name, "say", args, game_instance, err, 0, 99, source);
	pair_innate_function(&damage_func, function_name, "damage", args, game_instance, err, 1, 2, source);
	pair_innate_function(&attach_func, function_name, "attach", args, game_instance, err, 1, source);
	pair_innate_function(&attach_to_func, function_name, "attach_to", args, game_instance, err, 1, source);
	pair_innate_function(&unattach_func, function_name, "unattach", args, game_instance, err, 0, 1, source);
	pair_innate_function(&clone_func, function_name, "clone", args, game_instance, err, 1, source);
	pair_innate_function(&set_global_value_func, function_name, "set_global_value", args, game_instance, err, 2, source);
	pair_innate_function(&wipe_aliases_func, function_name, "wipe_aliases", args, game_instance, err, 2, source);
	pair_innate_function(&set_meta_value_func, function_name, "set_meta_value", args, game_instance, err, 2, source);
	pair_innate_function(&heal_func, function_name, "recover", args, game_instance, err, 1, 2, source);
	pair_innate_function(&set_max_hp_func, function_name, "set_max_hp", args, game_instance, err, 1, source);
	pair_innate_function(&set_hp_func, function_name, "set_hp", args, game_instance, err, 1, source);

	pair_innate_function(&set_clear_on_scene_change_func, function_name, "set_clear_on_scene_change", args, game_instance, err, 1, source);
	pair_innate_function(&set_save_any_time_func, function_name, "set_save_any_time", args, game_instance, err, 1, source);

	pair_innate_function(&describe_scene_func, function_name, "describe_scene", args, game_instance, err, 0, source);
	pair_innate_function(&clear_func, function_name, "clear", args, game_instance, err, 0, source);
	return err;
}

void entity::attach_to(entity* entity_to_attach_to)
{
	entity_to_attach_to->attach(this);
}

bool entity::is_perspective_entity() const
{
	return get_game_instance()->get_perspective_entity() == this;
}

bool entity::in_transfer_queue()
{
	return is_in_transfer_queue;
}

void entity::set_in_transfer_queue(bool val, const std::string& transfer_to)
{
	is_in_transfer_queue = val;
	transferring_to = transfer_to;
}

void entity::attach(entity* follower)
{
	attached_entity_names.push_back(follower->get_name());
	follower->attached_to = get_name();
}

void entity::damage(game* game_instance, game_obj* source, hp_t amount)
{
	if (amount < 0)
		amount = 0;
	std::string dummy_return_value;
	if (get_max_hp() >= 1)
	{
		
		current_command = ""; //Taking damage interrupts on-going commands.
		last_command_return_value = "TERMINATE";
		hp_t new_hp = get_hp() - amount;
		new_hp = set_hp(new_hp);
		interrupted = true;
		if (source != nullptr)
		{
			name_of_interrupter = source->get_name();
			call_function(game_instance, "on_damage_taken", { std::to_string(amount), source->get_name()}, dummy_return_value);
		}
		else
		{
			name_of_interrupter = "NULL";
			call_function(game_instance, "on_damage_taken", { std::to_string(amount), "NULL"}, dummy_return_value);
		}
	}
	else
	{
		if (source != nullptr)
		{
			name_of_interrupter = source->get_name();
			call_function(game_instance, "on_damage_taken", { "0", source->get_name()}, dummy_return_value);
		}
		else
		{
			name_of_interrupter = "NULL";
			call_function(game_instance, "on_damage_taken", { "0", "NULL" }, dummy_return_value);
		}
	}
}

void entity::damage(game* game_instance, hp_t amount)
{
	damage(game_instance, nullptr, amount);
}

void entity::copy_data_from(entity* c)
{
	known_aliases_mutex.lock();
	c->known_aliases_mutex.lock();

		known_aliases = c->known_aliases;
	
		set_max_hp(c->get_max_hp());
		set_hp(c->get_hp());

		copy_values_and_aliases_from(c);


	known_aliases_mutex.unlock();
	c->known_aliases_mutex.unlock();
}

void entity::execute_line_from_class(game* game_instance, line_num& line_number, std::string& err_msg, std::vector<std::string>& wildcards, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values)
{
	const std::string& line = get_line(line_number);
	string_utils string_utils;

	if (string_utils.matches_command("test", line, wildcards))
	{

	}
}

uint64_t entity::get_consecutive_command_repeats() const
{
	return command_repeats;
}

const std::list<std::string>& entity::get_attached_entity_names() const
{
	return attached_entity_names;
}

void entity::forget_aliases_for(const std::string& other_entity_name)
{
	auto found = known_aliases.find(other_entity_name);
	if (found != known_aliases.end())
	{
		known_aliases.erase(found);
	}
}

int32_t entity::get_hp() const
{
	hp_mutex.lock();
	hp_t val = hp;
	hp_mutex.unlock();
	return val;
}

int32_t entity::get_max_hp() const
{
	max_hp_mutex.lock();
	hp_t val = max_hp;
	max_hp_mutex.unlock();
	return val;
}

std::string entity::get_display_name(bool randomize, bool allow_titles, const std::vector<std::string>& known_names) const
{
	string_utils string_utils;
	std::string base = game_obj::get_display_name(randomize, allow_titles, known_names);
	if (base.length() >= 4 && base[0] == 't' && base[1] == 'h' && base[2] == 'e' && base[3] == ' ')
		return base;
	else
		return string_utils.format_as_name(base);
}

std::string entity::get_display_name(bool randomize, bool allow_titles) const
{
	std::string base = game_obj::get_display_name(randomize, allow_titles);
	string_utils string_utils;
	//If a name starts with 'the' then this is a title and does not need to capitalize the first letter of each "word"
	if (base.length() >= 4 && base[0] == 't' && base[1] == 'h' && base[2] == 'e' && base[3] == ' ')
		return base;
	else
		return string_utils.format_as_name(base);
}

std::string entity::get_display_name_of_other_entity(bool randomize, bool allow_titles, const std::string& category, entity* other_entity) const
{
	if (!randomize)
	{
		return other_entity->get_display_name(false, allow_titles);
	}
	else
	{
		if (known_aliases.find(other_entity->get_name()) == known_aliases.end())
		{
			return other_entity->get_display_name(true, allow_titles);
		}
		else
		{
			const auto& categories = known_aliases.at(other_entity->get_name());
			auto list_iterator = categories.find(category);
			if (list_iterator == categories.end())
			{
				return other_entity->get_display_name(true, allow_titles);
			}
			else
			{
				const std::vector<std::string>& list = list_iterator->second;
				return other_entity->get_display_name(true, allow_titles, list);
			}
		}
	}
}

std::string entity::get_display_name_of_other_entity(bool randomize, bool allow_titles, entity* other_entity) const
{
	if (!randomize)
	{
		return other_entity->get_display_name(false, allow_titles);
	}
	else
	{
		std::vector<std::string> possibilities;
		if (known_aliases.find(other_entity->get_name()) != known_aliases.end())
		{
			const std::map<std::string, std::vector<std::string>>& categories = known_aliases.at(other_entity->get_name());
			std::vector<std::string> master_list;
			for (auto category = categories.begin(); category != categories.end(); ++category)
			{
				const std::vector<std::string>& aliases_in_category = category->second;
				for (size_t i = 0; i < aliases_in_category.size(); ++i)
				{
					master_list.push_back(aliases_in_category[i]);
				}
			}

			return other_entity->get_display_name(true, allow_titles, master_list);
		}
			return other_entity->get_display_name(true, allow_titles);
	}
}

std::vector<std::string> entity::get_display_names_of_other_entity(bool allow_titles, entity* other_entity) const
{
	std::vector<std::string> possibilities;
	if (known_aliases.find(other_entity->get_name()) != known_aliases.end())
	{
		const std::map<std::string, std::vector<std::string>>& categories = known_aliases.at(other_entity->get_name());
		std::vector<std::string> master_list;
		for (auto category = categories.begin(); category != categories.end(); ++category)
		{
			const std::vector<std::string>& aliases_in_category = category->second;
			for (size_t i = 0; i < aliases_in_category.size(); ++i)
			{
				master_list.push_back(aliases_in_category[i]);
			}
		}

		return other_entity->get_display_names(allow_titles, master_list);
	}
	return other_entity->get_display_names(allow_titles);
}


const std::string& entity::get_attachment_name() const
{
	return attached_to;
}

const std::string& entity::get_interrupter() const
{
	return name_of_interrupter;
}

const std::string& entity::get_last_command() const
{
	return current_command;
}

const std::string& entity::get_scene_name() const
{
	return current_scene;
}

scene* entity::get_scene() const
{
	if (current_scene == "" || current_scene == "NULL")
		return nullptr;
	if (scene_ptr == nullptr) // FOR EXTRA POTENTIAL SAFETY: || scene_ptr->get_name() != current_scene
	{
		scene_ptr = get_game_instance()->get_scene(get_scene_name());
		return scene_ptr;
	}
	else
	{
		return scene_ptr;
	}
	
}

uint64_t entity::get_turn_number() const
{
	return current_turn;
}

bool entity::idle() const
{
	waiting_for_pc_mutex.lock();
	bool return_val = (has_never_entered_scene || current_turn >= get_game_instance()->get_perspective_entity()->get_turn_number()) && (!is_in_transfer_queue); // || waiting_for_pc
	waiting_for_pc_mutex.unlock();
	return return_val;
}

bool entity::knows_alias(std::string potential_alias, const entity* other_entity) const
{
	string_utils string_utils;
	potential_alias = string_utils.replace_all(potential_alias, variable_value_header, "", false);

	if (other_entity->has_alias(get_game_instance(),potential_alias))
		return true;

	engine* engine = get_game_instance()->get_engine();
	auto i = known_aliases.find(other_entity->get_name());
	
	if (i != known_aliases.end())
	{
		while (i != known_aliases.end())
		{
			const std::map<std::string, std::vector<std::string>>& categories_to_aliases_map = i->second;
			for(auto j = categories_to_aliases_map.begin(); j != categories_to_aliases_map.end(); ++j)
			{
				const std::vector<std::string>& aliases = j->second;
				for (size_t k = 0; k < aliases.size(); ++k)
				{
					std::string alias = string_utils.get_lowercase(aliases[k]);
					if (engine->extra_text_processing(alias, get_game_instance()) == engine->extra_text_processing(string_utils.get_lowercase(potential_alias), get_game_instance()))
						return true;
				}
			}
			++i;
		}
	}
	return false;
}

void entity::on_destroyed()
{
	scene* current_scene = get_scene();
	if (current_scene)
	{
		current_scene->get(get_game_instance(), "entities")->get(get_game_instance(), get_name())->destroy();
	}
}

void entity::interrupt()
{
	last_command_return_value = "NULL";
	name_of_interrupter = "NULL";
	interrupted = true;
}

void entity::learn_alias_for_entity(const std::string& entity_true_name, const std::string& alias_category, const std::string& alias)
{
	string_utils string_utils;
	std::string lowercase_alias = string_utils.get_lowercase(alias);
	std::string lowercase_entity_true_name = string_utils.get_lowercase(entity_true_name);
	std::string lowercase_alias_category = string_utils.get_lowercase(alias_category);
	std::vector<std::string> empty;
	if (known_aliases.find(lowercase_entity_true_name) == known_aliases.end())
	{
		std::map<std::string, std::vector<std::string>> def;
		def.insert_or_assign(lowercase_alias_category, empty);
		known_aliases.insert_or_assign(lowercase_entity_true_name, def);
	}
	else
	{
		const std::map<std::string, std::vector<std::string>>& category_map = known_aliases[lowercase_entity_true_name];
		if (category_map.find(lowercase_alias_category) == category_map.end())
		{
			known_aliases.at(lowercase_entity_true_name).insert_or_assign(lowercase_alias_category, empty);
		}
	}

	//Aliases should never be added redundently
	std::map<std::string, std::vector<std::string>>& category_map = known_aliases[lowercase_entity_true_name];
	std::vector<std::string>& alias_list = category_map.at(lowercase_alias_category);
	if(std::find(alias_list.begin(), alias_list.end(), lowercase_alias) == alias_list.end())
		alias_list.push_back(lowercase_alias);
}

game* entity::get_game_instance() const
{
	return static_cast<game*>(get_parent()->get_parent());
}

template <typename T>
void entity::pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int number_of_args, const std::string& source)
{
	if (checked_func_name == innate_function_name)
	{
		if (args.size() == number_of_args)
		{
			err = "";
			internal_func(game_instance, this, args, err, source);
		}
		else if (args.size() < number_of_args)
		{
			std::string args_string;
			for (int i = 0; i < args.size(); ++i)
				args_string += "'" + args[i] + "', ";

			if(args_string.size() > 1)
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

			err = "Too many arguments supplied for function " + 
				innate_function_name + 
				"; expected " + 
				std::to_string(number_of_args) + 
				", got " + 
				std::to_string(args.size()) + 
				" (" + args_string + ")";
		}
	}
}

template <typename T>
void entity::pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int min_args, int max_args, const std::string& source)
{
	if (checked_func_name == innate_function_name)
	{
		if (args.size() >= min_args && args.size() <= max_args)
		{
			err = "";
			internal_func(game_instance, this, args, err, source);
		}
		else if (args.size() < min_args)
		{
			std::string args_string;
			for (int i = 0; i < args.size(); ++i)
				args_string += "'" + args[i] + "', ";

			if (args_string.size() > 1)
				args_string.resize(args_string.size() - 2);

			err = "Too few arguments supplied for function '" +
				innate_function_name + "'; expected " +
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

void entity::process_line_from_file(const std::string& line)
{
	string_utils string_utils;
	
}

void entity::print(game* game_instance, const std::string& text)
{
	//narrative_queue += text;
	if (game_instance->get_perspective_entity() == this)
		game_instance->get_engine()->print(text);
}

void entity::println(game* game_instance, const std::string& line)
{
	//narrative_queue += line + "\n";
	if (game_instance->get_perspective_entity() == this)
		game_instance->get_engine()->println(line);
}

void entity::remove_from_scene()
{
	scene* current_scene = get_scene();
	std::string dummy_return_value;
	
	if (current_scene) //It has to remove the reference to itself in the scene it currently exists in.
	{
		current_scene->finalize_entity_removal();
		game_obj* current_reference_to_self = current_scene->get(get_game_instance(), "entities/" + get_name());
		if (current_reference_to_self)
			current_reference_to_self->destroy();
		scene_ptr = nullptr;
		this->current_scene = "NULL";
	}
}

bool entity::resolve_input(game* game_instance, entity* user, const std::string& input, std::string& reason_for_failure)
{
	line_num custom_func_line = 0;
	string_utils string_utils;
	std::vector<std::string> dummy_var_names;
	std::vector<std::string> dummy_var_values;
	reason_for_failure = "";
	if (get_command_func_lines().size() != 0)
	{
		for (size_t custom_func_line_index = 0; custom_func_line_index < get_command_func_lines().size(); ++custom_func_line_index)
		{
			custom_func_line = get_command_func_lines()[custom_func_line_index];
			std::string line = get_line(custom_func_line);
			std::vector<std::string> wildcards;
			if (string_utils.matches_command("function command: $func ( $args )", line, wildcards, " ():", false))
			{
				//std::cout << "MATCHED " << line << " AGAINST " << "function command: $func ( $args )" << std::endl;
				std::string actual_func_name = wildcards[0];
				std::string expected_args = wildcards[1];

				std::string check = "function command:" + input + "( " + expected_args + " )";

				if (string_utils.matches_command(line, check, wildcards, " ():", false))
				{
					std::vector<std::string> args;
					args.push_back(user->get_name());
					for (size_t i = 0; i < wildcards.size(); ++i)
						args.push_back(wildcards[i]);

					std::string err = call_function(game_instance, "command:" + actual_func_name, args, reason_for_failure);
					reason_for_failure = string_utils.replace_all(reason_for_failure, variable_value_header, "", false);
					reason_for_failure = string_utils.replace_all(reason_for_failure, {dummy_space}, " ", false);
					reason_for_failure = string_utils.replace_all(reason_for_failure, var_val_space, " ", false);
					string_utils.strip(reason_for_failure);
					string_utils.make_lowercase(reason_for_failure);
					if (reason_for_failure == "no_match" || reason_for_failure == "no match")
						continue;
					//std::string lowercase_last_command_return_value = string_utils.get_lowercase(reason_for_failure);
					
					/*
					bool successful_return_value = (
						lowercase_last_command_return_value == "success" ||
						lowercase_last_command_return_value == "succeed" ||
						lowercase_last_command_return_value == "good" ||
						lowercase_last_command_return_value == "finish" ||
						lowercase_last_command_return_value == "end" ||
						lowercase_last_command_return_value == "terminate" ||
						lowercase_last_command_return_value == "repeat" ||
						lowercase_last_command_return_value == "loop" ||
						lowercase_last_command_return_value == "continue" ||
						lowercase_last_command_return_value == "null" ||
						lowercase_last_command_return_value == "done"
						);

					if (err == "" && successful_return_value)
						return true;
					else if (err == "" && !successful_return_value)
					{
						return false;
					}
					*/
					if (err == "")
						return true;
					else
						return false;
				}
			}
		}
	}
	return false;
}

scene* entity::set_to_scene(const std::string& scene_name)
{
	scene* new_scene = dynamic_cast<scene*>(get_game_instance()->get(get_game_instance(), "scenes/" + scene_name));
	if (new_scene)
	{
		remove_from_scene();
		current_scene = scene_name; //Update my reference
		std::string dummy_return_value;
		new_scene->get(get_game_instance(), "entities")->add_child(new dummy(get_name())); //Add a reference to myself to the new scene
		scene_ptr = new_scene;
		has_never_entered_scene = false;
		set_in_transfer_queue(false, "");

		if (get_game_instance()->get_perspective_entity() == this)
		{
			new_scene->call_function(get_game_instance(), "describe");
		}

		new_scene->call_function(get_game_instance(), "on_enter_scene", { get_name() }, dummy_return_value);
		call_function(get_game_instance(), "on_enter_scene");

		const std::vector<entity*>& other_entities_in_scene = new_scene->get_entities_in_scene();
		for (size_t i = 0; i < other_entities_in_scene.size(); ++i)
		{
			entity* sibling = other_entities_in_scene[i];
			if (sibling != this && !sibling->in_transfer_queue())
			{
				sibling->call_function(get_game_instance(), "on_entity_approach", { get_name() }, dummy_return_value);
				call_function(get_game_instance(), "on_entity_approach", { sibling->get_name() }, dummy_return_value);
			}
		}
		return new_scene;
	}
	else
	{
		return nullptr;
	}
}

entity::hp_t entity::recover(hp_t amount)
{
	return recover(nullptr, amount);
}

entity::hp_t entity::recover(game_obj* source, hp_t amount)
{
	if (amount < 0)
		amount = 0;
	hp_t old_hp = get_hp();
	hp_t new_hp = set_hp(old_hp + amount);
	amount = new_hp - old_hp;
	std::string dummy_return_val;
	if(source == nullptr)
		call_function(get_game_instance(), "on_recover", { "NULL", std::to_string(amount)}, dummy_return_val);
	else
		call_function(get_game_instance(), "on_recover", { source->get_name(), std::to_string(amount)}, dummy_return_val);

	return amount;
}

void entity::set_turn_number(uint64_t turn_num)
{
	current_turn = turn_num;
}

bool entity::take_turn(game* game_instance, std::string& reason_for_failure)
{
	waiting_for_pc_mutex.lock();
	waiting_for_pc = false;
	waiting_for_pc_mutex.unlock();
	std::string input;
	engine* engine = game_instance->get_engine();
	string_utils string_utils;

	auto get_input = [&](std::string& input)
	{
		interrupted = false;
		name_of_interrupter = "NULL";
		if (game_instance->get_perspective_entity() == this) //If this entity is the perspective entity (player), then it gets its command from user input.
		{
			//engine->println("");
			//engine->print("     > ");
			
			if (!game_instance->game_is_active())
			{
				engine->println("Enter anything to return to the main menu");
				std::cout << PRINT_PREAMBLE;
				std::getline(std::cin, input);
				input = "quit";
			}
			else
			{
				input = engine->extra_text_processing(engine->get_input(), get_game_instance());
				engine->println("");
			}
		}
		else //If this *isn't* the player, then it autonomously executes its own commands. The command is controlled via the "get_ai_command" function. This isn't the only way to control automatic entity behavior, but it does allow for more dynamic and "open-world" interactions between them.
		{
			std::vector<std::string> empty_args;
			waiting_for_pc_mutex.lock();
			waiting_for_pc = true;
			waiting_for_pc_mutex.unlock();
			if (current_turn != game_instance->get_current_turn()) //Time advances at the speed of the player, so entities cannot advance further than 1 ahead of the player otherwise they'll desync.
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			waiting_for_pc_mutex.lock();
			waiting_for_pc = false;
			waiting_for_pc_mutex.unlock();
			
			call_function(game_instance, "get_ai_command", empty_args, input);
			for (int i = 0; i < input.size(); ++i)
			{
				engine::swap_from_dummy_char(input[i]);
			}
			if (game_instance->game_is_active())
				input = game_instance->get_engine()->extra_text_processing(input, get_game_instance());
			else
				input = "quit";
		}
	};

	std::string lowercase_last_command_return_value = string_utils.get_lowercase(last_command_return_value);
	string_utils.strip(lowercase_last_command_return_value);
	if (lowercase_last_command_return_value != "continue" && 
		lowercase_last_command_return_value != "repeat" && 
		lowercase_last_command_return_value != "loop") //This way a default-value command function will resolve immediately
	{
		get_input(current_command);
		command_repeats = 0;
	}
	else
	{
		++command_repeats;
	}

	//++current_turn; //Advance the turn counter.
	input = current_command;

	narrative_queue = "";
	if (input != "do nothing")
	{
		bool action_success = false;
		action_success = game_instance->resolve_input(game_instance, this, input, last_command_return_value); //First check if this is a game command

		if(!action_success)
			action_success = get_scene()->resolve_input(game_instance, this, input, last_command_return_value); //Then if it's a scene command

		if (!action_success) //And finally against all options given by entities present in the current scene
		{
			const auto& siblings = get_scene()->get_entities_in_scene();
			for (auto i = siblings.begin(); i != siblings.end(); ++i)
			{
				entity* sibling = *i;
				action_success = sibling->resolve_input(game_instance, this, input, last_command_return_value);
				if (action_success)
					break;
			}

		}

		lowercase_last_command_return_value = string_utils.get_lowercase(last_command_return_value);
		string_utils.strip(lowercase_last_command_return_value);
		lowercase_last_command_return_value = string_utils.replace_all(lowercase_last_command_return_value, variable_value_header, "", false);
		lowercase_last_command_return_value = string_utils.replace_all(lowercase_last_command_return_value, var_val_space, " ", false);
		if (lowercase_last_command_return_value == "failure" || lowercase_last_command_return_value == "fail" || lowercase_last_command_return_value == "no match" || lowercase_last_command_return_value == "no_match")
		{
			//std::cout << "HERE1\n";
			reason_for_failure = "";
			waiting_for_pc_mutex.lock();
			waiting_for_pc = true;
			waiting_for_pc_mutex.unlock();
			return false;
		}
		else if (action_success && 
			(lowercase_last_command_return_value == "success" ||
			lowercase_last_command_return_value == "succeed" ||
			lowercase_last_command_return_value == "good" || 
			lowercase_last_command_return_value == "finish" ||
			lowercase_last_command_return_value == "end" ||
			lowercase_last_command_return_value == "terminate" ||
			lowercase_last_command_return_value == "repeat" ||
			lowercase_last_command_return_value == "loop" ||
			lowercase_last_command_return_value == "continue" ||
			lowercase_last_command_return_value == "null" ||
			lowercase_last_command_return_value == "done"
			))
		{
			waiting_for_pc_mutex.lock();
			waiting_for_pc = true;
			waiting_for_pc_mutex.unlock();
			return true;
		}
		else if (action_success)
		{
			reason_for_failure = last_command_return_value;
			waiting_for_pc_mutex.lock();
			waiting_for_pc = true;
			waiting_for_pc_mutex.unlock();
			return false;
		}
		else//If it still failed to do anything it means it tried to pick an invalid choice.
		{
			waiting_for_pc_mutex.lock();
			//if (!is_perspective_entity())
				waiting_for_pc = true;
			waiting_for_pc_mutex.unlock();
			reason_for_failure = "";
			return false;
		}
	}

	waiting_for_pc_mutex.lock();
	//if (!is_perspective_entity())
		waiting_for_pc = true;
	waiting_for_pc_mutex.unlock();
	return true;
}

void entity::unattach(const std::string& follower)
{
	auto found = std::find(attached_entity_names.begin(), attached_entity_names.end(), follower);
	if (found != attached_entity_names.end())
	{
		//std::cout << "UNATTACHING " << follower << " FROM " << get_name() << std::endl;
		attached_entity_names.remove(follower);
		entity* existing_entity = get_game_instance()->get_entity_by_name(follower, get_name());
		if (existing_entity)
		{
			existing_entity->attached_to = "NULL";
		}
	}
	else
	{
		//std::cout << "DID NOT FIND " << follower << " IN " << get_name() << " ATTACHMENT LIST.\n";
	}
}

entity::hp_t entity::set_hp(hp_t new_hp)
{
	hp_mutex.lock();
	hp_t old_hp = hp;
	hp_t val = new_hp;
	if (val > max_hp)
		val =  max_hp;
	hp = val;
	hp_mutex.unlock();
	std::string dummy_return_val;
	call_function(get_game_instance(), "on_hp_change", { std::to_string(old_hp), std::to_string(val) }, dummy_return_val);
	return val;
}

void entity::set_max_hp(hp_t new_max_hp)
{
	hp_t old_max = get_max_hp();
	max_hp_mutex.lock();
	max_hp = new_max_hp;
	if (hp > max_hp)
		hp = max_hp;
	max_hp_mutex.unlock();
	std::string dummy_return_val;
	call_function(get_game_instance(), "on_max_hp_change", { std::to_string(old_max), std::to_string(new_max_hp) }, dummy_return_val);
}

void entity::unattach()
{
	if (attached_to != "NULL")
	{
		entity* attachment = get_game_instance()->get_entity_by_name(attached_to, get_name());
		if (attachment)
		{
			attachment->unattach(get_name());
		}
		attached_to = "NULL";
	}
}


bool entity::was_interrupted() const
{
	return interrupted;
}


void entity::wipe_aliases(game* game_instance)
{
	delete_aliases();
	const auto& all_entities = game_instance->get_entities();
	for (auto i = all_entities.begin(); i != all_entities.end(); ++i)
	{
		entity* ent = *i;
		ent->forget_aliases_for(get_name());
	}
}