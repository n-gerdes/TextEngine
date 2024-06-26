#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include "engine/headers/MASTER.h"
#include "engine/headers/util/string_utils.h"

class game;
class game_obj_save_registry;
class entity;
class engine
{
private:
	game_obj_save_registry* save_registry;
	std::vector<std::string> raw_input_removal;
	std::vector<std::string> input_removal; //This is a list of possible inputs to remove from player inputs, reducing the number of permutations it has to be capable of processing.
	std::map <std::string, std::string> input_substitution; //The key is a word that can be replaced, the value is what replaces it.
	std::map<std::string, std::map<std::string, std::vector<std::string>>> output_substitution_map;
	//The first key is the substitution list (generic, casual, formal, old-fasioned, etc).
	//The second key is the word to substitute
	//The output is a vector containing words that it can possibly change it to
	static std::mutex print_mutex;
protected:

public:
	entity*					AMBIGUOUS_CHARACTER() const;
	void					clear_screen();
	std::string				correct_tokenizer_bug(const std::string original) const;
							~engine();
							engine();

	std::string				extra_text_processing(const std::string& original_text) const;

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

	inline void				print() const{}
	inline void				println() const{}
	inline void				print_lines() const{}

private:
	static void				swap_dummy_char(char& c)
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
	}
public:

	inline void				print(char c) const { print_mutex.lock(); swap_dummy_char(c); std::cout << c; print_mutex.unlock(); }

	inline void				println(char c) const { print_mutex.lock(); swap_dummy_char(c); std::cout << c << std::endl; print_mutex.unlock(); }

	inline void				print(std::string str) { print_mutex.lock(); for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_dummy_char(c); } string_utils string_utils; string_utils.strip(str); std::cout << str; print_mutex.unlock(); }

	inline void				println(std::string str) { print_mutex.lock(); for (size_t i = 0; i < str.size(); ++i) { char& c = str[i]; swap_dummy_char(c); } string_utils string_utils; string_utils.strip(str); std::cout << str << std::endl; print_mutex.unlock(); }

	inline void				print(const char str[]) { print_mutex.lock(); for (size_t i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_dummy_char(c); } std::cout << str; print_mutex.unlock(); }
	inline void				println(const char str[]) { print_mutex.lock(); for (size_t i = 0; str[i] != 0; ++i) { char& c = *const_cast<char*>(&(str[i])); swap_dummy_char(c); } std::cout << str << std::endl; print_mutex.unlock(); }


	template <typename T, typename... Args>
	inline void				print(const T& output, Args... args) const { print_mutex.lock(); std::cout << output; print_mutex.unlock(); print(args...); }

	template <typename T, typename... Args>
	inline void				println(const T& output, Args... args) const { print(output); print(args...); print_mutex.lock(); std::cout << std::endl; print_mutex.unlock(); }

	template <typename T, typename... Args>
	inline void				print_lines(const T& output, Args... args) const { println(output); print_lines(args...); }

	void					start_new_game(const std::string& scenario_name);

	std::string				output_substitution(const std::string& thesuarus, std::string sentence, int replacement_percent_chance) const;
	
private:
	void					start_game_instance(game* game_instance);
};
#endif