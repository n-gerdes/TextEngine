#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include "engine/headers/MASTER.h"
#include "engine/headers/util/string_utils.h"
#include "engine/headers/procedure/substitution_wizard.h"

class game;
class game_obj_save_registry;
class entity;
class engine
{
private:
	game_obj_save_registry* save_registry;
	
	//The first key is the substitution list (generic, casual, formal, old-fasioned, etc).
	//The second key is the word to substitute
	//The output is a vector containing words that it can possibly change it to
	substitution_wizard subs;
	static std::mutex print_mutex;
protected:

public:
	entity*					AMBIGUOUS_CHARACTER() const;
	inline void				clear_screen();
	inline void				clear_screen(bool print_extra_line);
	std::string				correct_tokenizer_bug(const std::string original) const;
							~engine();
							engine();

	std::string				extra_text_processing(const std::string& original_text, game* game_instance) const; //Handles some of the extra text processing of input text

	std::string				get_scenario_directory(const std::string& scenario_name) const;
	std::string				get_saves_directory(const std::string& scenario_name) const;
	void					get_input(std::string* raw, std::string* case_preserved, std::string* processed) const; /*Gets input from the user, then
																						   allows access to that input through
																						   various stages of (minimal) processing.*/
	std::string				get_input() const; /*Gets minimally processed input from the user(extra spaces removedand made fully lowercase).
							 Roughly equivalent to the other get_input, but doesn't provide access to intermediary stages
							 Also strips out '~' entities.
							 */
	int						get_integer_input() const; //Get an integer from the player
	int						get_integer_input(int min, int max) const; //Get an integer from the player within a specific range.
	game_obj_save_registry* get_game_obj_save_registry();

	game*					load_game(const std::string& scenario_name, const std::string& save_file);

	void					main_menu();

	void					open_scenario(std::string& input);

	inline void				print() const{ }
	inline void				println() const { print_mutex.lock(); std::cout << std::endl;  last_character_printed = '\n'; print_mutex.unlock(); }
	inline void				print_lines() const{}

private:
public:
	static void				swap_from_dummy_char(char& c)
	{
		if (c == dummy_comma) { c = ','; }
		else if (c == dummy_left_paran) { c = '('; }
		else if (c == dummy_right_paran) { c = ')'; }
		else if (c == dummy_equals) { c = '='; }
		else if (c == dummy_slash) { c = '/'; }
		else if (c == dummy_plus) { c = '+'; }
		else if (c == dummy_star) { c = '*'; }
		else if (c == dummy_dash) { c = '-'; }
		else if (c == dummy_percent) { c = '%'; }
		else if (c == dummy_space) { c = ' '; }
		else if (c == dummy_colon) { c = ':'; }
		else if (c == dummy_exclamation) { c = '!'; }
		else if (c == dummy_period) { c = '.'; }
	}

	static void				swap_to_dummy_char(char& c)
	{
		if (c == ',') { c = dummy_comma; }
		else if (c == '(') { c = dummy_left_paran; }
		else if (c == ')') { c = dummy_right_paran; }
		else if (c == '=') { c = dummy_equals; }
		else if (c == '/') { c = dummy_slash; }
		else if (c == '+') { c = dummy_plus; }
		else if (c == '*') { c = dummy_star; }
		else if (c == '-') { c = dummy_dash; }
		else if (c == '%') { c = dummy_percent; }
		else if (c == ' ') { c = dummy_space; }
		else if (c == ':') { c = dummy_colon; }
		else if (c == '!') { c = dummy_exclamation; }
		else if (c == '.') { c = dummy_period; }
	}

private:

	static char last_character_printed;

	/*
	inline void				print_no_preamble(char c) const { print_mutex.lock(); swap_from_dummy_char(c); std::cout << c; print_mutex.unlock(); }
	inline void				print_no_preamble(std::string str) { print_mutex.lock(); for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_from_dummy_char(c); } string_utils string_utils; string_utils.strip(str); std::cout <<  str; print_mutex.unlock(); }
	inline void				print_no_preamble(const char str[]) { print_mutex.lock();  for (size_t i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_from_dummy_char(c); } std::cout << str; print_mutex.unlock(); }
	
	inline void				print_no_preamble() {};

	template <typename T, typename... Args>
	inline void				print_no_preamble(const T& output, Args... args) const { print_mutex.lock(); std::cout << output; print_mutex.unlock(); print_no_preamble(args...); }
	*/
public:

	inline void				print(char c) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } swap_from_dummy_char(c); std::cout << c; last_character_printed = c; print_mutex.unlock(); }

	inline void				println(char c) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } swap_from_dummy_char(c); std::cout << c << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }

	inline void				print(std::string str) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_from_dummy_char(c); } string_utils string_utils; string_utils.strip(str); std::cout << str; if (str.size() > 0) { last_character_printed = str[str.size() - 1]; } print_mutex.unlock(); }

	inline void				println(std::string str) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_from_dummy_char(c); } string_utils string_utils; string_utils.strip(str); std::cout << str << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }

	inline void				print(const char str[]) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } bool did_print = false; size_t i = 0;  for (i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_from_dummy_char(c); did_print = true; } std::cout << str; if (did_print) { --i; last_character_printed = str[i]; } print_mutex.unlock(); }
	inline void				println(const char str[]) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } bool did_print = false; size_t i = 0; for (i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_from_dummy_char(c); did_print = true; } std::cout << str << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }


	template <typename T, typename... Args>
	inline void				print(const T& output, Args... args) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } std::cout << output; last_character_printed = ' '; print_mutex.unlock(); print(args...); }

	template <typename T, typename... Args>
	inline void				println(const T& output, Args... args) const { print(output); print(args...); print_mutex.lock(); std::cout << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }

	template <typename T, typename... Args>
	inline void				print_lines(const T& output, Args... args) const { println(output); print_lines(args...); }

	void					start_new_game(const std::string& scenario_name);

	std::string				output_substitution(game* game_instance, const std::string& thesuarus, std::string sentence, int replacement_percent_chance);
	
private:
	void					start_game_instance(game* game_instance);
};
#endif