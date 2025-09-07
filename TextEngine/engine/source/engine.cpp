#include "engine/headers/engine.h"
#include "engine/headers/instance/game_obj_save_registry.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/scene.h"
#include "engine/headers/instance/entity.h"
#include <mutex>
#include <iostream>
#include <fstream>

std::mutex engine::print_mutex;
char engine::last_character_printed = '\n';

entity* engine::AMBIGUOUS_CHARACTER() const
{
	return reinterpret_cast<entity*>(1); //This ugly hack assumes that I will never have access to or allocate a entity at memory address 1, so the value should be free to use like an enum to denote a return value in which more than one entity could be returned.
}

void engine::clear_screen()
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
	last_character_printed = '\n';
	save_registry = new game_obj_save_registry();
	
	subs.load_input_substitution("gamedata/input_substitution.dat");

	subs.load_output_substitution_file("gamedata/", "generic");
	subs.load_output_substitution_file("gamedata/", "casual");
	subs.load_output_substitution_file("gamedata/", "formal");
	subs.load_output_substitution_file("gamedata/", "technical");
	subs.load_output_substitution_file("gamedata/", "medieval");
}

std::string engine::extra_text_processing(const std::string& original_text, game* game_instance) const
{
	string_utils string_utils;
	std::string modified = original_text;

	if (string_utils.starts_with(modified, "I ", false))
		modified = string_utils.replace_first(modified, "I ", "", false);

	//This handles all the basic input thesaurus stuff
	subs.apply_input_substitution(modified);

	if (game_instance != nullptr)
	{
		game_instance->get_substitution_wizard()->apply_input_substitution(modified);
	}

	for (int i = 0; i < modified.size(); ++i)
	{
		swap_from_dummy_char(modified[i]);
	}
	return modified;
}

engine::~engine()
{
	save_registry->cleanup_registry();
	delete save_registry;
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
				game_instance->load_from_file(input_file, *this, scenario_name, this);
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
		println();
		print_lines(
			"1. Open Scenario", 
			"2. Exit"
		);
		string_utils string_utils;
		input = get_input();
		string_utils.make_lowercase(input);
		string_utils.strip(input);
		

		if (input == "1" || input=="open" || input == "open scenario" || input == "scenario" || input == "o" || input == "s" || input == "os" || input == "one")
		{
			open_scenario(input);
		}
		else if (input == "2" || input == "exit" || input == "e" || input == "close" || input == "terminate" || input == "two"|| input == "end" || input == "quit")
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

			if (string_utils.matches_command("allow_custom_character : $bool", line, wildcards, ": ") && false) //dummied out for now. Custom characters must be implemented per-scenario
			{
				std::string& val = wildcards[0];
				if (val == "yes" || val == "true")
					allow_custom_character = true;
				else if (val == "no" || val == "false")
					allow_custom_character = false;
				else
					print_settings_bool_error();
			}
			else if (string_utils.matches_command("allow_character : $name : $alias", line, wildcards, ": "))
			{
				std::string& char_name = wildcards[0];
				std::string& char_alias = wildcards[1];
				allowed_starter_characters.push_back(char_name);
				starter_character_aliases.push_back(string_utils.format_as_name(char_alias));
			}
			else if (string_utils.matches_command("allow_character : $name", line, wildcards, ": "))
			{
				std::string& char_name = wildcards[0];
				allowed_starter_characters.push_back(char_name);
				starter_character_aliases.push_back(string_utils.format_as_name(char_name));
			}
		}

		//INITIALIZE GAME INSTANCE
		game* game_instance = new game(scenario_name, this);

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
				pc = game_instance->load_entity_from_file(starter_character_name);
			}
			else
			{
				println("Choose character:");
				for (int i = 0; i < allowed_starter_characters.size(); ++i)
					println(i + 1, ". ", starter_character_aliases[i]);

				int choice = get_integer_input(1, allowed_starter_characters.size() + 1);

				std::string starter_character_name = allowed_starter_characters[choice - 1];

				println("Chose ", starter_character_aliases[choice - 1]);
				pc = game_instance->load_entity_from_file(starter_character_name);
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
				pc = game_instance->load_entity_from_file(starter_character_name);
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
	game_instance->game_loop(); //Run the game
	game_instance->destroy(); //After it finishes, clear the memory for it.
	clear_screen();
}

void engine::open_scenario(std::string& input)
{
	println("What is the name of the scenario?");
	input = get_input();

	res_file save_files;
	bool found_saves = save_files.read_raw(get_saves_directory(input) + "files.dat");
	if (found_saves)
	{
		if (save_files.lines() == 0 || ((save_files.lines()==1 || save_files.lines()==2) && save_files.get_line(1)==""))
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
}