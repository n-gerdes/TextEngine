#ifndef GAME_RES_FILE_H
#define GAME_RES_FILE_H
#include <fstream>
#include <string>
#include <vector>
#include <map>

/*
This class offers the framework for loading text files and processing them as scripts. It forms the foundation of the scripting
system.
*/
class game;
class engine;
class res_file
{
public:
	typedef int64_t line_num;
private:
	std::map<std::string, line_num> function_line_nums;
	std::vector<line_num> command_func_lines;
	std::vector<std::string> innate_functions;
	std::vector<std::string> scripted_functions;
	std::vector<std::string> line_data;
	std::vector<int> line_commands;
	std::string filename;
	bool finished_loading = false;
	typedef void(*command_caller)(game*, res_file&, std::vector<uint32_t>&, res_file::line_num&, const std::string&, int&, int&, 
		std::vector<std::string>&, std::vector<std::string>&, std::vector<std::string>&, std::string&, bool&, std::string&);
	struct execution_registry_entry
	{
		std::string delimeters = " ().,:;+*/-=";
		std::string pattern;
		command_caller func;
		bool control_command = false;
		execution_registry_entry(const std::string& pattern, command_caller func, bool control_command) : pattern(pattern), func(func), control_command(control_command) {};
		execution_registry_entry(const std::string& pattern, command_caller func, bool control_command, const std::string& delimeters) : pattern(pattern), func(func), control_command(control_command), delimeters(delimeters) {};
	};

	static std::vector<execution_registry_entry> execution_registry;

private:
	bool						add_lines_from_file(const engine* engine, const std::string& scenario_name, const std::string& name, game* game_instance);

	void						check_line_match(const std::string& line, line_num line_num);
public:

	const static line_num NO_MATCH = -1;

	//Calls a function in the file's script, and returns an error message if it encounters one.
	std::string					call_function(game* game_instance, const std::string& function_name);
	std::string					call_function(game* game_instance, const std::string& function_name, std::vector<std::string>& arg_values, std::string& return_value);
	std::string					call_function(game* game_instance, const std::string& function_name, std::initializer_list<std::string> arg_values, std::string& return_value);
	virtual std::string			call_innate_function(game* game_instance, const std::string& function_name, std::vector<std::string>& arg_values);
public:
	bool						evaluate_condition(game* game_instance, const std::string& condition, std::string& err_msg, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values);
private:
	void						execute_line(game* game_instance, line_num& line, std::string& err_msg, int& look_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values, bool& early_return, std::string& return_value, std::vector<uint32_t>& if_conditions);
	//Executes the specified line of code & increments the line count, and sets the err_msg if necessary.
	virtual void				execute_line_from_class(game* game_instance, line_num& line, std::string& err_msg, std::vector<std::string>& wildcards, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values); /*
																		Allows subclasses to parse and execute lines themselves.	
																											*/
public:
	std::vector<std::string>	extract_args_from_token(std::string complete_args_token, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance);

public:
	line_num					find_exact(line_num starting_line, const std::string& line);
	line_num					find_match(line_num starting_line, const std::string& command);
	line_num					find_match(line_num starting_line, const std::string& command, std::vector<std::string>& wildcards);
private:
	line_num					find_match_remove_wildcards_from_function_name(line_num starting_line, std::string function_name, const std::string& args_string, std::vector<std::string>& wildcards);
public:
	line_num					find_match(line_num starting_line, const std::string& command, std::vector<std::string>& wildcards, const std::string& delimeters);
	const std::string&			get_line(line_num line_number) const; //Lines start at 0
	const std::string&			get_filename() const;
	const std::vector<line_num>& get_command_func_lines() const;
	line_num					lines() const;
	virtual void				process_line_from_file(const std::string& line);
	bool						read(const engine* engine, const std::string& scenario_name, const std::string& filename, game* game_instance);
	//bool						read(const std::string& scenario_name, const std::string& filename);
	bool						read_raw(const std::string& filename);
	bool						read_raw_external(const std::string& filename);
protected:
	void						register_innate_function(const std::string& innate_function_name);
public:
	std::string					resolve_expression(std::string raw_value, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance, bool re_place_quotes);
	std::string					resolve_expression(std::string raw_value, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance);
	virtual std::string			resolve_expression_custom(std::string raw_value, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance) const;
public:
	std::string					substitute_variables(const std::string& original, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance);
};

#endif