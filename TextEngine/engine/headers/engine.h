#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <chrono>
#include <mutex>
#include "engine/headers/MASTER.h"
#include "engine/headers/util/string_utils.h"
#include "engine/headers/procedure/substitution_wizard.h"

class game;
class game_obj_save_registry;
class entity;

enum class color : int
{
	DEFAULT = 0, BLACK = 1, RED = 2, GREEN = 3, YELLOW = 4, BLUE = 5, MAGENTA = 6, CYAN = 7, LIGHT_GRAY = 8, DARK_GRAY = 9, LIGHT_RED = 10, LIGHT_GREEN = 11,
	LIGHT_YELLOW = 12, LIGHT_BLUE = 13, LIGHT_MAGENTA = 14, LIGHT_CYAN = 15, WHITE = 16
};

class engine
{
private:
	game_obj_save_registry* save_registry;
	
	//The first key is the substitution list (generic, casual, formal, old-fasioned, etc).
	//The second key is the word to substitute
	//The output is a vector containing words that it can possibly change it to
	substitution_wizard subs;
	bool correct_menu_display_bug_flag = true; //Tracks whether or not this is a "fresh" engine instance., i.e., one where a scenario has not yet been loaded. Used to fix a bug in which exiting to the menu after loading a scenario causes it to print an extra newline for some reason.
	static std::mutex print_mutex;
	static std::vector<std::string> baked_scenarios_registry;

	const static std::string CONSOLE_DEFAULT;

	inline std::string get_formatting(color text, color background) const
	{
		const static std::vector<int> fg_colors = { 39,30,31,32,33,34,35,36,37,90,91,92,93,94,95,96,97 };
		const static std::vector<int> bg_colors = { 49,40,41,42,43,44,45,46,47,100,101,102,103,104,105,106,107 };

		std::string txt_color = std::to_string(fg_colors[*reinterpret_cast<int*>(&text)]);
		std::string bg_color = std::to_string(bg_colors[*reinterpret_cast<int*>(&background)]);

		return "\033[0;" + txt_color + ";" + bg_color + "m";
	}

	std::string highlight_formatting = "";
	std::string basic_formatting = "";

protected:

public:

	

	inline void set_default_colormode()
	{
		std::cout << CONSOLE_DEFAULT;

		highlight_formatting = get_formatting(color::CYAN, color::DEFAULT);
		basic_formatting = get_formatting(color::DEFAULT, color::DEFAULT);

		std::cout << basic_formatting;
	}

	inline void set_darkmode()
	{
		std::cout << CONSOLE_DEFAULT;

		highlight_formatting = get_formatting(color::CYAN, color::BLACK);
		basic_formatting = get_formatting(color::LIGHT_YELLOW, color::BLACK);

		std::cout << basic_formatting;
	}

	inline void set_lightmode()
	{
		std::cout << CONSOLE_DEFAULT;
		highlight_formatting = get_formatting(color::BLUE, color::WHITE);
		basic_formatting = get_formatting(color::BLACK, color::WHITE);
		std::cout << basic_formatting;
	}

	inline void set_text_color(color text_color) const
	{
		std::cout << get_formatting(text_color, color::DEFAULT);
	}

	static int				count_baked_scenarios() { return baked_scenarios_registry.size(); };
	static void				register_baked_scenario(const std::string& scenario_name);

	static bool				forces_baked_scenario() { return INCLUDE_BAKED_SCENARIOS && !DEV_MODE && baked_scenarios_registry.size() > 0; };

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
		else if (c == dummy_left_brace) { c = '{'; }
		else if (c == dummy_right_brace) { c = '}'; }
		else if (c == var_val_space_char) { c = ' '; }
		else if (c == dummy_bar) { c = '|'; }
		else if (c == dummy_ampersand) { c = '&'; }
		//else if (c == dummy_array_delimeter) { c = ','; }
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
		else if (c == '{') { c = dummy_left_brace; }
		else if (c == '}') { c = dummy_right_brace; }
		else if (c == '&') { c = dummy_ampersand; }
		else if (c == '|') { c = dummy_bar; }
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

	inline void				bake_scenarios();

	inline void				inst_print() const {}
	inline void				inst_println() const { print_mutex.lock(); std::cout << std::endl;  last_character_printed = '\n'; print_mutex.unlock(); }
	inline void				inst_print_lines() const {}

	inline void				inst_print(char c) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; } std::cout << c; last_character_printed = c; print_mutex.unlock(); }

	inline void				inst_println(char c) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; }  std::cout << c << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }

	inline void				inst_print(std::string str) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; } } string_utils string_utils; string_utils.strip(str); std::cout << str; if (str.size() > 0) { last_character_printed = str[str.size() - 1]; } print_mutex.unlock(); }

	inline void				inst_println(std::string str) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; } } string_utils string_utils; string_utils.strip(str); std::cout << str << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }

	inline void				inst_print(const char str[]) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } bool did_print = false; size_t i = 0;  for (i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; }  did_print = true; } std::cout << str; if (did_print) { --i; last_character_printed = str[i]; } print_mutex.unlock(); }
	inline void				inst_println(const char str[]) { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } bool did_print = false; size_t i = 0; for (i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; }  did_print = true; } std::cout << str << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }


	template <typename T, typename... Args>
	inline void				inst_print(const T& output, Args... args) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } std::cout << output; last_character_printed = ' '; print_mutex.unlock(); inst_print(args...); }

	template <typename T, typename... Args>
	inline void				inst_println(const T& output, Args... args) const { inst_print(output); inst_print(args...); print_mutex.lock(); std::cout << std::endl; last_character_printed = '\n'; print_mutex.unlock(); }

	template <typename T, typename... Args>
	inline void				inst_print_lines(const T& output, Args... args) const { inst_println(output); print_lines(args...); }


	inline void				print() const {}
	inline void				println() const 
	{ 
		print_mutex.lock(); 
		std::cout << std::endl; 
		std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS));  
		last_character_printed = '\n'; 
		set_text_color(color::DEFAULT);
		print_mutex.unlock(); 
	}
	inline void				print_lines() const {}

	inline void				print(char c) const { if (c == variable_value_header_char) { return; } if (c == var_val_space_char) { c = ' '; } print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; } std::cout << c; last_character_printed = c; std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS)); set_text_color(color::DEFAULT); print_mutex.unlock(); }

	inline void				println(char c) const { if (c == variable_value_header_char) { return; } if (c == var_val_space_char) { c = ' '; } print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } swap_from_dummy_char(c); if (c == dummy_array_delimeter) { c = ','; }  std::cout << c; std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS)); std::cout << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS)); last_character_printed = '\n'; set_text_color(color::DEFAULT); print_mutex.unlock(); }

	inline void				print(std::string str) 
	{ 
		print_mutex.lock(); 
		if (last_character_printed == '\n') 
		{ 
			std::cout << PRINT_PREAMBLE; 
		} 
		for (size_t i = 0; i < str.size(); ++i) 
		{ 
			char& c = str[i]; 
			swap_from_dummy_char(c); 
			if (c == dummy_array_delimeter) 
			{ 
				c = ','; 
			} 
		} 
		string_utils string_utils; 
		string_utils.strip(str);
		for (int i = 0; i < str.size(); ++i) { 
			char c = str[i];
			if (c == delay_marker_char)
			{
				std::string arg;
				size_t j;
				for (j = i + 1; j < str.size() && str[j] != delay_marker_char; ++j)
				{
					if(str[j]!=' ')
						arg += str[j];
				}
				i = j;
				long double delay_in_seconds = std::stod(arg);
				int delay_ms = delay_in_seconds * 1000.0L;
				//std::cout << std::endl << arg << " / " << delay_in_seconds << " / " << delay_ms << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
			}
			else if (c == set_print_color_header)
			{
				char cc = str[i + 1];
				if (cc == 'r')
				{
					set_text_color(color::RED);
				}
				else if (cc == 'g')
				{
					set_text_color(color::GREEN);
				}
				else if (cc == 'b')
				{
					set_text_color(color::BLUE);
				}
				else if (cc == 'R')
				{
					set_text_color(color::LIGHT_RED);
				}
				else if (cc == 'G')
				{
					set_text_color(color::LIGHT_GREEN);
				}
				else if (cc == 'B')
				{
					set_text_color(color::LIGHT_BLUE);
				}
				else if (cc == 'c')
				{
					set_text_color(color::CYAN);
				}
				else if (cc == 'C')
				{
					set_text_color(color::LIGHT_CYAN);
				}
				else if (cc == 'y')
				{
					set_text_color(color::YELLOW);
				}
				else if (cc == 'Y')
				{
					set_text_color(color::LIGHT_YELLOW);
				}
				else if (cc == 'm')
				{
					set_text_color(color::MAGENTA);
				}
				else if (cc == 'M')
				{
					set_text_color(color::LIGHT_MAGENTA);
				}
				else if (cc == 'Z')
				{
					set_text_color(color::LIGHT_GRAY);
				}
				else if (cc == 'z')
				{
					set_text_color(color::DARK_GRAY);
				}
				else if (cc == 'x')
				{
					set_text_color(color::DEFAULT);
				}
				else if (cc == 'w')
				{
					set_text_color(color::WHITE);
				}
				++i;
			}
			else if (c == variable_value_header_char)
			{
				continue;
			}
			else
			{
				std::cout << str[i];
				std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS));
			}
		} 
		if (str.size() > 0) 
		{ 
			last_character_printed = str[str.size() - 1]; 
		} print_mutex.unlock(); 
	}

	inline void				println(std::string str) 
	{
		print_mutex.lock();
		if (last_character_printed == '\n') 
		{ 
			std::cout << PRINT_PREAMBLE; 
		}
		string_utils string_utils;
		string_utils.strip(str);
		for (size_t i = 0; i < str.size(); ++i) 
		{
			char& c = str[i]; 
			if (c == delay_marker_char)
			{
				std::string arg;
				size_t j;
				for (j = i + 1; j < str.size() && str[j] != delay_marker_char; ++j)
				{
					if (str[j] != ' ')
						arg += str[j];
				}
				i = j;
				long double delay_in_seconds = std::stod(arg);
				int delay_ms = delay_in_seconds * 1000.0L;
				//std::cout << std::endl << arg << " / " << delay_in_seconds << " / " << delay_ms << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
			}
			else if (c == set_print_color_header)
			{
				char cc = str[i + 1];
				if (cc == 'r')
				{
					set_text_color(color::RED);
				}
				else if (cc == 'g')
				{
					set_text_color(color::GREEN);
				}
				else if (cc == 'b')
				{
					set_text_color(color::BLUE);
				}
				else if (cc == 'R')
				{
					set_text_color(color::LIGHT_RED);
				}
				else if (cc == 'G')
				{
					set_text_color(color::LIGHT_GREEN);
				}
				else if (cc == 'B')
				{
					set_text_color(color::LIGHT_BLUE);
				}
				else if (cc == 'c')
				{
					set_text_color(color::CYAN);
				}
				else if (cc == 'C')
				{
					set_text_color(color::LIGHT_CYAN);
				}
				else if (cc == 'y')
				{
					set_text_color(color::YELLOW);
				}
				else if (cc == 'Y')
				{
					set_text_color(color::LIGHT_YELLOW);
				}
				else if (cc == 'm')
				{
					set_text_color(color::MAGENTA);
				}
				else if (cc == 'M')
				{
					set_text_color(color::LIGHT_MAGENTA);
				}
				else if (cc == 'Z')
				{
					set_text_color(color::LIGHT_GRAY);
				}
				else if (cc == 'z')
				{
					set_text_color(color::DARK_GRAY);
				}
				else if (cc == 'x')
				{
					set_text_color(color::DEFAULT);
				}
				else if (cc == 'w')
				{
					set_text_color(color::WHITE);
				}
				++i;
			}
			else if (c == variable_value_header_char)
			{
				continue;
			}
			else
			{
				swap_from_dummy_char(c);
				if (c == dummy_array_delimeter) { c = ','; }
				std::cout << str[i];
				std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS));
			}
		}
		
		std::cout << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS));
			
		last_character_printed = '\n';
		set_text_color(color::DEFAULT);
		print_mutex.unlock();
	}

	inline void				print(const char str[]) 
	{ 
		print_mutex.lock(); 
		if (last_character_printed == '\n') 
		{ 
			std::cout << PRINT_PREAMBLE;
		} 
		bool did_print = false; 
		size_t i = 0;  
		for (i = 0; str[i] != 0; ++i) 
		{ 
			char& c = *const_cast<char*>(&(str[i])); 

			if (c == delay_marker_char)
			{
				std::string arg;
				size_t j;
				for (j = i + 1; str[j] != delay_marker_char && str[j] != 0; ++j)
				{
					if (str[j] != ' ')
						arg += str[j];
				}
				i = j;
				long double delay_in_seconds = std::stod(arg);
				int delay_ms = delay_in_seconds * 1000.0L;
				//std::cout << std::endl << arg << " / " << delay_in_seconds << " / " << delay_ms << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
			}
			else if (c == set_print_color_header)
			{
				char cc = str[i + 1];
				if (cc == 'r')
				{
					set_text_color(color::RED);
				}
				else if (cc == 'g')
				{
					set_text_color(color::GREEN);
				}
				else if (cc == 'b')
				{
					set_text_color(color::BLUE);
				}
				else if (cc == 'R')
				{
					set_text_color(color::LIGHT_RED);
				}
				else if (cc == 'G')
				{
					set_text_color(color::LIGHT_GREEN);
				}
				else if (cc == 'B')
				{
					set_text_color(color::LIGHT_BLUE);
				}
				else if (cc == 'c')
				{
					set_text_color(color::CYAN);
				}
				else if (cc == 'C')
				{
					set_text_color(color::LIGHT_CYAN);
				}
				else if (cc == 'y')
				{
					set_text_color(color::YELLOW);
				}
				else if (cc == 'Y')
				{
					set_text_color(color::LIGHT_YELLOW);
				}
				else if (cc == 'm')
				{
					set_text_color(color::MAGENTA);
				}
				else if (cc == 'M')
				{
					set_text_color(color::LIGHT_MAGENTA);
				}
				else if (cc == 'Z')
				{
					set_text_color(color::LIGHT_GRAY);
				}
				else if (cc == 'z')
				{
					set_text_color(color::DARK_GRAY);
				}
				else if (cc == 'x')
				{
					set_text_color(color::DEFAULT);
				}
				else if (cc == 'w')
				{
					set_text_color(color::WHITE);
				}
				++i;
			}
			else if (c == variable_value_header_char)
			{
				continue;
			}
			else
			{
				swap_from_dummy_char(c);
				if (c == dummy_array_delimeter) { c = ','; }
				did_print = true;
				std::cout << str[i];
				std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS));
			}
		}
		if (did_print) 
		{ 
			--i; 
			last_character_printed = str[i]; 
		}
		print_mutex.unlock(); 
	}

	inline void				println(const char str[]) 
	{ 
		print_mutex.lock(); 
		if (last_character_printed == '\n') 
		{ 
			std::cout << PRINT_PREAMBLE; 
		} 
		bool did_print = false;
		size_t i = 0; 
		for (i = 0; str[i] != 0; ++i) 
		{ 
			char& c = *const_cast<char*>(&(str[i])); 

			if (c == delay_marker_char)
			{
				std::string arg;
				size_t j;
				for (j = i + 1; str[j] != delay_marker_char && str[j]!=0; ++j)
				{
					if (str[j] != ' ')
						arg += str[j];
				}
				i = j;
				long double delay_in_seconds = std::stod(arg);
				int delay_ms = delay_in_seconds * 1000.0L;
				//std::cout << std::endl << arg << " / " << delay_in_seconds << " / " << delay_ms << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
			}
			else if (c == set_print_color_header)
			{
				char cc = str[i + 1];
				if (cc == 'r')
				{
					set_text_color(color::RED);
				}
				else if (cc == 'g')
				{
					set_text_color(color::GREEN);
				}
				else if (cc == 'b')
				{
					set_text_color(color::BLUE);
				}
				else if (cc == 'R')
				{
					set_text_color(color::LIGHT_RED);
				}
				else if (cc == 'G')
				{
					set_text_color(color::LIGHT_GREEN);
				}
				else if (cc == 'B')
				{
					set_text_color(color::LIGHT_BLUE);
				}
				else if (cc == 'c')
				{
					set_text_color(color::CYAN);
				}
				else if (cc == 'C')
				{
					set_text_color(color::LIGHT_CYAN);
				}
				else if (cc == 'y')
				{
					set_text_color(color::YELLOW);
				}
				else if (cc == 'Y')
				{
					set_text_color(color::LIGHT_YELLOW);
				}
				else if (cc == 'm')
				{
					set_text_color(color::MAGENTA);
				}
				else if (cc == 'M')
				{
					set_text_color(color::LIGHT_MAGENTA);
				}
				else if (cc == 'Z')
				{
					set_text_color(color::LIGHT_GRAY);
				}
				else if (cc == 'z')
				{
					set_text_color(color::DARK_GRAY);
				}
				else if (cc == 'x')
				{
					set_text_color(color::DEFAULT);
				}
				else if (cc == 'w')
				{
					set_text_color(color::WHITE);
				}
				++i;
			}
			else if (c == variable_value_header_char)
			{
				continue;
			}
			else
			{
				swap_from_dummy_char(c);
				if (c == dummy_array_delimeter) { c = ','; }
				did_print = true;
				std::cout << str[i];
				std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS));
			}
		} 
		std::cout << std::endl; 
		std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_BETWEEN_CHARACTERS)); 
		last_character_printed = '\n';
		set_text_color(color::DEFAULT);
		print_mutex.unlock(); 
	}


	template <typename T, typename... Args>
	inline void				print(const T& output, Args... args) const { print_mutex.lock(); if (last_character_printed == '\n') { std::cout << PRINT_PREAMBLE; } std::cout << output; last_character_printed = ' '; print_mutex.unlock(); print(args...); }

	template <typename T, typename... Args>
	inline void				println(const T& output, Args... args) const { print(output); print(args...); print_mutex.lock(); std::cout << std::endl; last_character_printed = '\n'; set_text_color(color::DEFAULT); print_mutex.unlock(); }

	template <typename T, typename... Args>
	inline void				print_lines(const T& output, Args... args) const { println(output); print_lines(args...); }

	void					start_new_game(const std::string& scenario_name);

	std::string				output_substitution(game* game_instance, const std::string& thesuarus, std::string sentence, int replacement_percent_chance);
	
private:
	void					start_game_instance(game* game_instance);
};
#endif