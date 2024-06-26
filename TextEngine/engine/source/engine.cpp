#include "engine/headers/engine.h"
#include "engine/headers/instance/game_obj_save_registry.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/scene.h"
#include "engine/headers/instance/entity.h"
#include <mutex>
#include <iostream>
#include <fstream>

std::mutex engine::print_mutex;

entity* engine::AMBIGUOUS_CHARACTER() const
{
	return reinterpret_cast<entity*>(1); //This ugly hack assumes that I will never have access to or allocate a entity at memory address 1, so the value should be free to use like an enum to denote a return value in which more than one entity could be returned.
}

void engine::clear_screen()
{
	for (int i = 0; i < 100; ++i)
		std::cout << std::endl;
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
	save_registry = new game_obj_save_registry();
	std::ifstream substitution_file;
	substitution_file.open("gamedata/input_substitution.dat");
	string_utils string_utils;
	if (substitution_file.is_open())
	{
		std::string line;
		while (substitution_file.good() && !substitution_file.eof())
		{
			std::getline(substitution_file, line);
			string_utils.strip(line);
			if (string_utils.starts_with(line, "remove)", true) && !string_utils.contains(line, ":"))
			{
				if (line == "remove)")
				{
					println("ERROR: input_substitution.dat incorrectly formatted: ", line, "; no string specified for removal.");
					std::exit(1);
				}
				else
					input_removal.push_back(line.substr(7));
			}
			else if (string_utils.starts_with(line, "remove_raw)", true) && !string_utils.contains(line, ":"))
			{
				if (line == "remove_raw)")
				{
					println("ERROR: input_substitution.dat incorrectly formatted: ", line, "; no string specified for removal.");
					std::exit(1);
				}
				else
					raw_input_removal.push_back(line.substr(11));
			}
			else
			{
				auto tokens = string_utils.extract_tokens(line, ":");
				if (tokens.size() != 3)
				{
					println("ERROR: input_substitution.dat incorrectly formatted: ", line, "; found ", tokens.size(), " tokens, expected 3");
					std::exit(1);
				}
				else
				{
					input_substitution.insert_or_assign(tokens[2], tokens[0]);
				}
			}
		}
		substitution_file.close();
	}
	else
	{
		println("ERROR: Could not open input_substitution.dat");
		std::exit(1);
	}

	auto load_output_substitution_file = [&](const std::string& modifier)
	{

		std::string filename = "gamedata/" + modifier + "_output_substitution.dat";
		substitution_file.open(filename);
		if (substitution_file.is_open())
		{
			output_substitution_map.insert_or_assign(modifier, std::map<std::string, std::vector<std::string>>());

			auto add_word_substitution = [&](const std::string& replacable_word, const std::string& potential_replacement)
			{
				auto& map = output_substitution_map.at(modifier);
				if (map.find(replacable_word) == map.end())
				{
					std::vector<std::string> initializer;
					initializer.push_back(potential_replacement);
					map.insert_or_assign(replacable_word, initializer);
				}
				else
				{
					map.at(replacable_word).push_back(potential_replacement);
				}
			};

			std::string line;
			uint32_t line_num = 0;
			while (substitution_file.good() && !substitution_file.eof())
			{
				++line_num;
				std::getline(substitution_file, line);
				string_utils.strip(line);
				auto tokens = string_utils.extract_tokens(line, ":,");
				if (tokens.size() < 3)
				{
					println("ERROR: Line ", line_num, " in ", filename, " is malformed.");
					std::exit(1);
				}
				else
				{
					std::string& first_word = tokens[0];
					std::string& first_separator = tokens[1];

					//Sanity check
					for (size_t i = 0; i < tokens.size(); ++i)
					{
						std::string& token = tokens[i];
						if (token == ":" && i != 1)
						{
							println("ERROR: Line ", line_num, " in ", filename, " is malformed.");
							std::exit(1);
						}
					}

					if (first_separator == ":") //If a thesaurus file is defining a bunch of words one word can turn into
					{
						for (size_t i = 2; i < tokens.size(); ++i)
						{
							std::string& token = tokens[i];
							if (token != ",")
							{
								add_word_substitution(first_word, token);
							}
						}
					}
					else if (first_separator == ",") //If the line defines a bunch of words that can be mutually exchanged.
					{
						for (size_t i = 0; i < tokens.size(); ++i)
						{
							std::string& base_word = tokens[i];
							if (base_word != ",")
							{
								for (size_t j = 0; j < tokens.size(); ++j)
								{
									std::string& sub_word = tokens[j];
									if (sub_word != "," && base_word != sub_word)
									{
										add_word_substitution(base_word, sub_word);
									}
								}
							}
						}
					}
					else
					{
						println("ERROR: Line ", line_num, " in ", filename, " is malformed.");
						std::exit(1);
					}
				}
			}
			substitution_file.close();
		}
		else
		{
			println("ERROR: Could not open ",filename);
			std::exit(1);
		}
	};

	load_output_substitution_file("generic");
	load_output_substitution_file("casual");
	load_output_substitution_file("formal");
	load_output_substitution_file("technical");
	load_output_substitution_file("medieval");
}

std::string engine::extra_text_processing(const std::string& original_text) const
{
	string_utils string_utils;
	std::string modified = original_text;

	if (string_utils.starts_with(modified, "I ", false))
		modified = string_utils.replace_first(modified, "I ", "", false);

	//This handles all the basic input thesaurus stuff
	for (auto i = input_substitution.begin(); i != input_substitution.end(); ++i)
	{
		modified = string_utils.replace_all(modified, i->first, i->second, true, false);
	}

	for (auto i = raw_input_removal.begin(); i != raw_input_removal.end(); ++i)
	{
		modified = string_utils.replace_all(modified, *i, "", false, false);
	}

	for (auto i = input_removal.begin(); i != input_removal.end(); ++i)
	{
		modified = string_utils.replace_all(modified, *i, "", true, false);
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
	string_utils string_utils;
	if (raw == nullptr && case_preserved == nullptr && processed == nullptr) //For some reason I ask for nothing.
	{
		return;
	}
	else if (raw != nullptr && case_preserved == nullptr && processed == nullptr) //I want only the raw input
	{
		std::getline(std::cin, *raw);
	}
	else if (raw == nullptr && case_preserved != nullptr && processed == nullptr) //I want only the case preserved input
	{
		std::getline(std::cin, *case_preserved);
		string_utils.strip(*case_preserved);
		*case_preserved = string_utils.replace_all(*case_preserved, "~", "", false);
	}
	else if (raw == nullptr && case_preserved == nullptr && processed != nullptr) //I want only the fully processed input
	{
		std::getline(std::cin, *processed);
		string_utils.strip(*processed);
		string_utils.make_lowercase(*processed);
		*processed = string_utils.replace_all(*processed, "~", "", false);
	}
	else if (raw != nullptr && case_preserved == nullptr && processed != nullptr) //I want the raw and fully processed input
	{
		std::getline(std::cin, *raw);
		*processed = *raw;
		string_utils.strip(*processed);
		string_utils.make_lowercase(*processed);
		*processed = string_utils.replace_all(*processed, "~", "", false);
	}
	else if (raw != nullptr && case_preserved != nullptr && processed != nullptr) //I want all three intermediary stages
	{
		std::getline(std::cin, *raw);
		*case_preserved = *raw;
		string_utils.strip(*case_preserved);
		*case_preserved = string_utils.replace_all(*case_preserved, "~", "", false);
		*processed = *case_preserved;
		string_utils.make_lowercase(*processed);
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
		print_lines(
			"1. Open Scenario", 
			"2. Exit"
		);

		input = get_input();

		if (input == "1")
		{
			open_scenario(input);
		}
		else if (input == "2")
		{
			game_going = false;
		}
		else
		{
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

			if (string_utils.matches_command("allow_custom_character : $bool", line, wildcards, ": "))
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
			println("Choose character:");
			for (int i = 0; i < allowed_starter_characters.size(); ++i)
				println(i+1, ". ", starter_character_aliases[i]);

			int choice = get_integer_input(1, allowed_starter_characters.size() + 1);

			std::string starter_character_name = allowed_starter_characters[choice - 1];

			println("Chose ", starter_character_aliases[choice - 1]);
			pc = game_instance->load_entity_from_file(starter_character_name);
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

std::string engine::output_substitution(const std::string& thesuarus, std::string sentence, int replacement_percent_chance) const
{
	if (replacement_percent_chance < 1)
		return sentence;

	string_utils string_utils;
	std::string lowercase_sentence = string_utils.get_lowercase(sentence);
	const std::map<std::string, std::vector<std::string>>& map = output_substitution_map.at(thesuarus);
	for (auto source_word_iterator = map.begin(); source_word_iterator != map.end(); ++source_word_iterator)
	{
		std::string source_word = string_utils.get_lowercase(source_word_iterator->first);
		size_t offset = 0;
		size_t location = lowercase_sentence.find(source_word, offset);
		const std::vector<std::string>& synonyms = source_word_iterator->second;
		while (location != std::string::npos && offset < lowercase_sentence.size())
		{
			std::string new_word = source_word;
			int replacement_roll = 1 + (rand() % 100);
			if(replacement_roll <= replacement_percent_chance)
				new_word = synonyms[rand() % synonyms.size()];
			
			sentence = string_utils.replace_from_offset(sentence, source_word, new_word, location, false);
			offset = location + 1;
			location = lowercase_sentence.find(source_word, offset);
		}
	}

	return sentence;
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
		if (save_files.lines() == 0)
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