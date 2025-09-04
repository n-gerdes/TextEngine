#include "engine/headers/instance/game_obj.h"
#include "engine/headers/instance/game_obj_save_registry.h"
#include "engine/headers/util/string_utils.h"
#include "engine/headers/engine.h"
#include "engine/headers/instance/game.h"
#include <random>

game_obj::child_t* game_obj::add_child(child_t* new_child)
{
	if (reinterpret_cast<void*>(new_child->get_parent()) != reinterpret_cast<void*>(this))
	{
		parent_t* old_parent = reinterpret_cast<parent_t*>(new_child->get_parent());
		if (old_parent != nullptr)
		{
			new_child->detach();
		}
		children.push_back(new_child);
		new_child->parent = this;
		child_added(new_child);
		new_child->on_added(reinterpret_cast<child_t::parent_t*>(this));
		return new_child;
	}
	else
	{
		return nullptr;
	}
}

void game_obj::add_alias(const std::string& alias)
{
	alias_mutex.lock();
	string_utils string_utils;
	std::string lowercase_alias = string_utils.get_lowercase(alias);
	if(   std::find(aliases.begin(), aliases.end(), lowercase_alias) == aliases.end()   )
		aliases.push_back(string_utils.get_lowercase(alias));
	alias_mutex.unlock();
}

void game_obj::add_title(const std::string& alias)
{
	alias_mutex.lock();
	string_utils string_utils;
	std::string lowercase_alias = string_utils.get_lowercase(alias);
	if (std::find(aliases.begin(), aliases.end(), lowercase_alias) == aliases.end())
	{
		aliases.push_back(lowercase_alias);
		aliases.push_back("the " + lowercase_alias);
	}
	alias_mutex.unlock();
}

void game_obj::copy_values_and_aliases_from(game_obj* o)
{
	o->var_mutex.lock();
	var_mutex.lock();
	variables = o->variables;
	var_mutex.unlock();
	o->var_mutex.unlock();

	o->alias_mutex.lock();
	alias_mutex.lock();
	aliases = o->aliases;
	alias_mutex.unlock();
	o->alias_mutex.unlock();
}

void game_obj::delete_aliases()
{
	aliases.clear();
}

void game_obj::destroy()
{
	while (children.size() > 0)
	{
		children.back()->destroy();
	}
	//detach();
	delete this;
}

game_obj* game_obj::detach()
{
	if (parent == nullptr)
		return nullptr;
	return parent->remove(this);
}

std::vector<game_obj*> game_obj::find_children(game* game_instance, const std::string& child_name) const
{
	std::vector<game_obj*> list;
	list.reserve(children.size());
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		game_obj* child = *i;
		if (child->get_name() == child_name || child->has_alias(game_instance, child_name))
			list.push_back(child);
	}
	return list;
}

game_obj* game_obj::find_first_child(game* game_instance, const std::string& child_name) const
{
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		if ((*i)->get_name() == child_name || (*i)->has_alias(game_instance, child_name))
			return *i;
	}
	return nullptr;
}

game_obj* game_obj::find_first_child(game* game_instance, const std::string& child_name, bool can_use_true_name, bool can_use_alias) const
{
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		if ((can_use_true_name && (*i)->get_name() == child_name) || (can_use_alias && (*i)->has_alias(game_instance, child_name)))
			return *i;
	}
	return nullptr;
}

game_obj::~game_obj()
{
	detach();
	on_destroyed();
}

game_obj* game_obj::get(game* game_instance, const std::string& path)
{
	string_utils string_utils;
	game_obj* found = this;

	const std::vector<std::string>& raw_tokens = string_utils.extract_tokens(path, "/");

	std::vector<std::string> tokens;
	tokens.reserve(raw_tokens.size());
	for (size_t i = 0; i < raw_tokens.size(); ++i)
	{
		if (raw_tokens[i] != "/")
			tokens.push_back(raw_tokens[i]);
	}

	for (size_t i = 0; i < tokens.size() && found!=nullptr; ++i)
	{
		const std::string& token = tokens[i];
		if (token == "..")
		{
			found = found->get_parent();
		}
		else
		{
			found = found->find_first_child(game_instance, token, true, false);
		}
	}

	return found;
}

std::string game_obj::get_display_name(bool randomize, bool allow_titles, const std::vector<std::string>& known_names) const
{
	std::vector<std::string> possible_names;
	possible_names.reserve(known_names.size() + aliases.size());
	//First it adds privately known names.
	for (size_t i = 0; i < known_names.size(); ++i) //Next it will add all aliases and titles
	{
		const std::string& alias = known_names[i];
		if (alias.find("~") == std::string::npos)
		{
			if (i == known_names.size() - 1) //If it's the last one then it can't be a title
			{
				possible_names.push_back(alias);
			}
			else
			{
				if (known_names[i + 1] == "the " + alias) //If the next one on the list is "the [blank]" then this alias is a title, and the title must be added as a possible display name but not this (i.e., "the wizard casts a spell" vs "wizard casts a spell").
				{
					if (allow_titles)
						possible_names.push_back(known_names[i + 1]);
					++i; //Since it would've checked the next alias (a known title) to see if it was a title (when I know this one was) then there is no need to check it again; increment i so when it increments again it skips to the next unique alias.
				}
			}
		}
	}

	for (size_t i = 0; i < aliases.size(); ++i) //Next it will add all aliases and titles
	{
		const std::string& alias = aliases[i];
		if (alias.find("~") == std::string::npos)
		{
			if (i == aliases.size() - 1) //If it's the last one then it can't be a title
			{
				possible_names.push_back(alias);
			}
			else
			{
				if (aliases[i + 1] == "the " + alias) //If the next one on the list is "the [blank]" then this alias is a title, and the title must be added as a possible display name but not this (i.e., "the wizard casts a spell" vs "wizard casts a spell").
				{
					if (allow_titles)
						possible_names.push_back(aliases[i + 1]);
					++i; //Since it would've checked the next alias (a known title) to see if it was a title (when I know this one was) then there is no need to check it again; increment i so when it increments again it skips to the next unique alias.
				}
			}
		}
	}

	if (possible_names.size() == 0)
		return "err_no_allowed_display_name";

	string_utils string_utils;
;
	if (randomize)
		return string_utils.get_lowercase(possible_names[rand() % possible_names.size()]);
	else
		return string_utils.get_lowercase(possible_names[0]);
}

std::string game_obj::get_display_name(bool randomize, bool allow_titles) const
{
	std::vector<std::string> possible_names;
	possible_names.reserve(aliases.size() + 1);

	if (game_obj_name.find("~") == std::string::npos) //A non-ID name can be added to the list of possible display names
	{
		possible_names.push_back(game_obj_name);
	}

	for (size_t i = 0; i < aliases.size(); ++i) //Next it will add all aliases and titles
	{
		const std::string& alias = aliases[i];
		if (alias.find("~") == std::string::npos)
		{
			if (i == aliases.size() - 1) //If it's the last one then it can't be a title
			{
				possible_names.push_back(alias);
			}
			else
			{
				if (aliases[i + 1] == "the " + alias) //If the next one on the list is "the [blank]" then this alias is a title, and the title must be added as a possible display name but not this (i.e., "the wizard casts a spell" vs "wizard casts a spell").
				{
					if (allow_titles)
						possible_names.push_back(aliases[i + 1]);
					++i; //Since it would've checked the next alias (a known title) to see if it was a title (when I know this one was) then there is no need to check it again; increment i so when it increments again it skips to the next unique alias.
				}
			}
		}
	}

	if (possible_names.size() == 0)
		return "err_no_allowed_display_name";

	string_utils string_utils;
	;
	if (randomize)
		return string_utils.get_lowercase(possible_names[rand() % possible_names.size()]);
	else
		return string_utils.get_lowercase(possible_names[0]);
}

const game_obj::game_obj_storage_t& game_obj::get_children() const
{
	return children;
}

uint32_t game_obj::get_save_id() const
{
	return 0;
}

const std::string& game_obj::get_name() const
{
	return game_obj_name;
}

game_obj::parent_t* game_obj::get_parent() const
{
	return parent;
}

std::string game_obj::get_value(const std::string& variable_name) const
{
	var_mutex.lock();
	auto found = variables.find(variable_name);
	if (found == variables.end())
	{
		var_mutex.unlock();
		return "0";
	}
	else
	{
		std::string val = found->second;
		var_mutex.unlock();
		return val;
	}
	
}

bool game_obj::has_alias(game* game_instance, const std::string& potential_alias) const
{
	alias_mutex.lock();
	std::string processed_alias = game_instance->get_engine()->extra_text_processing(potential_alias, game_instance);
	string_utils string_utils;
	for (size_t i = 0; i < aliases.size(); ++i)
	{
		std::string alias = game_instance->get_engine()->extra_text_processing(aliases[i], game_instance);
		if (alias == processed_alias)
		{
			alias_mutex.unlock();
			return true;
		}
		
	}
	alias_mutex.unlock();
	return false;
	//return std::find(aliases.begin(), aliases.end(), string_utils.get_lowercase(potential_alias)) != aliases.end();
}

game_obj* game_obj::instance() const
{
	return new game_obj();
}

void game_obj::load_children(std::ifstream& file, engine& game_engine, const std::string& scenario_name, engine* engine)
{
	while (children.size() > 0) //When loading children, it has to clear out any that already exist.
		children.back()->destroy();

	game_obj_save_registry* game_obj_registry = game_engine.get_game_obj_save_registry();
	uint32_t save_id = 0;
	uint64_t number_of_children = 0;
	load_uint64_t(file, number_of_children);
	for (uint64_t i = 0; i < number_of_children; ++i)
	{
		load_uint32_t(file, save_id);
		child_t* new_child = dynamic_cast<child_t*>(game_obj_registry->get_instance(save_id));
		if (new_child)
		{
			add_child(new_child);
			new_child->load_from_file(file, game_engine, scenario_name, engine);
		}
		else
		{
			std::exception E;
			throw E;
		}
	}
}

void game_obj::load_from_file(std::ifstream& file, engine& game_engine, const std::string& scenario_name, engine* engine)
{
	save_mutex.lock();
	load_children(file, game_engine, scenario_name, engine); //The first value each one saves is the Save ID, but that gets loaded before this function
	//call so that it knows what class to instantiate. First thing it does is load its children.

	load_string(file, game_obj_name); //Next it needs the object's name.

	uint32_t number_of_aliases = 0;
	load_uint32_t(file, number_of_aliases); //It loads in the number of aliases that were assigned to this object
	aliases.resize(number_of_aliases);
	for (size_t i = 0; i < number_of_aliases; ++i) //Once it knows how many aliases to load, it simply loads them in sequentially
	{
		load_string(file, aliases[i]);
	}

	uint32_t& number_of_variables = number_of_aliases; //Reusing memory for this variable. To do otherwise would be wasteful.
	std::string dynamic_variable_value = "";
	load_uint32_t(file, number_of_variables); //Now it has to load dynamic user-generated variables. First start with the number.
	std::string dynamic_variable_name = "";

	for (size_t i = 0; i < number_of_variables; ++i) //Now that it knows how many to load, it just has to load them.
	{
		load_string(file, dynamic_variable_name); //Needs to get the variable's name
		load_string(file, dynamic_variable_value); //And its value
		set_value(dynamic_variable_name, dynamic_variable_value); //And then it can assign it.
	}

	load_variables(file, scenario_name, engine); //Now load hardcoded C++ class variables from game_obj subclasses.
	save_mutex.unlock();
}

void game_obj::print_hierarchy() const
{
	print_hierarchy(0);
}

void game_obj::print_hierarchy(int layer) const
{
	for (int i = 0; i < layer; ++i)
	{
		std::cout << "   ";
	}
	std::cout << get_name() << std::endl;

	for (auto i = children.begin(); i != children.end(); ++i)
		(*i)->print_hierarchy(layer + 1);

}

game_obj::child_t* game_obj::remove(child_t* current_child)
{
	if (current_child->get_parent() == this)
	{
		children.remove(current_child);
		current_child->parent = nullptr;
		current_child->on_removed(this);
		child_removed(current_child);
		return current_child;
	}
	return nullptr;
}

void game_obj::save_children(std::ofstream& file, const std::string& scenario_name, engine* engine) const
{
	save_uint64_t(file, static_cast<uint64_t>(children.size()));
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		(*i)->save_to_file(file, scenario_name, engine);
	}
}

void game_obj::save_to_file(std::ofstream& file, const std::string& scenario_name, engine* engine) const
{
	save_mutex.lock();
	save_uint32_t(file, get_save_id()); //This is the Save ID for this class so that it knows what class to instantiate when loading
	save_children(file, scenario_name, engine); //Then it saves its children
	save_string(file, game_obj_name); //Then it saves the name

	save_uint32_t(file, static_cast<uint32_t>(aliases.size())); //Now it saves the number of aliases it has
	for (size_t i = 0; i < aliases.size(); ++i) //And then saves each of the aliases.
	{
		save_string(file, aliases[i]);
	}

	save_uint32_t(file, static_cast<uint32_t>(variables.size())); //It also has to save the number of named user variables it has
	for (auto i = variables.begin(); i != variables.end(); ++i) //Then save the user variables
	{
		save_string(file, i->first);
		save_string(file, i->second);
	}

	save_variables(file, scenario_name, engine); //A virtual function meant to be overridden by subclasses. It saves hardcoded C++ class variables from
	//subclasses of game_obj, as opposed to user-generated dynamic variables.
	save_mutex.unlock();
}

std::string& game_obj::set_name(const std::string& name)
{
	game_obj_name = name;
	return game_obj_name;
}

void game_obj::set_value(const std::string& variable_name, const std::string& new_value)
{
	var_mutex.lock();
	variables.insert_or_assign(variable_name, new_value);
	var_mutex.unlock();
}
