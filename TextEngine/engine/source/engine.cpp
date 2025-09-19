#include "engine/headers/engine.h"
#include "engine/headers/instance/game_obj_save_registry.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/scene.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/procedure/file_reader.h"
#include <mutex>
#include <iostream>
#include <fstream>

#if DEV_MODE
#include <filesystem>
#endif

std::mutex engine::print_mutex;
char engine::last_character_printed = '\n';
std::vector<std::string> engine::baked_scenarios_registry;
const std::string engine::CONSOLE_DEFAULT = "\033[0;0m";

void engine::register_baked_scenario(const std::string& scenario_name)
{
	baked_scenarios_registry.push_back(scenario_name);
}

entity* engine::AMBIGUOUS_CHARACTER() const
{
	return reinterpret_cast<entity*>(1); //This ugly hack assumes that I will never have access to or allocate a entity at memory address 1, so the value should be free to use like an enum to denote a return value in which more than one entity could be returned.
}

void engine::clear_screen()
{
	clear_screen(true);
}

void engine::clear_screen(bool extra_line)
{
	print_mutex.lock();
	last_character_printed = '\n';
	if (!FULL_CLEAR)
	{
		for (int i = 0; i < 100; ++i)
		{
			std::cout << std::endl;
		}
	}

#ifdef _WIN32
	if (FULL_CLEAR)system("cls");
#endif

#if defined(__linux__) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(__MACH__)
	if (FULL_CLEAR)system("clear");
#endif
	if(extra_line)
		std::cout << std::endl;
	print_mutex.unlock();
}

std::string engine::correct_tokenizer_bug(const std::string original) const
{
	string_utils string_utils;
	std::string modified = string_utils.replace_all(original, " .", ".", false);

	std::string dummy_left_paran_string = "0";
	std::string dummy_right_paran_string = "0";
	dummy_left_paran_string[0] = dummy_left_paran;
	dummy_right_paran_string[0] = dummy_right_paran;

	modified = " " + modified + " ";
	modified = string_utils.replace_all(modified, " "+dummy_right_paran_string+" ", dummy_right_paran_string +" ", false);
	modified = string_utils.replace_all(modified, " "+dummy_left_paran_string+" ", " "+dummy_left_paran_string, false);

	modified = string_utils.replace_all(modified, dummy_left_paran_string + " ", " " + dummy_left_paran_string, false);
	modified = string_utils.replace_all(modified, " " + dummy_right_paran_string, dummy_right_paran_string + " ", false);

	modified = string_utils.replace_all(modified, " ) ", ") ", false);
	modified = string_utils.replace_all(modified, " ( ", " (", false);

	modified = string_utils.replace_all(modified, "( ", " (", false);
	modified = string_utils.replace_all(modified, " )", ") ", false);

	string_utils.strip(modified);
	std::string dummy1 = " ,";
	std::string dummy2 = ",";
	dummy1[1] = dummy_comma;
	dummy2[0] = dummy_comma;
	modified = string_utils.replace_all(modified, dummy1, dummy2, false);
	return modified;
}

engine::engine()
{
	std::cout << "Initializing..." << std::endl;
	baked_scenarios_registry.clear();
	file_reader::clear_internal_files();
	file_reader::bake();
	last_character_printed = '\n';
	save_registry = new game_obj_save_registry();
	
	subs.load_input_substitution("gamedata/input_substitution.dat");

	subs.load_output_substitution_file("gamedata/", "generic");
	subs.load_output_substitution_file("gamedata/", "casual");
	subs.load_output_substitution_file("gamedata/", "formal");
	subs.load_output_substitution_file("gamedata/", "technical");
	subs.load_output_substitution_file("gamedata/", "medieval");
	clear_screen(false);
}

engine::~engine()
{
	save_registry->cleanup_registry();
	delete save_registry;
}

std::string engine::extra_text_processing(const std::string& original_text, game* game_instance) const
{
	string_utils string_utils;
	std::string modified = original_text;

	for (int i = 0; i < modified.size(); ++i)
	{
		swap_from_dummy_char(modified[i]);
	}

	modified = string_utils.replace_all(modified, "  ", " ", false);
	modified = string_utils.replace_all(modified, ".", "", false);
	modified = string_utils.replace_all(modified, ", ", "", false);
	//string_utils.make_lowercase(input);
	modified = string_utils.replace_all(modified, "i'd like you to", "", false);
	modified = string_utils.replace_all(modified, "i'd love you to", "", false);
	modified = string_utils.replace_all(modified, "i'd like for you to", "", false);
	modified = string_utils.replace_all(modified, "i'd love for you to", "", false);
	modified = string_utils.replace_all(modified, "if you will", "", false);
	modified = string_utils.replace_all(modified, "can you", "", false);
	string_utils.strip(modified);


	if (string_utils.starts_with(modified, "I ", false))
		modified = string_utils.replace_first(modified, "I ", "", false);

	//This handles all the basic input thesaurus stuff
	bool substitution_override = false;
	if (game_instance != nullptr)
	{
		if (game_instance->has_input_substitution_override() && game_instance->get_substitution_wizard()->has_input_substitution())
			substitution_override = true;
	}
	if(!substitution_override)
		subs.apply_input_substitution(modified);

	if (game_instance != nullptr)
	{
		game_instance->get_substitution_wizard()->apply_input_substitution(modified);
	}
	string_utils.strip(modified);
	string_utils.replace_all(modified, "  ", " ", false);
	return modified;
}

std::string engine::get_scenario_directory(const std::string& scenario_name) const
{
	return "Scenarios/" + scenario_name + "/";
}

std::string engine::get_saves_directory(const std::string& scenario_name) const
{
	return get_scenario_directory(scenario_name) + "saves/";
}

void engine::get_input(std::string* raw, std::string* case_preserved, std::string* processed) const
{
	auto swap_away_dummy_chars = [&](std::string& str)
		{
			for (size_t i = 0; i < str.size(); ++i)
			{
				engine::swap_from_dummy_char(str[i]);
			}
		};
	string_utils string_utils;
	if (raw == nullptr && case_preserved == nullptr && processed == nullptr) //For some reason I ask for nothing.
	{
		return;
	}
	else if (raw != nullptr && case_preserved == nullptr && processed == nullptr) //I want only the raw input
	{
		//println();
		print("     > ");
		std::getline(std::cin, *raw);
		last_character_printed = '\n';
		println();
	}
	else if (raw == nullptr && case_preserved != nullptr && processed == nullptr) //I want only the case preserved input
	{
		//println();
		print("     > ");
		std::getline(std::cin, *case_preserved);
		string_utils.strip(*case_preserved);
		*case_preserved = string_utils.replace_all(*case_preserved, "~", "", false);
		last_character_printed = '\n';
		println();
	}
	else if (raw == nullptr && case_preserved == nullptr && processed != nullptr) //I want only the fully processed input
	{
		//println();
		print("     > ");
		std::getline(std::cin, *processed);
		string_utils.strip(*processed);
		string_utils.make_lowercase(*processed);
		*processed = string_utils.replace_all(*processed, "~", "", false);
		last_character_printed = '\n';
		println();
	}
	else if (raw != nullptr && case_preserved == nullptr && processed != nullptr) //I want the raw and fully processed input
	{
		//println();
		print("     > ");
		std::getline(std::cin, *raw);
		*processed = *raw;
		string_utils.strip(*processed);
		string_utils.make_lowercase(*processed);
		*processed = string_utils.replace_all(*processed, "~", "", false);
		last_character_printed = '\n';
		println();
	}
	else if (raw != nullptr && case_preserved != nullptr && processed != nullptr) //I want all three intermediary stages
	{
		//println();
		print("     > ");
		std::getline(std::cin, *raw);
		*case_preserved = *raw;
		string_utils.strip(*case_preserved);
		*case_preserved = string_utils.replace_all(*case_preserved, "~", "", false);
		*processed = *case_preserved;
		string_utils.make_lowercase(*processed);
		last_character_printed = '\n';
		println();
	}
}

std::string engine::get_input() const
{
	std::string destination;
	get_input(nullptr, nullptr, &destination);
	return destination;
}

int engine::get_integer_input() const
{
	std::string input = "";
	bool got_input = false;
	int intput = 0;
	while (!got_input)
	{
		input = get_input();
		try
		{
			intput = std::stoi(input);
			got_input = true;
		}
		catch (const std::exception& E)
		{
			
		}
	}

	return intput;
}

int engine::get_integer_input(int min, int max) const
{
	int choice = min - 1;
	while (choice < min || choice > max)
		choice = get_integer_input();

	return choice;
}

game_obj_save_registry* engine::get_game_obj_save_registry()
{
	return save_registry;
}

game* engine::load_game(const std::string& scenario_name, const std::string& save_file)
{
	println("Attempting to load ", save_file);
	game* game_instance = new game(scenario_name, this);
	uint32_t dummy_save_id;

	std::ifstream input_file;
	std::string save_file_name = get_scenario_directory(scenario_name) + "saves/" + save_file;
	input_file.open(save_file_name, std::ios::binary);

	if (input_file.is_open())
	{
		game_instance->load_uint32_t(input_file, dummy_save_id);
		if (dummy_save_id != game_instance->get_save_id())
		{
			println("Error: save file corrupt");
			game_instance->destroy();
			return nullptr;
		}
		else
		{
			try
			{
				clear_screen();
				game_instance->load_from_file(input_file, *this, scenario_name, this);
				game_instance->describe_scene(game_instance->get_perspective_entity()->get_scene());
				/*
				
				game_instance->get_perspective_entity()->get_scene()->call_function(game_instance, "describe");
				auto ents = game_instance->get_perspective_entity()->get_scene()->get_entities_in_scene();
				for (auto i = ents.begin(); i != ents.end(); ++i)
				{
					entity* d = *i;
					if (d != game_instance->get_perspective_entity())
					{
						d->call_function(game_instance, "describe");
					}
				}
				*/
			}
			catch (const std::exception& E)
			{
				println("Error: save file corrupt");
			}
		}
	}
	else
	{
		println("Error: failed to open " + save_file_name);
		game_instance->destroy();
		return nullptr;
	}

	return game_instance;
}

void engine::main_menu()
{
	//string_utils string_utils;
	bool game_going = true;
	std::string input;
	std::string raw;
	while (game_going)
	{
		if (correct_menu_display_bug_flag)
		{
			println();
		}
		else
		{
			correct_menu_display_bug_flag = !correct_menu_display_bug_flag;
		}
		if (DEV_MODE)
		{
			print_lines(
				"1. Open Scenario",
				"2. Bake Scenario",
				"3. Exit"
			);
		}
		else
		{
			if (INCLUDE_BAKED_SCENARIOS && baked_scenarios_registry.size() >=1)
			{
				if (baked_scenarios_registry.size() == 1 && FORCE_BAKED_SCENARIOS)
				{
					print_lines(
						"1. Begin",
						"2. Exit"
					);
				}
				else
				{
					print_lines(
						"1. Open Scenario",
						"2. Exit"
					);
				}
			}
			else
			{
				print_lines(
					"1. Open Scenario",
					"2. Exit"
				);
			}
		}
		
		string_utils string_utils;
		input = get_input();
		string_utils.make_lowercase(input);
		string_utils.strip(input);
		input = extra_text_processing(input, nullptr);
		
		/*
		string_utils.replace_all(input, "  ", " ", false);
		input = string_utils.replace_all(input, ".", "", false);
		string_utils.make_lowercase(input);
		input = string_utils.replace_all(input, "i'd like you to", "", false);
		input = string_utils.replace_all(input, "i'd love you to", "", false);
		input = string_utils.replace_all(input, "i'd like for you to", "", false);
		input = string_utils.replace_all(input, "i'd love for you to", "", false);
		input = string_utils.replace_all(input, "if you will", "", false);
		input = string_utils.replace_all(input, "can you", "", false);
		string_utils.strip(input);
		*/

		if (DEV_MODE)
		{
			if (input == "1" || input == "open" || input == "open scenario" || input == "scenario" || input == "o" || input == "s" || input == "os" || input == "one" || input == "open a scenario" || input == "open the scenario" || input == "1 1")
			{
				open_scenario(input);
			}
			else if (input == "2" || input == "bake" || input == "compile" || input == "prebake" || input == "2 1" || input == "pre-bake" || input == "pre-bake scenario" || input == "prebake scenario" || input == "compile scenario" || input == "bake scenario")
			{
				bake_scenarios();
			}
			else if (input == "3" || input == "exit" || input == "e" || input == "close" || input == "terminate" || input == "two" || input == "end" || input == "quit" || input == "3 1" || input == "3. exit" || input == "3 exit")
			{
				game_going = false;
			}
			else
			{
				clear_screen();
				println("That's not an option");
			}
		}
		else
		{
			if (input == "1" || input == "open" || input == "open scenario" || input == "scenario" || input == "o" || input == "s" || input == "os" || input == "one" || input == "open a scenario" || input == "open the scenario" || input == "1 1")
			{
				open_scenario(input);
			}
			else if (input == "2" || input == "exit" || input == "e" || input == "close" || input == "terminate" || input == "two" || input == "end" || input == "quit" || input == "2 1" || input == "2. exit" || input == "2 exit")
			{
				game_going = false;
			}
			else
			{
				clear_screen();
				println("That's not an option");
			}
		}
	}
}

void engine::start_new_game(const std::string& scenario_name)
{
	clear_screen();
	//println("Starting new game ",scenario_name);
	string_utils string_utils;
	std::string settings_directory = get_scenario_directory(scenario_name) + "settings.dat";
	res_file settings;

	std::vector<std::string> allowed_starter_characters;
	std::vector<std::string> starter_character_aliases;
	bool allow_custom_character = false;
	bool clear_on_scene_change = false;
	bool save_any_time = true;
	bool input_substitution_override = false;
	std::vector<std::string> wildcards;

	bool read_settings = settings.read_raw(settings_directory);
	
	if (read_settings)
	{
		//READ SETTINGS FILE
		for (res_file::line_num i = 0; i < settings.lines(); ++i)
		{
			const std::string& line = string_utils.get_lowercase(settings.get_line(i));

			auto print_settings_bool_error = [&]()
			{
				println("Error on ", settings_directory, " line ", i + 1, ": invalid value for setting, defaulting to false.");
			};
			allow_custom_character = false;
			if (string_utils.matches_command("allow_custom_character : $bool", line, wildcards, ": ", false) && false) //dummied out for now. Custom characters must be implemented per-scenario
			{
				std::string& val = wildcards[0];
				if (val == "yes" || val == "true" || val == "y" || val == "t")
					allow_custom_character = true;
				else if (val == "no" || val == "false" || val == "f" || val == "n")
					allow_custom_character = false;
				else
					print_settings_bool_error();
			}
			else if (string_utils.matches_command("save_any_time : $bool", line, wildcards, ": ", false)) //dummied out for now. Custom characters must be implemented per-scenario
			{
				std::string& val = wildcards[0];
				if (val == "yes" || val == "true" || val == "y" || val == "t")
					save_any_time = true;
				else if (val == "no" || val == "false" || val == "f" || val == "n")
					save_any_time = false;
				else
					print_settings_bool_error();
			}
			else if (string_utils.matches_command("clear_on_scene_change : $bool", line, wildcards, ": ", false)) //dummied out for now. Custom characters must be implemented per-scenario
			{
				std::string& val = wildcards[0];
				if (val == "yes" || val == "true" || val == "y" || val == "t")
					clear_on_scene_change = true;
				else if (val == "no" || val == "false" || val == "f" || val == "n")
					clear_on_scene_change = false;
				else
					print_settings_bool_error();
			}
			else if (string_utils.matches_command("input_substitution_override : $bool", line, wildcards, ": ", false)) //dummied out for now. Custom characters must be implemented per-scenario
			{
				std::string& val = wildcards[0];
				if (val == "yes" || val == "true" || val == "y" || val == "t")
					input_substitution_override = true;
				else if (val == "no" || val == "false" || val == "f" || val == "n")
					input_substitution_override = false;
				else
					print_settings_bool_error();
			}
			else if (string_utils.matches_command("allow_character : $name : $alias", line, wildcards, ": ", false))
			{
				std::string& char_name = wildcards[0];
				std::string& char_alias = wildcards[1];
				allowed_starter_characters.push_back(char_name);
				starter_character_aliases.push_back(string_utils.format_as_name(char_alias));
			}
			else if (string_utils.matches_command("allow_character : $name", line, wildcards, ": ", false))
			{
				std::string& char_name = wildcards[0];
				allowed_starter_characters.push_back(char_name);
				starter_character_aliases.push_back(string_utils.format_as_name(char_name));
			}
		}

		//INITIALIZE GAME INSTANCE
		game* game_instance = new game(scenario_name, this);
		game_instance->set_clear_on_scene_change(clear_on_scene_change);
		game_instance->set_save_any_time(save_any_time);
		game_instance->set_input_substitution_override(input_substitution_override);
		//HANDLE CHARACTER SELECTION / CREATION
		entity* pc = nullptr;
		if (!allow_custom_character && allowed_starter_characters.size() == 0)
		{
			println("Error: Scenario does not permit player to start with any characters");
			return;
		}
		else if (!allow_custom_character)
		{
			if (allowed_starter_characters.size() == 1)
			{
				std::string starter_character_name = allowed_starter_characters[0];
				pc = game_instance->load_entity_from_file(starter_character_name, "engine.cpp",  true);
			}
			else
			{
				println("Choose character:");
				for (int i = 0; i < allowed_starter_characters.size(); ++i)
					println(i + 1, ". ", starter_character_aliases[i]);

				int choice = get_integer_input(1, allowed_starter_characters.size() + 1);

				std::string starter_character_name = allowed_starter_characters[choice - 1];

				println("Chose ", starter_character_aliases[choice - 1]);
				pc = game_instance->load_entity_from_file(starter_character_name, "engine.cpp", true);
			}
			
		}
		else if (allow_custom_character && allowed_starter_characters.size() != 0)
		{
			println("1. Create Character");
			for (int i = 0; i < allowed_starter_characters.size(); ++i)
				println(i + 2, ". ", starter_character_aliases[i]);
			int choice = get_integer_input(1, allowed_starter_characters.size() + 2);

			if (choice == 1)
			{
				/*TODO: CHARACTER CREATION; MUST FORBID TAKEN NAMES FROM BEING USED!*/
			}
			else
			{
				std::string starter_character_name = allowed_starter_characters[choice - 2];
				println("Chose ", starter_character_aliases[choice - 2]);
				pc = game_instance->load_entity_from_file(starter_character_name, "engine.cpp", true);
				if (pc == nullptr)
				{
					println("ERROR: Could not open ", starter_character_name, ".entity or associated file was not a character.");
				}
			}
		}
		else if (allow_custom_character && allowed_starter_characters.size() == 0)
		{
			/*TODO: CHARACTER CREATION; MUST FORBID TAKEN NAMES FROM BEING USED!*/
		}
		
		clear_screen();
		//Settings have been loaded. A character is chosen. It's time to begin.
		scene* starting_scene = game_instance->get_scene("start");

		if (pc)
		{
			game_instance->set_perspective_entity(pc);
			starting_scene->queue_transfer(pc);
			//pc->set_to_scene("start");
			start_game_instance(game_instance);
		}
		else
		{
			println("ERROR: COULD NOT LOAD PERSPECTIVE ENTITY - ABORTING.");
		}

	}
	else
	{
		println("Error: Could not open ", settings_directory);
	}
	
}

std::string engine::output_substitution(game* game_instance, const std::string& thesuarus, std::string sentence, int replacement_percent_chance)
{
	if (game_instance->get_substitution_wizard()->has_thesaurus(thesuarus, game_instance->get_scenario_directory()))
	{
		return game_instance->get_substitution_wizard()->output_substitution(thesuarus, sentence, replacement_percent_chance);
	}
	else if (subs.has_thesaurus(thesuarus, "gamedata"))
	{
		return subs.output_substitution(thesuarus, sentence, replacement_percent_chance);
	}
	else
	{
		return sentence;
	}
}

void engine::start_game_instance(game* game_instance)
{
	correct_menu_display_bug_flag = false;
	game_instance->game_loop(); //Run the game
	game_instance->destroy(); //After it finishes, clear the memory for it.
	clear_screen();
}

void engine::open_scenario(std::string& input)
{

	auto pick_scenario = [&](const std::string& scenario_name)
		{
			res_file save_files;
			bool found_saves = save_files.read_raw_external(get_saves_directory(scenario_name) + "files.dat");
			if (found_saves)
			{
				if (save_files.lines() == 0 || ((save_files.lines() == 1 || save_files.lines() == 2) && save_files.get_line(1) == ""))
				{
					start_new_game(input);
				}
				else
				{
					println("1. New Game");
					for (res_file::line_num i = 1; i < save_files.lines() && save_files.get_line(i).size() != 0; ++i)
						println(i + 1, ". ", save_files.get_line(i));

					int choice = get_integer_input(1, save_files.lines());
					if (choice == 1)
					{
						start_new_game(input);
					}
					else
					{
						game* loaded_game = load_game(input, save_files.get_line(static_cast<res_file::line_num>(choice - 1)));
						start_game_instance(loaded_game);
					}
				}
			}
			else
			{
				println("Error: Could not find scenario");
			}
		};

	if (forces_baked_scenario() && baked_scenarios_registry.size() == 1) //Player must choose a baked scenario and only one is available
	{
		pick_scenario(baked_scenarios_registry[0]);
	}
	else if (INCLUDE_BAKED_SCENARIOS && baked_scenarios_registry.size() > 1) //Player can choose a baked scenario and more than one is present
	{
		clear_screen();
		for (int i = 0; i < baked_scenarios_registry.size(); ++i)
		{
			println(i + 1, ". ", baked_scenarios_registry[i]);
		}
		if (forces_baked_scenario()) //If there is more than one pre-baked scenario and the player must choose from the list.
		{
			println();
			println("Which one?");
		}
		else //If there is more than one pre-baked scenario available but the player isn't forced to choose from them.
		{
			println("You can choose from this list, or load from an external folder.");
		}

		string_utils string_utils;

		input = get_input();
		string_utils.strip(input);

		bool matched_baked_scenario = false;
		for (int i = 0; i < baked_scenarios_registry.size(); ++i)
		{
			std::string num = std::to_string(i + 1);
			const std::string& name = baked_scenarios_registry[i];
			if (input == num || input == name || input == (num + " " + name) || input == (num + "." + name) || input == (num + ". " + name) || input == (num + ":" + name) || input == (num + ": " + name))
			{
				input = name;
				matched_baked_scenario = true;
				break;
			}
		}
		while (forces_baked_scenario() && !matched_baked_scenario)
		{
			println("That's not an option. Which one do you want from the list?");

			input = get_input();
			string_utils.strip(input);

			for (int i = 0; i < baked_scenarios_registry.size(); ++i)
			{
				std::string num = std::to_string(i + 1);
				const std::string& name = baked_scenarios_registry[i];
				if (input == num || input == name || input == (num + " " + name) || input == (num + "." + name) || input == (num + ". " + name) || input == (num + ":" + name) || input == (num + ": " + name))
				{
					input = name;
					matched_baked_scenario = true;
					break;
				}
			}
		}

		pick_scenario(input);
	}
	else //No baked scenarios available, can only pick from external folder scenarios.
	{
		println("What is the name of the scenario?");
		input = get_input();
		pick_scenario(input);
	}
	
}

inline void engine::bake_scenarios()
{
#if DEV_MODE
	try {
		std::filesystem::path dir = std::filesystem::current_path();
		inst_println("First ensure that every scenario to bake is present in \'Scenarios\' folder,","and that 'Scenarios' is present in the working directory the engine was called from.");
		inst_println("Current working directory: ", dir);
		inst_println("Enter anything once ready.");
		std::string input = get_input();
		std::filesystem::path scenarios_dir = dir.append("Scenarios");
		inst_println("Scenarios Direcory: ", scenarios_dir);
		if (!std::filesystem::exists(scenarios_dir))
			throw;
		
		string_utils string_utils;

		std::vector<std::string> internal_file_builder_cpp;

		internal_file_builder_cpp.push_back("#include \"engine/headers/procedure/internal_file_builder.h\"");
		internal_file_builder_cpp.push_back("#include \"engine/headers/engine.h\"");
		internal_file_builder_cpp.push_back("");
		internal_file_builder_cpp.push_back("//This file essentially hard-codes scenario data into the executable.");
		internal_file_builder_cpp.push_back("");
		internal_file_builder_cpp.push_back("void internal_file_builder::build_internal_files()");
		internal_file_builder_cpp.push_back("{");
		internal_file_builder_cpp.push_back("#if INCLUDE_BAKED_SCENARIOS");
		internal_file_builder_cpp.push_back("\tstd::vector<std::string>* ptr = nullptr;");

		auto bake_file_to_code = [&](const std::string & dir)
		{
				inst_println("Baking ", dir);
				std::vector<std::string> lines;
				std::ifstream reader;
				reader.open(dir);
				if (reader.is_open())
				{
					std::string line;
					while (reader.is_open() && reader.good() && !reader.eof())
					{
						std::getline(reader, line);
						std::string finished_line;
						finished_line.reserve(line.size());
						for (int i = 0; i < line.size(); ++i)
						{
							char lc = line[i];
							if (lc == '"' || lc == '\'')
							{
								finished_line += '\\';
							}
							finished_line += lc;
						}
						lines.push_back(finished_line);
					}
					reader.close();
				}
				else
				{
					throw;
				}

				internal_file_builder_cpp.push_back("\tfile_reader::initialize_internal_file(\"" + dir + "\", " + std::to_string(lines.size()) + ");");
				internal_file_builder_cpp.push_back("\tptr = file_reader::get_internal_file_lines(\"" + dir + "\");");
				for (size_t i = 0; i < lines.size(); ++i)
				{
					internal_file_builder_cpp.push_back("\tptr->push_back(\""+lines[i] + "\");");
				}
				internal_file_builder_cpp.push_back("");
		};

		bake_file_to_code("gamedata/packages/area_scene");
		bake_file_to_code("gamedata/packages/base_character");
		bake_file_to_code("gamedata/packages/base_entity");
		bake_file_to_code("gamedata/packages/base_item");

		bake_file_to_code("gamedata/casual_output_substitution.dat");
		bake_file_to_code("gamedata/formal_output_substitution.dat");
		bake_file_to_code("gamedata/generic_output_substitution.dat");
		bake_file_to_code("gamedata/input_substitution.dat");
		bake_file_to_code("gamedata/medieval_output_substitution.dat");
		bake_file_to_code("gamedata/technical_output_substitution.dat");

		auto bake_scenario = [&](const std::filesystem::path& scen)
			{
				std::string scenario_name = scen.stem().generic_string();
				std::filesystem::path entities_folder = scen;
				std::filesystem::path scenes_folder = scen;
				std::filesystem::path templates_folder = scen;
				scenes_folder.append("scenes");
				entities_folder.append("entities");
				templates_folder.append("templates");
				if (!std::filesystem::exists(scenes_folder))
					throw;
				if (!std::filesystem::exists(entities_folder))
					throw;
				if (!std::filesystem::exists(templates_folder))
					throw;

				std::string dir = "";

				internal_file_builder_cpp.push_back("\tengine::register_baked_scenario(\"" + scenario_name + "\");");

				for (auto& entry : std::filesystem::directory_iterator(scen))
				{
					std::filesystem::path scene = entry.path();
					std::string filename = scene.filename().generic_string();
					if (string_utils.ends_with(filename, ".dat"))
					{
						dir = "Scenarios/"  + scenario_name + "/" + filename;
						bake_file_to_code(dir);
					}
				}

				for (auto& entry : std::filesystem::directory_iterator(scenes_folder))
				{
					std::filesystem::path scene = entry.path();
					std::string filename = scene.filename().generic_string();
					if (string_utils.ends_with(filename, ".scene"))
					{
						dir = "Scenarios/" + scenario_name + "/scenes/" + filename;
						bake_file_to_code(dir);
					}
				}

				for (auto& entry : std::filesystem::directory_iterator(entities_folder))
				{
					std::filesystem::path entity = entry.path();
					std::string filename = entity.filename().generic_string();
					if (string_utils.ends_with(filename, ".entity"))
					{
						dir = "Scenarios/" + scenario_name + "/entities/" + filename;
						bake_file_to_code(dir);
					}
				}

				for (auto& entry : std::filesystem::directory_iterator(templates_folder))
				{
					std::filesystem::path template_file = entry.path();
					std::string filename = template_file.filename().generic_string();
					if (string_utils.ends_with(filename, ".template"))
					{
						dir = "Scenarios/" + scenario_name + "/templates/" + filename;
						bake_file_to_code(dir);
					}
				}

			};


		for (auto& entry : std::filesystem::directory_iterator(scenarios_dir))
		{

			inst_println("Found scenario: ", entry);

			bake_scenario(entry.path());
		}

		internal_file_builder_cpp.push_back("#endif");
		internal_file_builder_cpp.push_back("}");

		std::ofstream writer;
		writer.open("internal_file_builder.cpp");
		if (writer.is_open())
		{
			for (size_t i = 0; i < internal_file_builder_cpp.size(); ++i)
			{
				const std::string& line = internal_file_builder_cpp[i];
				writer << line << std::endl;
			}
			writer.close();
		}
		else
		{
			throw;
		}
	}
	catch (const std::exception& E)
	{
		std::cout << "Encountered Error: " << E.what() << std::endl;
	}
#endif
}
