#include "engine/headers/instance/res_file.h"
#include "engine/headers/util/string_utils.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/instance/scene.h"
#include "engine/headers/MASTER.h"
#include <deque>

//This class is for reading/loading/executing resource/script files. It forms the basis for both entities and scenes

inline void increment_execution_layer(int& execution_layer)
{
	++execution_layer;
}

inline void increment_line_layer(int& line_layer)
{
	++line_layer;
}

inline void decrement_execution_layer(int& execution_layer)
{
	--execution_layer;
}

inline void decrement_line_layer(int& line_layer)
{
	--line_layer;
}

void template_command_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num, 
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values, 
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	
}

void if_then_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num, const std::string& code, 
	int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values, 
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	std::string& condition = wildcards[0];
	if ((line_layer == execution_layer))
	{
		bool evaluation = script.evaluate_condition(game_instance, condition, err_msg, variable_names, variable_values);
		if (evaluation)
			increment_execution_layer(execution_layer);

		if_conditions.push_back(evaluation);
	}
	increment_line_layer(line_layer);
}

void while_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num, const std::string& code, 
	int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values, 
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	std::string& condition = wildcards[0];
	if ((line_layer == execution_layer))
	{
		bool evaluation = script.evaluate_condition(game_instance, condition, err_msg, variable_names, variable_values);
		if (evaluation)
			increment_execution_layer(execution_layer);

		if_conditions.push_back(line_num);
	}
	increment_line_layer(line_layer);
}

void end_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line, const std::string& code, 
	int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values, 
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	//std::cout << "Reached 'end' on line " << line << " in file " << script.get_filename() << std::endl;
	//matched_control_statement = true;
	//matched_command_id = 3;
	if (line_layer == 0)
	{
		--line; // Needs to decrement before returning because then it will re-increment to the same spot after this call; if the last line of a code file is an 'end' statement, it would basically try to go past the end and therfore returns an error even though none is encountered
		early_return = true;
		return;
	}

	if (execution_layer == line_layer)
	{
		//std::cout << "Popping back condition " << if_conditions[if_conditions.size() - 1] << " on line " << line + 1 << std::endl;
		uint32_t last_condition = if_conditions[if_conditions.size() - 1];
		if_conditions.pop_back();
		decrement_execution_layer(execution_layer);
		if (last_condition > 1) //The condition is only greater than 1 when it was set by a while loop and not an if-then; thus, it becomes a reference to the line this 'end' should return to when reached.
		{
			line = last_condition - 1; //I return to the reference minus one because line will be incremented at the end.
		}
	}
	else if (execution_layer == (line_layer - 1))
	{
		//std::cout << "Popping back condition " << if_conditions[if_conditions.size() - 1] << " on line " << line + 1 << std::endl;
		if_conditions.pop_back();
	}

	decrement_line_layer(line_layer);

	if (line_layer < execution_layer)
	{
		err_msg = "'end' has no matching beginning (function, command, or if statement)";
	}
}

void else_if_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	//matched_control_statement = true;
	std::string& condition = wildcards[0];
	bool evaluation = script.evaluate_condition(game_instance, condition, err_msg, variable_names, variable_values);
	if ((execution_layer == (line_layer - 1)) && evaluation && (!if_conditions[if_conditions.size() - 1]))
	{
		increment_execution_layer(execution_layer);
		if_conditions.pop_back();
		if_conditions.push_back(true);
	}
	else if (execution_layer == line_layer && if_conditions[if_conditions.size() - 1])
	{
		decrement_execution_layer(execution_layer);
	}
}

void else_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	//matched_command_id = 5;
	//matched_control_statement = true;
	bool last_condition = if_conditions[if_conditions.size() - 1];

	if (execution_layer == (line_layer - 1) && !last_condition) //This indicates that the previous if-then block did not execute
	{
		increment_execution_layer(execution_layer);
	}
	else if (execution_layer == line_layer && last_condition) //This indicates that the previous if-then block did execute
	{
		decrement_execution_layer(execution_layer);
	}
}

void println_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	std::string text = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//std::cout << "HERE = " << text << std::endl;
	game_instance->get_engine()->println(game_instance->get_engine()->correct_tokenizer_bug(text));
}

void print_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	std::string text = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//std::cout << "HERE = " << text << std::endl;
	game_instance->get_engine()->print(game_instance->get_engine()->correct_tokenizer_bug(text));
}

void set_arr_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	string_utils string_utils;
	//matched_first_chunk = true;
	//matched_command_id = 50;
	const std::string& var_name = wildcards[0];

	if (var_name.find("\"") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '\"'";
	}
	else if (var_name.find("'") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '''";
	}
	else if (var_name.find(" ") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ' '";
	}
	else if (var_name.find("=") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '='";
	}
	else if (var_name.find(",") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ','";
	}
	else if (var_name.find("(") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '('";
	}
	else if (var_name.find(")") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ')'";
	}
	else if (var_name.find("[") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '['";
	}
	else if (var_name.find("]") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ']'";
	}
	else if (var_name.find(".") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '.'";
	}
	else if (string_utils.is_numeric(var_name))
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Cannot assign value to number";
	}
	else
	{
		std::string rval;
		for (size_t i = 2; i < wildcards.size(); ++i)
			rval += wildcards[i];

		std::string var_val = script.resolve_expression(rval, variable_names, variable_values, game_instance);
		if (script.find_match(0, "function " + var_name + "( $args )") == res_file::NO_MATCH)
		{
			size_t existing_variable_index;
			for (existing_variable_index = 0; existing_variable_index < variable_names.size(); ++existing_variable_index)
			{
				if (variable_names[existing_variable_index] == var_name)
				{
					//std::cout << "?????????\n";
					break;
				}
			}
			if (var_name != var_val) //Assigning a variable to itself does nothing
			{

				std::string pair_delimeter_string = "0";
				pair_delimeter_string[0] = pair_delimeter_character;

				const std::string& index = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				//std::cout << "INDEX = " << std::endl;
				if (existing_variable_index == variable_names.size())
				{
					std::string total_value = "{" + index + pair_delimeter_string + var_val + "}";
					variable_names.push_back(var_name);
					variable_values.push_back(total_value);
				}
				else
				{
					//std::cout << var_name << std::endl;
					std::string new_total_value = "{";
					std::string current_value = variable_values[existing_variable_index];
					if (current_value == "{}" || current_value == "{ }")
					{
						new_total_value = "{" + index + pair_delimeter_string + var_val + "}";
						variable_values[existing_variable_index] = new_total_value;
						return;
					}
					else if (!(current_value.size() >= 2 && current_value[0] == '{' && current_value[current_value.size() - 1] == '}'))
					{
						err_msg = var_name + " does not hold an array";
						return;
					}
					current_value = current_value.substr(1);
					current_value.resize(current_value.size() - 1);
					const std::vector<std::string>& current_pairs = string_utils.extract_tokens(current_value, ",");
					size_t delimeters_found = 0;
					size_t i;
					bool did_add_var = false;
					for (i = 0; i < current_pairs.size(); ++i)
					{
						if (current_pairs[i] == ",")
						{
							++delimeters_found;
						}
						else
						{
							std::string current_index;
							std::string current_value;
							const std::vector<std::string>& pair = string_utils.extract_tokens(current_pairs[i], pair_delimeter_string);
							if (pair.size() == 1)
							{
								current_index = std::to_string(i - delimeters_found);
								current_value = pair[0];
							}
							else if (pair.size() == 3)
							{
								current_index = pair[0];
								current_value = pair[2];
							}
							else
							{
								std::cout << "A grave error has been encountered in " __FILE__ << " on line " << __LINE__ << std::endl;
							}
							if (current_index == index)
							{
								current_value = var_val;
								did_add_var = true;
							}

							new_total_value += current_index + pair_delimeter_string + current_value + ",";
						}
					}

					//If it got this far then it never found the index in the array.
					if (i == current_pairs.size() && !did_add_var)
					{
						new_total_value += index + pair_delimeter_string + var_val + "}";
					}
					new_total_value[new_total_value.size() - 1] = '}';
					variable_values[existing_variable_index] = new_total_value;
				}

			}
			else
			{
				if (existing_variable_index == variable_names.size()) //If the variable is being assigned to itself and it doesn't exist, this is an error because it cannot be initialized to itself.
				{
					err_msg = "Error: Variable \'" + var_name + "\' cannot be initialized to itself.";
				}
			}
		}
		else
		{
			err_msg = "Error: Variable \'" + var_name + "\' shares a name with a function";
		}
	}
}

void set_var_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_val)
{
	string_utils string_utils;
	//matched_first_chunk = true;
	std::string& var_name = wildcards[0];
	//matched_command_id = 51;
	if (var_name.find("\"") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '\"'";
	}
	else if (var_name.find("'") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '''";
	}
	else if (var_name.find(" ") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ' '";
	}
	else if (var_name.find("=") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '='";
	}
	else if (var_name.find(",") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ','";
	}
	else if (var_name.find("(") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '('";
	}
	else if (var_name.find(")") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ')'";
	}
	else if (var_name.find("[") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		//std::cout << "HERE = " << var_name << std::endl;
		err_msg = "Error: Variable names cannot contain '['";
	}
	else if (var_name.find("]") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain ']'";
	}
	else if (var_name.find(".") != std::string::npos)
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Variable names cannot contain '.'";
	}
	else if (string_utils.is_numeric(var_name))
	{
		//found_error_in_variable_name = true;
		err_msg = "Error: Cannot assign value to number";
	}
	else
	{
		std::string var_val = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
		//std::cout << "Setting " << var_name << " to " << var_val << std::endl;
		if (script.find_match(0, "function " + var_name + "( $args )") == res_file::NO_MATCH)
		{
			size_t existing_variable_index;
			for (existing_variable_index = 0; existing_variable_index < variable_names.size(); ++existing_variable_index)
			{
				if (variable_names[existing_variable_index] == var_name)
					break;
			}
			if (var_name != var_val) //Assigning a variable to itself does nothing
			{
				if (existing_variable_index == variable_names.size())
				{
					//std::cout << "INITIALIZING " << var_name << " TO " << var_val << std::endl;
					variable_names.push_back(var_name);
					variable_values.push_back(var_val);
				}
				else
				{
					//std::cout << "SETTING " << var_name << " TO " << var_val << std::endl;
					variable_values[existing_variable_index] = var_val;//script.resolve_expression(var_val, variable_names, variable_values, game_instance);//script.substitute_variables(var_val, variable_names, variable_values, game_instance);
				}
			}
			else
			{
				if (existing_variable_index == variable_names.size()) //If the variable is being assigned to itself and it doesn't exist, this is an error because it cannot be initialized to itself.
				{
					err_msg = "Error: Variable \'" + var_name + "\' cannot be initialized to itself.";
				}
			}
		}
		else
		{
			err_msg = "Error: Variable \'" + var_name + "\' shares a name with a function";
		}
	}
}


void return_val_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	return_value = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	early_return = true;
	//matched_command_id = 52;
}

void return_nothing_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	early_return = true;
	//matched_command_id = 53;
}

void throw_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	err_msg = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
}

void throw_empty_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	err_msg = "UNSPECIFIED SCRIPT ERROR";
}

void call_perspective_entity_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	//matched_command_id = 10;
	std::string& func_name = wildcards[0];
	std::vector<std::string> empty_args;
	entity* entity_reference = game_instance->get_perspective_entity();
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of perspective entity is ambiguous - which is a problem.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no perpsective entity - which is a problem.";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}


void call_perspective_entity_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	//matched_command_id = 11;
	std::string& func_name = wildcards[0];
	std::string& complete_args_token = wildcards[1];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	entity* entity_reference = game_instance->get_perspective_entity();
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of perspective entity is ambiguous - and that's a problem.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no perspective entity - and that's a problem.";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_entity_func_by_alias_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	//matched_command_id = 12;
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = game_instance->get_entity(entity_name, true);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_entity_func_by_alias_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	//matched_command_id = 13;
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	entity* entity_reference = game_instance->get_entity(entity_name, true);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_entity_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	//matched_command_id = 14;
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = game_instance->get_entity_by_name(entity_name);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_entity_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//err_msg = "";
	//matched_command_id = 15;
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	entity* entity_reference = game_instance->get_entity_by_name(entity_name);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		//std::cout << "HERE\n";
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_first_entity_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//matched_command_id = 16;
	//err_msg = "";
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = game_instance->get_first_entity(entity_name);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_first_entity_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//matched_command_id = 17;
	//err_msg = "";
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	entity* entity_reference = game_instance->get_first_entity(entity_name);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_any_entity_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//matched_command_id = 18;
	//err_msg = "";
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//substitute_variables(wildcards[0], variable_names, variable_values);
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = game_instance->get_any_entity(entity_name);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_any_entity_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//matched_command_id = 19;
	//err_msg = "";
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	entity* entity_reference = game_instance->get_any_entity(entity_name);
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_entity_here_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//matched_command_id = 20;
	//err_msg = "";
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);

	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_scene->get_name(), false);
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_entity->get_scene_name(), false);
	}
	else
	{
		entity_reference = nullptr;
	}
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_entity_here_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	//matched_command_id = 21;
	//err_msg = "";
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);

	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_scene->get_name(), false);
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_entity->get_scene_name(), false);
	}
	else
	{
		entity_reference = nullptr;
	}

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_first_entity_here_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//matched_command_id = 22;
	//err_msg = "";
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_first_entity_in_scene(entity_name, this_scene->get_name());
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_first_entity_in_scene(entity_name, this_entity->get_scene_name());
	}
	else
	{
		entity_reference = nullptr;
	}
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_first_entity_here_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	//err_msg = "";
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	//matched_command_id = 23;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_first_entity_in_scene(entity_name, this_scene->get_name());
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_first_entity_in_scene(entity_name, this_entity->get_scene_name());
	}
	else
	{
		entity_reference = nullptr;
	}

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_any_entity_here_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//matched_command_id = 24;
	//err_msg = "";
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_any_entity_in_scene(entity_name, this_scene->get_name());
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_any_entity_in_scene(entity_name, this_entity->get_scene_name());
	}
	else
	{
		entity_reference = nullptr;
	}
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_any_entity_here_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	//matched_command_id = 25;
	//err_msg = "";
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_any_entity_in_scene(entity_name, this_scene->get_name());
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_any_entity_in_scene(entity_name, this_entity->get_scene_name());
	}
	else
	{
		entity_reference = nullptr;
	}

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_any_entity_here_by_alias_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//matched_command_id = 26;
	//err_msg = "";
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_scene->get_name(), true);
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_entity->get_scene_name(), true);
	}
	else
	{
		entity_reference = nullptr;
	}
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_any_entity_here_by_alias_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	//matched_command_id = 27;
	//err_msg = "";
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_scene->get_name(), true);
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_entity->get_scene_name(), true);
	}
	else
	{
		entity_reference = nullptr;
	}

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_entity_by_alias_here_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	//matched_command_id = 28;
	//err_msg = "";
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_scene->get_name(), true);
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_entity->get_scene_name(), true);
	}
	else
	{
		entity_reference = nullptr;
	}
	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_entity_by_alias_here_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string entity_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	//err_msg = "";
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	//matched_command_id = 29;
	entity* entity_reference = nullptr;
	entity* this_entity = dynamic_cast<entity*>(&script);
	scene* this_scene = dynamic_cast<scene*>(&script);
	if (this_scene)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_scene->get_name(), true);
	}
	else if (this_entity)
	{
		entity_reference = game_instance->get_entity_in_scene(entity_name, this_entity->get_scene_name(), true);
	}
	else
	{
		entity_reference = nullptr;
	}

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_entity_by_alias_in_scene_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 38;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	entity* entity_reference = game_instance->get_entity_in_scene(entity_name, scene_name, true);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_entity_by_alias_in_scene_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 39;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	std::string& complete_args_token = wildcards[3];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);

	entity* entity_reference = game_instance->get_entity_in_scene(entity_name, scene_name, true);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_entity_in_scene_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 40;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	entity* entity_reference = game_instance->get_entity_in_scene(entity_name, scene_name, false);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_entity_in_scene_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 41;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	std::string& complete_args_token = wildcards[3];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);

	entity* entity_reference = game_instance->get_entity_in_scene(entity_name, scene_name, false);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_first_entity_in_scene_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 42;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	entity* entity_reference = game_instance->get_first_entity_in_scene(entity_name, scene_name);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_first_entity_in_scene_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 43;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	std::string& complete_args_token = wildcards[3];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);

	entity* entity_reference = game_instance->get_first_entity_in_scene(entity_name, scene_name);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_any_entity_in_scene_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script.resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 44;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	entity* entity_reference = game_instance->get_any_entity_in_scene(entity_name, scene_name);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no \'" + entity_name + "\'";
	}
	else
	{
		entity_reference->call_function(game_instance, func_name);
	}
}

void call_any_entity_in_scene_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string entity_name = script. resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
	//matched_command_id = 45;
	//err_msg = "";
	std::string& func_name = wildcards[2];
	std::string& complete_args_token = wildcards[3];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);

	entity* entity_reference = game_instance->get_any_entity_in_scene(entity_name, scene_name);

	if (entity_reference == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
	{
		err_msg = "Identity of \'" + entity_name + "\' is ambiguous.";
	}
	else if (entity_reference == nullptr)
	{
		err_msg = "There is no one by the name of \'" + entity_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		entity_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_generic_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string& func_name = wildcards[0];
	std::string& complete_args_string = wildcards[1];
	//err_msg = "";
	std::string dummy_return_value;
	std::vector<std::string> new_call_arg_values = script.extract_args_from_token(complete_args_string, variable_names, variable_values, game_instance);
	script.call_function(game_instance, func_name, new_call_arg_values, dummy_return_value);
	//matched_command_id = 48;
}

void call_generic_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string& func_name = wildcards[0];
	std::vector<std::string> empty_args;
	//err_msg = "";
	script.call_function(game_instance, func_name);
	//matched_command_id = 49;
}

void call_scene_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::string& complete_args_token = wildcards[2];
	std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);
	scene* scene_reference = game_instance->get_scene(scene_name);
	if (scene_reference == nullptr)
	{
		//std::cout << "HERE\n";
		err_msg = "There is scene by the name of \'" + scene_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		scene_reference->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
}

void call_scene_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	std::string scene_name = script.resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
	std::string& func_name = wildcards[1];
	std::vector<std::string> empty_args;
	scene* scene_reference = game_instance->get_scene(scene_name);
	if (scene_reference == nullptr)
	{
		//std::cout << "HERE\n";
		err_msg = "There is scene by the name of \'" + scene_name + "\'";
	}
	else
	{
		std::string dummy_return_value;
		scene_reference->call_function(game_instance, func_name, empty_args, dummy_return_value);
	}
}

void call_this_scene_func_argless_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	scene* this_scene = dynamic_cast<scene*>(&script);
	entity* this_entity = dynamic_cast<entity*>(&script);

	scene* current_scene = nullptr;
	if (this_scene)
	{
		current_scene = this_scene;
	}
	else if (this_entity)
	{
		current_scene = this_entity->get_scene();
	}
	else
	{
		err_msg = "Something has gone horribly wrong";
		return;
	}

	if (current_scene)
	{
		std::string& func_name = wildcards[0];
		std::vector<std::string> empty_args;

		std::string dummy_return_value;
		current_scene->call_function(game_instance, func_name, empty_args, dummy_return_value);
	}
	else
	{
		err_msg = "Could not identify current scene for " + dynamic_cast<game_obj*>(&script)->get_name();
	}
}

void call_this_scene_func_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	scene* this_scene = dynamic_cast<scene*>(&script);
	entity* this_entity = dynamic_cast<entity*>(&script);

	scene* current_scene = nullptr;
	if (this_scene)
	{
		current_scene = this_scene;
	}
	else if (this_entity)
	{
		current_scene = this_entity->get_scene();
	}
	else
	{
		err_msg = "Something has gone horribly wrong";
		return;
	}

	if (current_scene)
	{
		std::string& func_name = wildcards[0];

		std::string& complete_args_token = wildcards[1];
		std::vector<std::string> new_call_args = script.extract_args_from_token(complete_args_token, variable_names, variable_values, game_instance);

		std::string dummy_return_value;
		current_scene->call_function(game_instance, func_name, new_call_args, dummy_return_value);
	}
	else
	{
		err_msg = "Could not identify current scene for " + dynamic_cast<game_obj*>(&script)->get_name();
	}
}

void breakpoint_func(game* game_instance, res_file& script, std::vector<uint32_t>& if_conditions, res_file::line_num& line_num,
	const std::string& code, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values,
	std::vector<std::string>& wildcards, std::string& err_msg, bool& early_return, std::string& return_value)
{
	game_instance->get_engine()->println("BREAKPOINT REACHED");
}


std::vector<res_file::execution_registry_entry> res_file::execution_registry = {
	res_file::execution_registry_entry("if ( $condition )", &if_then_func, true),
	res_file::execution_registry_entry("while ( $condition )", &while_func, true),
	res_file::execution_registry_entry("end", &end_func, true),
	res_file::execution_registry_entry("else if ( $condition )", &else_if_func, true),
	res_file::execution_registry_entry("else", &else_func, true),

	res_file::execution_registry_entry("println : $text", &println_func, false, ": "),
	res_file::execution_registry_entry("print : $text", &print_func, false, ": "),


	res_file::execution_registry_entry("$var [ $index ] = $val", &set_arr_func, false, " []"),
	res_file::execution_registry_entry("$var = $val", &set_var_func, false, " "),

	res_file::execution_registry_entry("return $val", &return_val_func, false),
	res_file::execution_registry_entry("return", &return_nothing_func, false),

	res_file::execution_registry_entry("throw $err", &throw_func, false),
	res_file::execution_registry_entry("throw", &throw_empty_func, false),

	res_file::execution_registry_entry("perspective_entity() . $func_name ()", &call_perspective_entity_func_argless_func, false),
	res_file::execution_registry_entry("perspective_entity() . $func_name ( $args )", &call_perspective_entity_func_func, false),
	res_file::execution_registry_entry("entity_by_alias( $entity_name ) . $func_name ()", &call_entity_func_by_alias_argless_func, false),
	res_file::execution_registry_entry("entity_by_alias( $entity_name ) . $func_name ( $args )", &call_entity_func_by_alias_func, false),
	res_file::execution_registry_entry("entity( $entity_name ) . $func_name ()", &call_entity_func_argless_func, false),
	res_file::execution_registry_entry("entity( $entity_name ) . $func_name ( $args )", &call_entity_func_func, false),
	res_file::execution_registry_entry("first_entity( $entity_name ) . $func_name ()", &call_first_entity_func_argless_func, false),
	res_file::execution_registry_entry("first_entity( $entity_name ) . $func_name ( $args )", &call_first_entity_func_func, false),
	res_file::execution_registry_entry("any_entity( $entity_name ) . $func_name ()", &call_any_entity_func_argless_func, false),
	res_file::execution_registry_entry("any_entity( $entity_name ) . $func_name ( $args )", &call_any_entity_func_func, false),
	res_file::execution_registry_entry("entity_here( $entity_name ) . $func_name ()", &call_entity_here_func_argless_func, false),
	res_file::execution_registry_entry("entity_here( $entity_name ) . $func_name ( $args )", &call_entity_here_func_func, false),
	res_file::execution_registry_entry("first_entity_here( $entity_name ) . $func_name ()", &call_first_entity_here_func_argless_func, false),
	res_file::execution_registry_entry("first_entity_here( $entity_name ) . $func_name ( $args )", &call_first_entity_here_func_func, false),
	res_file::execution_registry_entry("any_entity_here( $entity_name ) . $func_name ()", &call_any_entity_here_func_argless_func, false),
	res_file::execution_registry_entry("any_entity_here( $entity_name ) . $func_name ( $args )", &call_any_entity_here_func_func, false),
	res_file::execution_registry_entry("entity_here_by_alias( $entity_name ) . $func_name ()", &call_any_entity_here_by_alias_func_argless_func, false),
	res_file::execution_registry_entry("entity_here_by_alias( $entity_name ) . $func_name ( $args )", &call_any_entity_here_by_alias_func_func, false),
	res_file::execution_registry_entry("entity_by_alias_here( $entity_name ) . $func_name ()", &call_entity_by_alias_here_func_argless_func, false),
	res_file::execution_registry_entry("entity_by_alias_here( $entity_name ) . $func_name ( $args )", &call_entity_by_alias_here_func_func, false),

	res_file::execution_registry_entry("scene().entity_by_alias( $entity_name ) . $func_name ()", &call_entity_by_alias_here_func_argless_func, false),
	res_file::execution_registry_entry("scene().entity_by_alias( $entity_name ) . $func_name ( $args )", &call_entity_by_alias_here_func_func, false),
	res_file::execution_registry_entry("scene().entity( $entity_name ) . $func_name ()", &call_entity_here_func_argless_func, false),
	res_file::execution_registry_entry("scene().entity( $entity_name ) . $func_name ( $args )", &call_entity_here_func_func, false),
	res_file::execution_registry_entry("scene().first_entity( $entity_name ) . $func_name ()", &call_first_entity_here_func_argless_func, false),
	res_file::execution_registry_entry("scene().first_entity( $entity_name ) . $func_name ( $args )", &call_first_entity_here_func_func, false),
	res_file::execution_registry_entry("scene().any_entity( $entity_name ) . $func_name ()", &call_any_entity_here_func_argless_func, false),
	res_file::execution_registry_entry("scene().any_entity( $entity_name ) . $func_name ( $args )", &call_any_entity_here_func_func, false),

	res_file::execution_registry_entry("scene( $scene_name ).entity_by_alias( $entity_name ) . $func_name ()", &call_entity_by_alias_in_scene_func_argless_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).entity_by_alias( $entity_name ) . $func_name ( $args )", &call_entity_by_alias_in_scene_func_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).entity( $entity_name ) . $func_name ()", &call_entity_in_scene_func_argless_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).entity( $entity_name ) . $func_name ( $args )", &call_entity_in_scene_func_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).first_entity( $entity_name ) . $func_name ()", &call_first_entity_in_scene_func_argless_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).first_entity( $entity_name ) . $func_name ( $args )", &call_first_entity_in_scene_func_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).any_entity( $entity_name ) . $func_name ()", &call_any_entity_in_scene_func_argless_func, false),
	res_file::execution_registry_entry("scene( $scene_name ).any_entity( $entity_name ) . $func_name ( $args )", &call_any_entity_in_scene_func_func, false),

	res_file::execution_registry_entry("scene( $scene_name ) . $func_name ( $args )", &call_scene_func_argless_func, false),
	res_file::execution_registry_entry("scene( $scene_name ) . $func_name ( $args )", &call_scene_func_func, false),
	res_file::execution_registry_entry("scene() . $func_name ()", &call_this_scene_func_argless_func, false),
	res_file::execution_registry_entry("scene() . $func_name ( $args )", &call_this_scene_func_func, false),

	res_file::execution_registry_entry("$func_name ( $args )", &call_generic_func_func, false),
	res_file::execution_registry_entry("$func_name ()", &call_generic_func_argless_func, false),

	res_file::execution_registry_entry("BREAKPOINT", &breakpoint_func, false)
};

void preprocess_line(std::string& line, const string_utils& string_utils, const std::string& filename)
{
	for (size_t i = 0; i < line.size(); ++i)
	{
		char check = line[i];
		if (string_utils.is_command_char(check))
		{
			throw;
		}
	}
	size_t comment_index = line.find("//");
	if (comment_index != std::string::npos)
		line.resize(comment_index);
	string_utils.strip(line);
	std::string prestring, poststring;
	std::vector<std::string> wildcards;
	bool subbed = true;

	while (subbed)
	{
		//std::cout << "LOPO\n";
		subbed = string_utils.complex_replacement(line, "if ( $ ) then", prestring, poststring, wildcards, " ()", false, true);
		if (subbed)
		{
			std::string args;
			for (size_t i = 0; i < wildcards.size(); ++i)
				args += wildcards[i];
			line = prestring + "if (" + args + ")" + poststring;
			//std::cout << "NEWLINE = " << line << std::endl;
		}
	}
	subbed = true;
	while (subbed)
	{
		subbed = string_utils.complex_replacement(line, "while ($) do", prestring, poststring, wildcards, " ()", false, true);
		if (subbed)
		{
			std::string args;
			for (size_t i = 0; i < wildcards.size(); ++i)
				args += wildcards[i];
			line = prestring + "while (" + args + ")" + poststring;
		}
	}

	bool quote_sub = true;

	std::deque<std::string> quoted_material;

	std::string dummy_command_string = "0";
	dummy_command_string[0] = dummy_command_character;

	std::string dummy_equals_string = "0";
	dummy_equals_string[0] = dummy_equals;

	while (quote_sub)
	{
		size_t first_quote_index = std::string::npos;
		size_t second_quote_index = std::string::npos;
		for (size_t i = 0; i < line.size(); ++i)
		{
			char c = line[i];
			if (c == '"')
			{
				if (first_quote_index == std::string::npos)
				{
					first_quote_index = i;
				}
				else
				{
					second_quote_index = i;
					break;
				}
			}
		}
		if (second_quote_index != std::string::npos && second_quote_index != first_quote_index + 1)
		{
			std::string quote = string_utils.substring(line, first_quote_index + 1, second_quote_index - 1);
			quote = string_utils.replace_all(quote, "=", dummy_equals_string, false);
			quoted_material.push_back(quote);
			if (second_quote_index == line.size() - 1)
				line = line.substr(0, first_quote_index) + dummy_command_string;
			else
				line = line.substr(0, first_quote_index) + dummy_command_string + line.substr(second_quote_index + 1);

			quote_sub = true;
		}
		else
		{
			quote_sub = false;
		}
	}

	line = " " + line + " ";
	const std::string delimeters = " ()+=[]";
	auto substitute_alias_function = [&](const std::string& alias_func, const std::string& original_func)
	{
		for (size_t i = 0; i < delimeters.size(); ++i)
		{
			std::string delimeter = "0";
			delimeter[0] = delimeters[i];
			line = string_utils.replace_all(line, delimeter + alias_func + "(", delimeter + original_func + "(", false);
			line = string_utils.replace_all(line, delimeter + alias_func + " (", delimeter + original_func + "(", false);
		}
	};


	substitute_alias_function("get_interrupter", "interrupter");
	substitute_alias_function("get_true_name", "get_name");

	substitute_alias_function("get_consecutive_command_repeats", "get_command_repeats");
	substitute_alias_function("get_repeats", "get_command_repeats");
	substitute_alias_function("get_consecutive_repeats", "get_command_repeats");

	substitute_alias_function("knows_alias_of_entity", "knows_alias");
	substitute_alias_function("knows_alias_of_other_entity", "knows_alias");
	substitute_alias_function("knows_alias_of", "knows_alias");

	substitute_alias_function("was_interrupted", "interrupted");
	substitute_alias_function("get_interrupted", "interrupted");

	substitute_alias_function("double", "decimal");
	substitute_alias_function("float", "decimal");

	substitute_alias_function("floor", "int");

	substitute_alias_function("is_integer", "is_int");

	substitute_alias_function("is_float", "is_decimal");
	substitute_alias_function("is_double", "is_decimal");
	substitute_alias_function("is_arr", "is_array");
	substitute_alias_function("is_numeric", "is_number");

	substitute_alias_function("rand", "random");

	substitute_alias_function("general_substitution", "generic_substitution");

	substitute_alias_function("get_var", "get_value");
	substitute_alias_function("get_val", "get_value");
	substitute_alias_function("get_variable", "get_value");
	
	substitute_alias_function("set_var", "set_value");
	substitute_alias_function("set_val", "set_value");
	substitute_alias_function("set_variable", "set_value");

	substitute_alias_function("get_global_var", "get_global_value");
	substitute_alias_function("get_global_val", "get_global_value");
	substitute_alias_function("get_global_variable", "get_global_value");

	substitute_alias_function("set_global_var", "set_global_value");
	substitute_alias_function("set_global_val", "set_global_value");
	substitute_alias_function("set_global_variable", "set_global_value");

	substitute_alias_function("substring", "substr");
	substitute_alias_function("get_substring", "substr");
	substitute_alias_function("get_substr", "substr");

	substitute_alias_function("concatanate", "concat");
	substitute_alias_function("concatenate", "concat");
	substitute_alias_function("strcat", "concat");

	substitute_alias_function("copy", "clone");

	substitute_alias_function("delete_aliases", "wipe_aliases");
	substitute_alias_function("erase_aliases", "wipe_aliases");
	substitute_alias_function("reset_aliases", "wipe_aliases");

	substitute_alias_function("take_damage", "damage");
	substitute_alias_function("reduce_hp", "damage");
	substitute_alias_function("reduce_health", "damage");
	substitute_alias_function("hurt", "damage");

	substitute_alias_function("get_meta_var", "get_meta_value");
	substitute_alias_function("get_meta_val", "get_meta_value");
	substitute_alias_function("get_meta_variable", "get_meta_value");

	substitute_alias_function("set_meta_var", "set_meta_value");
	substitute_alias_function("set_meta_val", "set_meta_value");
	substitute_alias_function("set_meta_variable", "set_meta_value");

	substitute_alias_function("set_attachment", "attach_to");

	substitute_alias_function("recover_hp", "recover");
	substitute_alias_function("recover_health", "recover");
	substitute_alias_function("heal", "recover");
	substitute_alias_function("gain_hp", "recover");
	substitute_alias_function("gain_health", "recover");
	substitute_alias_function("increase_hp", "recover");
	substitute_alias_function("increase_health", "recover");

	substitute_alias_function("get_health", "get_hp");
	substitute_alias_function("get_max_health", "get_max_hp");

	substitute_alias_function("set_health", "set_hp");

	substitute_alias_function("set_scene", "transfer");
	substitute_alias_function("set_to_scene", "transfer");
	substitute_alias_function("move_to_scene", "transfer");
	substitute_alias_function("move_to", "transfer");
	substitute_alias_function("transfer_to_scene", "transfer");
	substitute_alias_function("transfer_to", "transfer");
	substitute_alias_function("change_to_scene", "transfer");
	substitute_alias_function("change_scene", "transfer");

	substitute_alias_function("upper", "to_uppercase");
	substitute_alias_function("to_upper", "to_uppercase");
	substitute_alias_function("uppercase", "to_uppercase");
	substitute_alias_function("capitalize", "to_uppercase");
	substitute_alias_function("capital", "to_uppercase");
	substitute_alias_function("to_capitals", "to_uppercase");
	substitute_alias_function("make_uppercase", "to_uppercase");
	
	substitute_alias_function("lower", "to_lowercase");
	substitute_alias_function("to_lower", "to_lowercase");
	substitute_alias_function("lowercase", "to_lowercase");
	substitute_alias_function("make_lowercase", "to_lowercase");


	//					RETURNING QUOTE LITERALS
	while (quoted_material.size() > 0)
	{
		line = string_utils.replace_first(line, dummy_command_string, "\"" + quoted_material[0] + "\"");
		quoted_material.pop_front();
	}
	string_utils.strip(line);
}

void res_file::check_line_match(const std::string& line, line_num line_num)
{
	if (line.size() == 0)
	{
		line_commands.push_back(-1);
		return;
	}
	string_utils string_utils;
	std::vector<std::string> dummy_wildcards;
	for (size_t i = 0; i < execution_registry.size(); ++i)
	{
		execution_registry_entry& entry = execution_registry[i];
		if (string_utils.matches_command(entry.pattern, line, dummy_wildcards, entry.delimeters))
		{
			//std::cout << "Found match for command '" << line << "' in file " << filename <<  " = command #" << i << " ('" << entry.pattern << "'), line " << line_num << std::endl;
			line_commands.push_back(i);
			return;
		}
	}
	line_commands.push_back(-1);
}

bool res_file::add_lines_from_file(const engine* engine, const std::string& scenario_name, const std::string& name)
{
	std::ifstream file;
	string_utils string_utils;

	file.open(name);
	if (file.is_open())
	{
		std::string line;
		std::vector<std::string> dummy_wildcards;
		std::vector<std::string> imported_files; //Files to be included from engine packages.
		std::vector<std::string> included_files; //Files to be included from the local scenario packages
		std::vector<std::string> command_funcs_to_alias_with_input_substitution;
		std::vector<std::string> command_funcs_to_alias_with_input_substitution_args;
		while (file.good() && file.is_open() && !file.eof())
		{
			std::getline(file, line);
			//++current_line_number;
			preprocess_line(line, string_utils, name);
			if (string_utils.matches_command("function $func_name ( $args )", line, dummy_wildcards, " ()"))
			{
				const std::string& func_name = dummy_wildcards[0];
				scripted_functions.push_back(func_name);
				line_commands.push_back(0);
			}
			else if (string_utils.matches_command("function $func_name ()", line, dummy_wildcards, " ()"))
			{
				const std::string& func_name = dummy_wildcards[0];
				scripted_functions.push_back(func_name);
				line_commands.push_back(0);
			}
			else
			{
				check_line_match(line, line_data.size());
			}
			if (string_utils.matches_command("function command: $func ( $args )", line, dummy_wildcards, " ():"))
			{
				line = "function command: " + engine->extra_text_processing(dummy_wildcards[0]) + "(" + dummy_wildcards[1] + ")";
				command_func_lines.push_back(line_data.size());
			}

			
			if (engine != nullptr)
			{
				std::vector<std::string> wildcards;
				const static std::string command1 = "#include";
				const static std::string command2 = "#import";
				if (string_utils.matches_command(command1+" $other_file", line, wildcards))
				{
					included_files.push_back("Scenarios/"+scenario_name+"/templates/" + line.substr(command1.length() + 1));
					line_data.push_back("");
				}
				else if (string_utils.matches_command(command2 + " $other_file", line, wildcards))
				{
					std::string other_file = "gamedata/packages/" + line.substr(command2.length() + 1);
					imported_files.push_back(other_file);
					line_data.push_back("");
				}
				else
				{
					line_data.push_back(line);
					process_line_from_file(line);
				}
			}
			else
			{
				line_data.push_back(line);
				process_line_from_file(line);
			}
		}
		file.close();

		size_t i;
		for (i = 0; i < command_funcs_to_alias_with_input_substitution.size(); ++i)
		{
			const std::string& original_func_name = command_funcs_to_alias_with_input_substitution[i];
			const std::string& args = command_funcs_to_alias_with_input_substitution_args[i];
			std::string alias_func_name = engine->extra_text_processing(original_func_name);
			//++current_line_number;
			command_func_lines.push_back(line_data.size());
			line_data.push_back("function command:" + alias_func_name + " (" + args + ")");
			line_data.push_back("command:"+original_func_name + "(" + args + ")");
			line_data.push_back("end");
		}

		for (i = 0; i < included_files.size(); ++i)
		{
			std::string& other_file = included_files[i];
			if (!add_lines_from_file(engine, scenario_name, other_file))
				return false;
		}

		for (i = 0; i < imported_files.size(); ++i)
		{
			std::string& other_file = imported_files[i];
			if (!add_lines_from_file(engine, scenario_name, other_file))
				return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

std::string res_file::call_function(game* game_instance, const std::string& function_name)
{
	std::vector<std::string> empty_args;
	std::string dummy_return_value;
	return call_function(game_instance, function_name, empty_args, dummy_return_value);
}

std::string res_file::call_function(game* game_instance, const std::string& function_name, std::initializer_list<std::string> arg_values, std::string& return_value)
{
	std::vector<std::string> args;
	args.reserve(arg_values.size());
	for (auto i = arg_values.begin(); i != arg_values.end(); ++i)
		args.push_back(*i);

	return call_function(game_instance, function_name, args, return_value);
}

std::string res_file::call_function(game* game_instance, const std::string& function_name, std::vector<std::string>& arg_values, std::string& return_value)
{
	//std::cout << "Calling function " << function_name << std::endl;
	return_value = "NULL";
	if (!finished_loading)
		return "";
	std::string args_string = "";
	for (size_t arg_def = 0; arg_def < arg_values.size(); ++arg_def)
	{
		if (arg_def != 0)
			args_string += " , ";
		args_string += " $arg ";
	}
	std::string err_msg{""};
	std::vector<std::string> arg_names;
	arg_names.reserve(arg_values.size());
	//string_utils string_utils;
	line_num start = find_match_remove_wildcards_from_function_name(0, "function " + function_name, "(" + args_string + ")", arg_names);//find_match(0, "function " + function_name + "(" + args_string + ")", arg_names);
	
	if (std::find(innate_functions.begin(), innate_functions.end(), function_name) != innate_functions.end())
	{
		if (start != NO_MATCH) //If start ISN'T no match, then it means the user tried to override this innate function
		{
			err_msg = "Error: Cannot override innate function \'" + function_name + "\'";
			game_instance->get_engine()->println(err_msg);
			return err_msg;
		}
		else
		{
			err_msg = call_innate_function(game_instance, function_name, arg_values);
			if (err_msg != "")
				game_instance->get_engine()->println(err_msg);
			return err_msg;
		}
	}
	
	if (start == NO_MATCH)
	{
		return_value = "NO_SUCH_FUNCTION_" + function_name;
		std::string full_args;
		for (size_t i = 0; i < arg_values.size(); ++i)
		{
			if (i != 0)
				full_args += ", ";
			full_args += "'" + arg_values[i] + "'";
		}
		err_msg = "Could not locate function '" + function_name  + "' with " + std::to_string(arg_values.size()) + " args: " + full_args;
		game_instance->get_engine()->println(err_msg);
		//game_instance->get_engine()->println("Error in file \"", filename, "\": Could not locate function \"", function_name, "\" with ",arg_values.size()," arguments");
		return err_msg;
	}
	line_num current_line = start + 1;
	int line_layer = 0;
	int execution_layer = 0;
	bool early_return = false;
	std::vector<uint32_t> if_conditions;
	while (err_msg == "" && !early_return)
	{ //At this point the arg name / value vectors are essentially utilized as variable name / value vectors, including user-defined variables.
		execute_line(game_instance, current_line, err_msg, line_layer, execution_layer, arg_names, arg_values, early_return, return_value, if_conditions);
	}
	if (current_line == lines())
		err_msg = "No corresponding 'end' statement";
	if (err_msg != "" && game_instance->game_is_active())
	{
		return_value = "ERROR";
		game_instance->get_engine()->println("Encountered error on line ", current_line , " of ", filename, " during call to function '", function_name, ": ", err_msg );
	}
	return err_msg;
}

//Meant to be overwritten by subclasses so they can define their own innate functions.
std::string res_file::call_innate_function(game* game_instance, const std::string& function_name, std::vector<std::string>& args)
{
	return "";
}

bool res_file::evaluate_condition(game* game_instance, const std::string& condition, std::string& err_msg, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values)
{
	string_utils string_utils;
	std::string processed_condition = resolve_expression(condition, variable_names, variable_values, game_instance);
	string_utils.strip(processed_condition);

	if(string_utils.is_integer(processed_condition))
	{
		int numeric = std::stoi(processed_condition);
		if (numeric != 0)
			return true;
		else
			return false;
	}
	else
	{
		if (   (string_utils.get_lowercase(processed_condition) == "true") || (string_utils.get_lowercase(processed_condition) == "yes")   )
			return true;
		else
			return false;
	}
}

void res_file::execute_line(game* game_instance, line_num& line, std::string& err_msg, int& line_layer, int& execution_layer, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values, bool& early_return, std::string& return_value, std::vector<uint32_t>& if_conditions)
{
	if (!game_instance->game_is_active())
	{
		err_msg = "Game has concluded.";
		return;
	}
	line_data;
	line_commands;
	string_utils string_utils;
	std::string code = get_line(line);
	//std::string precomment;
	//std::string postcomment;
	//std::vector<std::string> comment_wildcard;
	//string_utils.complex_replacement(code, "//$", precomment, postcomment, comment_wildcard);
	//code = precomment;
	std::vector<std::string> wildcards;
	bool matched_control_statement = false;
	
	int matched_command_id = 0;

	if (code.size() == 0)
	{
		++line;
		return;
	}
	//std::cout << "Executing line " << line << " (" << code << ")" << " = command #" << line_commands[line] << std::endl;
	
	if (line_commands[line] != -1)
	{
		execution_registry_entry& entry = execution_registry[line_commands[line]];
		command_caller& func = entry.func;
		const std::string& pattern = entry.pattern;
		std::vector<std::string> wildcards;
		auto try_call_func = [&]()
		{
			if (string_utils.matches_command(entry.pattern, code, wildcards, entry.delimeters))
			{
				//std::cout << "Executed command #" << line_commands[line] << "(" << pattern << ") on line " << line << ": " << code << std::endl;
				func(game_instance, *this, if_conditions, line, code, line_layer, execution_layer, variable_names, variable_values, wildcards, err_msg, early_return, return_value);
			}
			else
			{
				err_msg = "An unexpected error has occured.";
			}
		};

		if (execution_layer == line_layer)
		{
			try_call_func();
		}
		else
		{
			if (entry.control_command)
			{
				try_call_func();
			}
		}
		
	}
	else
	{
		err_msg = "Unable to parse code ('" + code + "')";
		err_msg += "\n";
		for (size_t i = 0; i < lines(); ++i)
		{
			//err_msg += "Line " + std::to_string(i) + ": " + get_line(i) + "; matched command = #" + std::to_string(line_commands[i]) + "\n";
			err_msg += "Line " + std::to_string(i) + ": " + get_line(i) + "\n";
		}
	}

	//bool found_error_in_variable_name = false;
	//bool matched_first_chunk = false;
	
	//std::cout << code << " matched with command id " << matched_command_id << std::endl;
	/*
	if (matched_command_id != 50 && matched_command_id != 51 && found_error_in_variable_name)
	{
		err_msg = "";
	}
	*/
	if(err_msg == "")
		++line;
}


//No instructions here - just meant for subclasses to override.
void res_file::execute_line_from_class(game* game_instance, line_num& line, std::string& err_msg, std::vector<std::string>& wildcards, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values)
{

}

std::vector<std::string> res_file::extract_args_from_token(std::string complete_args_token, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance)
{
	complete_args_token = resolve_expression(complete_args_token, variable_names, variable_values, game_instance);
	string_utils string_utils;
	std::vector<std::string> new_call_arg_values;
	std::vector<std::string> arg_tokens = string_utils.extract_tokens(complete_args_token, ",");
	new_call_arg_values.reserve(arg_tokens.size());
	for (size_t i = 0; i < arg_tokens.size(); ++i)
	{
		if (arg_tokens[i] != ",")
		{
			if (arg_tokens[i][0] == ' ')
			{
				arg_tokens[i] = arg_tokens[i].substr(1);
			}
			new_call_arg_values.push_back(resolve_expression(arg_tokens[i], variable_names, variable_values, game_instance));
		}
	}

	return new_call_arg_values;
}

res_file::line_num res_file::find_exact(line_num starting_line, const std::string& line)
{
	for (line_num i = 0; i < lines(); ++i)
	{
		if (line_data[i] == line)
			return i;
	}
	return NO_MATCH;
}

res_file::line_num res_file::find_match(line_num starting_line, const std::string& command)
{
	string_utils string_utils;
	std::vector<std::string> dummy_vec;
	for (line_num i = 0; i < lines(); ++i)
	{
		if (string_utils.matches_command(command, line_data[i], dummy_vec))
			return i;
	}
	return NO_MATCH;
}

res_file::line_num res_file::find_match(line_num starting_line, const std::string& command, std::vector<std::string>& wildcards)
{
	string_utils string_utils;
	for (line_num i = 0; i < lines(); ++i)
	{
		if (string_utils.matches_command(command, line_data[i], wildcards))
			return i;
	}
	return NO_MATCH;
}

/*Highly bespoke. Only used by call_function to sanitize function names so that wildcard function names work for the purpose 
of allowing scripts to define functions with wildcards in the name that get passed into it as arguments, used to define 
custom commands for scenes & entities
*/
res_file::line_num res_file::find_match_remove_wildcards_from_function_name(line_num starting_line, std::string function, const std::string& args_string, std::vector<std::string>& wildcards)
{
	//string_utils string_utils;
	//std::cout << function << std::endl;
	string_utils string_utils;
	auto found = function_line_nums.find(function);
	if (found != function_line_nums.end())
	{
		line_num i = found->second;
		if (i == NO_MATCH)
			return NO_MATCH;
		std::string line = string_utils.replace_all(line_data[i], "$", "", false);

		if (string_utils.matches_command(function + args_string, line, wildcards))
		{
			//function_line_nums.insert_or_assign(function, i);
			//std::cout << function << " is at LINE " << i << std::endl;
			return i;
		}
		else
		{
			return NO_MATCH; //In theory this should never execute, but including this eliminates a compiler warning.
		}
	}
	else
	{
		function = string_utils.replace_all(function, "$", "", false);
		for (line_num i = 0; i < lines(); ++i)
		{
			std::string line = string_utils.replace_all(line_data[i], "$", "", false);

			if (string_utils.matches_command(function + args_string, line, wildcards))
			{
				function_line_nums.insert_or_assign(function, i);
				//std::cout << function << " is at LINE " << i << std::endl;
				return i;
			}
		}
		function_line_nums.insert_or_assign(function, NO_MATCH);
		return NO_MATCH;
	}
}

res_file::line_num res_file::find_match(line_num starting_line, const std::string& command, std::vector<std::string>& wildcards, const std::string& delimeters)
{
	string_utils string_utils;
	for (line_num i = 0; i < lines(); ++i)
	{
		if (string_utils.matches_command(command, line_data[i], wildcards, delimeters))
			return i;
	}
	return NO_MATCH;
}

const std::string& res_file::get_line(line_num line_number) const
{
	return line_data[line_number];
}

const std::string& res_file::get_filename() const
{
	return filename;
}

const std::vector<res_file::line_num>& res_file::get_command_func_lines() const
{
	return command_func_lines;
}

res_file::line_num res_file::lines() const
{
	return line_data.size();
}

void res_file::process_line_from_file(const std::string& line)
{
	//Meant to be overridden by subclasses
}

bool res_file::read(const engine* engine, const std::string& scenario_name, const std::string& filename)
{
	string_utils string_utils;
	line_data.clear();
	line_data.emplace_back();
	command_func_lines.clear();
	function_line_nums.clear();
	line_commands.clear();
	line_commands.push_back(-1);
	this->filename = filename;
	bool added_lines = add_lines_from_file(engine, scenario_name, filename);
	if (added_lines)
		finished_loading = true;
	else
		this->filename = "";
	return added_lines;
}

bool res_file::read_raw(const std::string& filename)
{
	string_utils string_utils;
	line_data.clear();
	line_data.emplace_back();
	command_func_lines.clear();
	function_line_nums.clear();
	line_commands.clear();
	line_commands.push_back(-1);
	this->filename = filename;

	auto add_raw_lines = [&]() -> bool
	{
		std::ifstream file;
		file.open(filename);
		if (file.is_open())
		{
			std::string line;
			while (file.good() && !file.eof())
			{
				std::getline(file, line);
				line_data.push_back(line);
			}
		}
		else
		{
			return false;
		}
	};

	bool added_lines = add_raw_lines();
	if (added_lines)
		finished_loading = true;
	else
		this->filename = "";
	return added_lines;
}

void res_file::register_innate_function(const std::string& innate_function_name)
{
	innate_functions.push_back(innate_function_name);
}

std::string res_file::resolve_expression(std::string raw_value, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance)
{
	if (!game_instance->game_is_active())
		return raw_value;
	string_utils string_utils;

	std::vector<std::string> quotes_wildcards;
	std::string pre_quote_string;
	std::string post_quote_string;
	
	std::string dummy_command_string = "[";
	dummy_command_string[0] = dummy_command_character;
	std::deque<std::string> quoted_material;

	string_utils.strip(raw_value);

	bool quote_sub = true;
				//DEBUG_BREAKPOINT(1);
	while (quote_sub)
	{
		//std::cout << raw_value << std::endl;
		size_t first_quote_index = std::string::npos;
		size_t second_quote_index = std::string::npos;
		for (size_t i = 0; i < raw_value.size(); ++i)
		{
			char c = raw_value[i];
			if (c == '"')
			{
				if (first_quote_index == std::string::npos)
				{
					first_quote_index = i;
				}
				else
				{
					second_quote_index = i;
					break;
				}
			}
		}
		if (second_quote_index != std::string::npos && second_quote_index!=first_quote_index+1)
		{
			std::string quote = string_utils.substring(raw_value, first_quote_index + 1, second_quote_index - 1);
			for (size_t i = 0; i < quote.size(); ++i)
			{
				char& c = quote[i];
				if (c == ',')
					c = dummy_comma;
				else if (c == '(')
					c = dummy_left_paran;
				else if (c == ')')
					c = dummy_right_paran;
				else if (c == '/')
					c = dummy_slash;
				else if (c == '+')
					c = dummy_plus;
				else if (c == '*')
					c = dummy_star;
				else if (c == '-')
					c = dummy_dash;
				else if (c == '%')
					c = dummy_percent;
			}
			if (quote == " ")
			{
				quote[0] = dummy_space;
			}
			quoted_material.push_back(quote);
			if (second_quote_index == raw_value.size() - 1)
				raw_value = raw_value.substr(0, first_quote_index) + dummy_command_string;
			else
				raw_value = raw_value.substr(0, first_quote_index) + dummy_command_string + raw_value.substr(second_quote_index + 1);

			quote_sub = true;
		}
		else
		{
			quote_sub = false;
		}
	}
					//DEBUG_BREAKPOINT(2);
	std::vector<std::string> wildcards;
	raw_value = substitute_variables(raw_value, variable_names, variable_values, game_instance);
					//DEBUG_BREAKPOINT(3);
	//					RETURNING QUOTE LITERALS
	while (quoted_material.size() > 0)
	{
		raw_value = string_utils.replace_first(raw_value, dummy_command_string, quoted_material[0]);
		quoted_material.pop_front();
	}
					//DEBUG_BREAKPOINT(4);
	//					TOKENIZE EXCLAMATION MARKS
	for (size_t i = 0; i < raw_value.size(); ++i)
	{
		char c = raw_value[i];
		if (c == '!')
		{
			if (i == raw_value.size() - 1)
				raw_value = raw_value.substr(0, i) + " !";
			else
				raw_value = raw_value.substr(0, i) + " ! " + raw_value.substr(i + 1);
			++i;
		}
	}

	if (raw_value.size() > 0)
	{
		if (raw_value[raw_value.size() - 1] == ' ')
			raw_value.resize(raw_value.size() - 1);
		if (raw_value[0] == ' ')
			raw_value = raw_value.substr(1);
	}
	
	//std::cout << "RESOLVING EXPRESSION " << raw_value << std::endl;

	//								BEGIN HEAVY LIFTING

	bool has_subbed = true;
	std::string prestring;
	std::string poststring;

	
	typedef std::string(*char_getter_handler)(game*, entity*, std::vector<std::string>&);

	auto register_entity_getter = [&](const std::string& char_script_func_name, char_getter_handler func, entity* this_entity)
	{
		//                                       HANDLES BASE FUNCTION CALLS
		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false,true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_entity(char_name, false);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					//std::cout << "EGASRGAE " << raw_value << std::endl;
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity_by_alias($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false,true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_entity(char_name, true);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "first_entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false,true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_first_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "any_entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_any_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "perspective_entity()." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::vector<std::string> args = extract_args_from_token(wildcards[0], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_perspective_entity();
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}


		//                      HANDLES ARGUMENT-LESS INNATE GETTERS

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_entity(char_name, false);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity_by_alias($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_entity(char_name, true);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "first_entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_first_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "any_entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_any_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "perspective_entity()." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_perspective_entity();
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}


		//                      HANDLES GETTERS FOR INNATE FUNCTIONS OF ENTITIES WITHIN THE CURRENT SCENE

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity_here_by_alias($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, true);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), true);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}


		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity_here($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, false);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), false);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "first_entity_here($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_first_entity(game_instance, char_name);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_first_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "any_entity_here($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_scene->get_name());
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}


		//                    ARGUMENTLESS SCENE-SPECIFIC INNATE GETTERS FOR ENTITIES

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity_here_by_alias($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, true);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), true);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity_here($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, false);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), false);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "first_entity_here($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_first_entity(game_instance, char_name);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_first_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "any_entity_here($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_scene->get_name());
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		//					HERE IS MORE FOR ACCESSING LOCAL ENTITIES, BUT USING scene()... SYNTAX

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().entity_by_alias($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, true);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), true);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, false);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), false);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().first_entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_first_entity(game_instance, char_name);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_first_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().any_entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_scene->get_name());
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}


		//                    ARGUMENTLESS SCENE-SPECIFIC INNATE GETTERS FOR ENTITIES VIA SCENE()... SYNTAX

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().entity_by_alias($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, true);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), true);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_entity(game_instance, char_name, false);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_entity_in_scene(char_name, this_entity->get_scene_name(), false);
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().first_entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = this_scene->get_first_entity(game_instance, char_name);
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_first_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene().any_entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = nullptr;
				const scene* this_scene = dynamic_cast<const scene*>(this); //Extremely bad practice for a class to be aware of its subclasses.
				const entity* this_entity = dynamic_cast<const entity*>(this);
				if (this_scene)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_scene->get_name());
				}
				else if (this_entity)
				{
					char_ptr = game_instance->get_any_entity_in_scene(char_name, this_entity->get_scene_name());
				}
				else
				{
					char_ptr = nullptr;
				}

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		//					HERE IS WHERE CODE GOES TO CALL INNATE GETTERS ON ENTITIES WITHIN A SCENE BY NAME

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).entity_by_alias($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, true);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}


		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).first_entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).any_entity($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
			else
			{
				has_subbed = false;
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).entity_by_alias($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, true);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}

		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).first_entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).any_entity($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);

				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else if (!(args.size() == 1 && args[0]==")"))
				{
					raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
				}
			}
		}



		//										 HANDLES SELF FUNCTION CALLS
		if (this_entity)
		{

			//Have to make sure it's not trying to call a scene getter instead.

			has_subbed = string_utils.complex_replacement(raw_value, "scene()."+char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false, true);
			if(!has_subbed)
				has_subbed = string_utils.complex_replacement(raw_value, "scene()." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			
			if (!has_subbed)
				has_subbed = string_utils.complex_replacement(raw_value, "scene($)." + char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false, true);

			if (!has_subbed)
				has_subbed = string_utils.complex_replacement(raw_value, "scene($)." + char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
			
			if (!has_subbed)
			{

				has_subbed = true;
				while (has_subbed)
				{
					has_subbed = string_utils.complex_replacement(raw_value, char_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
					if (has_subbed)
					{
						std::vector<std::string> args;

						entity* char_ptr = this_entity;
						if (char_ptr == nullptr)
						{
							raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
						}
						else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
						{
							raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
						}
						else
						{
							raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
						}
					}
				}

				has_subbed = true;
				while (has_subbed)
				{
					has_subbed = string_utils.complex_replacement(raw_value, char_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false, true);
					if (has_subbed)
					{
						std::vector<std::string> args = extract_args_from_token(wildcards[0], variable_names, variable_values, game_instance);
						entity* char_ptr = this_entity;
						if (char_ptr == nullptr)
						{
							raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
						}
						else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
						{
							raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
						}
						else if (!(args.size() == 1 && args[0] == ")"))
						{
							//std::cout << "AEGHSRGEAWFE " << raw_value << std::endl;
							raw_value = prestring + func(game_instance, char_ptr, args) + poststring;
						}
						else
						{
							has_subbed = false;
						}
					}
				}

			}
			
		}

	};

	//THIS IS WHERE CODE GOES THAT HANDLES GETTING RETURN VALUES FROM ENTITY INNATE FUNCTIONS

	char_getter_handler get_val_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR get_value; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			return char_ptr->get_value(args[0]);
		}
	};

	char_getter_handler get_global_val_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR get_global_value; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			return game_instance->get_value(args[0]);
		}
	};

	char_getter_handler get_meta_val_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR get_global_value; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			return game_instance->get_meta_value(args[0]);
		}
	};


	//TODO: Make it so user can toggle whether or not the display name retrieved is randomized.
	char_getter_handler get_display_name_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0 && args.size() != 1)
		{
			std::string all_args;
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (i != 0)
					all_args += ", ";
				all_args += "'" + args[i] + "'";
			}
			return "INVALID ARGS FOR 'get_display_name'; EXPECTED 0 OR 1 BUT GOT " + std::to_string(args.size()) + ": " + all_args;
		}
		else if (args.size() == 0)
		{
			return char_ptr->get_display_name(true, true);
		}
		else if (args.size() == 1)
		{
			entity* found_entity = game_instance->get_entity_by_name(args[0]);
			if (found_entity)
			{
				return found_entity->get_display_name_of_other_entity(true, true, char_ptr);
			}
			else
			{
				return "NO_SUCH_ENTITY_" + args[0];
			}
		}
		else
		{
			return "GRAVE_ERROR";
		}
	};

	char_getter_handler entity_exists_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR 'entity_exists'; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			bool exists = game_instance->get_entity(args[0], char_ptr) != nullptr;

			if (exists)
			{
				return "true";
			}
			else
			{
				return "false";
			}
		}
	};

	char_getter_handler entity_exists_here_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR 'entity_exists_here'; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			bool exists = game_instance->get_entity_in_scene(args[0], char_ptr->get_scene_name(), char_ptr) != nullptr;

			if (exists)
			{
				return "true";
			}
			else
			{
				return "false";
			}
		}
	};

	char_getter_handler get_command_repeats_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'get_command_repeats'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			uint64_t repeats = char_ptr->get_consecutive_command_repeats();

			return std::to_string(repeats);
		}
	};

	char_getter_handler knows_alias_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 2)
		{
			return "INVALID ARGS FOR 'knows_alias'; EXPECTED 2, GOT " + args.size();
		}
		else
		{
			std::string& other_entity_true_name = args[1];
			std::string& alias = args[0];
			entity* other_entity = game_instance->get_entity(other_entity_true_name, false);
			bool knows = false;
			if (other_entity)
			{
				if (char_ptr->knows_alias(alias, other_entity))
					knows = true;
			}
			else
			{
				return "FOUND NO ENTITY WITH TRUE NAME OF " + other_entity_true_name;
			}
			if (knows)
				return "true";
			else
				return "false";
		}
	};

	char_getter_handler get_true_name_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			std::string all_args;
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (i != 0)
					all_args += ", ";
				all_args += args[i];
			}
			return "INVALID ARGS FOR 'get_name'; EXPECTED 0, GOT " + std::to_string(args.size()) + ": " + all_args;
		}
		else
		{
			return char_ptr->get_name() + " ";
		}
	};

	char_getter_handler get_hp_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'get_hp'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			return std::to_string(char_ptr->get_hp());
		}
	};

	char_getter_handler get_max_hp_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'get_max_hp'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			return std::to_string(char_ptr->get_max_hp());
		}
	};

	char_getter_handler get_interrupted_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'interrupted'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			if (char_ptr->was_interrupted())
			{
				return "true";
			}
			else
				return "false";
		}
	};

	char_getter_handler get_interrupter_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'interrupter'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			if (char_ptr->was_interrupted())
			{
				return char_ptr->get_interrupter();
			}
			else
				return "NULL";
		}
	};

	char_getter_handler get_last_command_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'interrupter'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			return char_ptr->get_last_command();
		}
	};

	char_getter_handler get_attachment_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'get_attachment'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			return char_ptr->get_attachment_name();
		}
	};

	char_getter_handler get_attachments_handler = [](game* game_instance, entity* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			return "INVALID ARGS FOR 'get_attachment'; EXPECTED 0, GOT " + args.size();
		}
		else
		{
			std::string var = "{";
			const auto& attachments = char_ptr->get_attached_entity_names();
			bool first = true;
			for (auto i = attachments.begin(); i != attachments.end(); ++i)
			{
				const std::string& attachment = *i;
				if (!first)
					var += ",";
				else
					first = false;
				
				var += attachment;
			}
			var += "}";
			return var;
		}
	};


	entity* this_entity = dynamic_cast<entity*>(const_cast<res_file*>(this));
	register_entity_getter("get_value", get_val_handler, this_entity);
	register_entity_getter("get_global_value", get_global_val_handler, this_entity);
	register_entity_getter("get_meta_value", get_global_val_handler, this_entity);
	register_entity_getter("get_display_name", get_display_name_handler, this_entity);
	register_entity_getter("get_name", get_true_name_handler, this_entity);
	register_entity_getter("entity_exists", entity_exists_handler, this_entity);
	register_entity_getter("entity_exists_here", entity_exists_here_handler, this_entity);
	register_entity_getter("get_command_repeats", get_command_repeats_handler, this_entity);
	register_entity_getter("knows_alias", knows_alias_handler, this_entity);

	register_entity_getter("get_hp", get_hp_handler, this_entity);
	register_entity_getter("get_max_hp", get_max_hp_handler, this_entity);

	register_entity_getter("interrupted", get_interrupted_handler, this_entity);

	register_entity_getter("interrupter", get_interrupter_handler, this_entity);

	register_entity_getter("get_last_command", get_last_command_handler, this_entity);
	register_entity_getter("get_attachment", get_attachment_handler, this_entity);
	register_entity_getter("get_attachments", get_attachments_handler, this_entity);

	//////////////////////////////////////////////////////////////////////////////////
	/* HERE IS WHERE CODE GOES THAT CAN HANDLE GETTING RETURN VALUES FROM USER-FUNCTION CALLS TO ENTITIES */
	{

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				//std::cout << "HERE  " << raw_value << std::endl;
				
				entity* char_ptr = game_instance->get_entity(char_name, false);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "entity($).$()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args;
				//std::cout << "5euthdsrgre\n";
				entity* char_ptr = game_instance->get_entity(char_name, false);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "first_entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_first_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "first_entity($).$()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_first_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "any_entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_any_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "any_entity($).$()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_any_entity(char_name);
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "perspective_entity() . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string func_name = wildcards[0];
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				entity* char_ptr = game_instance->get_perspective_entity();
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "perspective_entity().$()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string func_name = wildcards[0];
				std::vector<std::string> args;

				entity* char_ptr = game_instance->get_perspective_entity();
				if (char_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
				}
				else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
				{
					raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
				}
				else
				{
					std::string return_val;
					char_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	/*     HERE IS WHERE CODE GOES THAT HANDLES CALLS TO USER FUNCTIONS IN SCENES    */
	
	typedef std::string(*scene_getter_handler)(game*, scene*, std::vector<std::string>&);

	auto register_scene_getter = [&](const std::string& scene_script_func_name, scene_getter_handler func, scene* this_scene, entity* this_entity)
	{
		//                                       HANDLES BASE FUNCTION CALLS
		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($)." + scene_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args = extract_args_from_token(wildcards[1], variable_names, variable_values, game_instance);
				if (!(args.size() == 1 && args[0] == ")"))
				{
					scene* scene_ptr = game_instance->get_scene(scene_name);
					if (scene_ptr)
					{
						raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
					}
				}
				else
				{
					has_subbed = false;
				}
			}
		}

		//								   HANDLES ARGUMENT-LESS INNATE GETTERS
		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($)." + scene_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::vector<std::string> args;

				scene* scene_ptr = game_instance->get_scene(scene_name);
				if (scene_ptr)
				{
					raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
				}
			}
		}


		if (this_entity)
		{
			has_subbed = true;
			while (has_subbed)
			{
				has_subbed = string_utils.complex_replacement(raw_value, "scene()." + scene_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false);
				if (has_subbed)
				{
					std::vector<std::string> args = extract_args_from_token(wildcards[0], variable_names, variable_values, game_instance);
					if (!(args.size() == 1 && args[0] == ")"))
					{
						scene* scene_ptr = this_entity->get_scene();
						raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
					}
					else
					{
						has_subbed = false;
					}
				}
			}

			has_subbed = true;
			while (has_subbed)
			{
				has_subbed = string_utils.complex_replacement(raw_value, "scene()." + scene_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
				if (has_subbed)
				{
					std::vector<std::string> args;
					scene* scene_ptr = this_entity->get_scene();
					raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
				}
			}
		}


		//										 HANDLES SELF FUNCTION CALLS
		else if (this_scene)
		{

			has_subbed = true;
			while (has_subbed)
			{
				has_subbed = string_utils.complex_replacement(raw_value, "scene()."+scene_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false);
				if (has_subbed)
				{
					std::vector<std::string> args = extract_args_from_token(wildcards[0], variable_names, variable_values, game_instance);
					if (!(args.size() == 1 && args[0] == ")"))
					{
						scene* scene_ptr = this_scene;
						raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
					}
					else
					{
						has_subbed = false;
					}
				}
			}

			has_subbed = true;
			while (has_subbed)
			{
				has_subbed = string_utils.complex_replacement(raw_value, "scene()."+scene_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
				if (has_subbed)
				{
					std::vector<std::string> args;
					scene* scene_ptr = this_scene;
					raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
				}
			}


			has_subbed = true;
			while (has_subbed)
			{
				has_subbed = string_utils.complex_replacement(raw_value, scene_script_func_name + "($)", prestring, poststring, wildcards, ".() ", false);
				if (has_subbed)
				{
					std::vector<std::string> args = extract_args_from_token(wildcards[0], variable_names, variable_values, game_instance);
					if (!(args.size() == 1 && args[0] == ")"))
					{
						scene* scene_ptr = this_scene;
						raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
					}
					else
					{
						has_subbed = false;
					}
				}
			}

			has_subbed = true;
			while (has_subbed)
			{
				has_subbed = string_utils.complex_replacement(raw_value, scene_script_func_name + "()", prestring, poststring, wildcards, ".() ", false);
				if (has_subbed)
				{
					std::vector<std::string> args;
					scene* scene_ptr = this_scene;
					raw_value = prestring + func(game_instance, scene_ptr, args) + poststring;
				}
			}
		}

	};
	scene* this_scene = dynamic_cast<scene*>(const_cast<res_file*>(this));

	scene_getter_handler scene_get_name_handler = [](game* game_instance, scene* scene_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			std::string all_args;
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (i != 0)
					all_args += ", ";
				all_args += args[i];
			}
			return "INVALID ARGS; EXPECTED 0, GOT " + std::to_string(args.size()) + "(" + all_args + ")";
		}
		else
		{
			return scene_ptr->get_name();
		}
	};

	scene_getter_handler scene_get_children_handler = [](game* game_instance, scene* scene_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 0)
		{
			std::string all_args;
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (i != 0)
					all_args += ", ";
				all_args += args[i];
			}
			return "INVALID ARGS; EXPECTED 0, GOT " + std::to_string(args.size()) + "(" + all_args + ")";
		}
		else
		{
			std::string arr = "{";
			const std::vector<entity*>& children = scene_ptr->get_entities_in_scene();
			for (size_t i = 0; i < children.size(); ++i)
			{
				entity* child = children[i];
				const std::string& name = child->get_name();
				if (i != 0)
					arr += ",";
				arr += name;
			}
			arr += "}";
			return arr;
		}
	};

	scene_getter_handler scene_entity_exists_here_handler = [](game* game_instance, scene* scene_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR 'entity_exists_here'; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			bool exists = game_instance->get_entity_in_scene(args[0], scene_ptr->get_name(), false) != nullptr;

			if (exists)
			{
				return "true";
			}
			else
			{
				return "false";
			}
		}
	};

	scene_getter_handler scene_entity_exists_handler = [](game* game_instance, scene* scene_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR 'entity_exists'; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			bool exists = game_instance->get_entity_by_name(args[0]) != nullptr;

			if (exists)
			{
				return "true";
			}
			else
			{
				return "false";
			}
		}
	};

	scene_getter_handler scene_get_global_val_handler = [](game* game_instance, scene* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR get_global_value; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			return game_instance->get_value(args[0]);
		}
	};

	scene_getter_handler scene_get_meta_val_handler = [](game* game_instance, scene* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR get_meta_value; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			return game_instance->get_meta_value(args[0]);
		}
	};

	scene_getter_handler scene_get_value_handler = [](game* game_instance, scene* char_ptr, std::vector<std::string>& args) -> std::string
	{
		if (args.size() != 1)
		{
			return "INVALID ARGS FOR get_value; EXPECTED 1, GOT " + args.size();
		}
		else
		{
			return char_ptr->get_value(args[0]);
		}
	};


	register_scene_getter("get_name", scene_get_name_handler, this_scene, this_entity);
	register_scene_getter("get_children", scene_get_children_handler, this_scene, this_entity);
	register_scene_getter("entity_exists_here", scene_entity_exists_here_handler, this_scene, this_entity);
	register_scene_getter("entity_exists", scene_entity_exists_handler, this_scene, this_entity);
	register_scene_getter("get_global_value", scene_entity_exists_handler, this_scene, this_entity);
	register_scene_getter("get_meta_value", scene_entity_exists_handler, this_scene, this_entity);
	register_scene_getter("get_value", scene_get_value_handler, this_scene, this_entity);

	{
		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).$($)", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
				scene* scene_ptr = game_instance->get_scene(scene_name);
				if (scene_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_SCENE" + poststring;
				}
				else
				{
					std::string return_val;
					scene_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, "scene($).$()", prestring, poststring, wildcards, ".() ", false);
			if (has_subbed)
			{
				std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string func_name = wildcards[1];
				std::vector<std::string> args;

				scene* scene_ptr = game_instance->get_scene(scene_name);
				if (scene_ptr == nullptr)
				{
					raw_value = prestring + "NO_SUCH_SCENE" + poststring;
				}
				else
				{
					std::string return_val;
					scene_ptr->call_function(game_instance, func_name, args, return_val);
					raw_value = prestring + return_val + poststring;
				}
			}
		}

	}
	//                           THIS HANDLES CALLS TO USER-FUNCTIONS IN ENTITIES BY SCENE

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene($).entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[2];
			std::vector<std::string> args = extract_args_from_token(wildcards[3], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene($).entity($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[2];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene($).first_entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[2];
			std::vector<std::string> args = extract_args_from_token(wildcards[3], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene($).first_entity($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[2];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene($).any_entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[2];
			std::vector<std::string> args = extract_args_from_token(wildcards[3], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene($).any_entity($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			std::string scene_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string char_name = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[2];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}


	//							THIS ALLOWS FOR USER FUNCTIONS TO BE CALLED WITHIN ENTITIES IN THE CURRENT SCENE

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene().entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene().entity($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene().first_entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene().first_entity($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene().any_entity ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "scene().any_entity($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}


	//DEBUG_BREAKPOINT(5);

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "entity_here ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "entity_here($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_entity_in_scene(char_name, scene_name, false);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "first_entity_here ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "first_entity_here($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_first_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "any_entity_here ( $ ) . $ ( $ )", prestring, poststring, wildcards, ".() ", false, true);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args = extract_args_from_token(wildcards[2], variable_names, variable_values, game_instance);
			entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "any_entity_here($).$()", prestring, poststring, wildcards, ".() ", false);
		if (has_subbed)
		{
			const scene* this_scene = dynamic_cast<const scene*>(this);
			const entity* this_entity = dynamic_cast<const entity*>(this);
			std::string scene_name = "";
			if (this_scene)
			{
				scene_name = this_scene->get_name();
			}
			else if (this_entity)
			{
				scene_name = this_entity->get_scene_name();
			}
			std::string char_name = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string func_name = wildcards[1];
			std::vector<std::string> args;

			entity* char_ptr = game_instance->get_any_entity_in_scene(char_name, scene_name);
			if (char_ptr == nullptr)
			{
				raw_value = prestring + "NO_SUCH_CHARACTER" + poststring;
			}
			else if (char_ptr == game_instance->get_engine()->AMBIGUOUS_CHARACTER())
			{
				raw_value = prestring + "AMBIGUOUS_CHARACTER" + poststring;
			}
			else
			{
				std::string return_val;
				char_ptr->call_function(game_instance, func_name, args, return_val);
				raw_value = prestring + return_val + poststring;
			}
		}
	}

	//							THIS HANDLES SIZE() FUNCTION THAT GETS THE SIZE OF AN ARRAY
	//DEBUG_BREAKPOINT(6);
	auto is_array_format = [&](const std::string& val) -> bool
	{
		if (val.size() > 2 && val[0] == '{' && val[val.size() - 1] == '}')
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	has_subbed = true;
	while (has_subbed)
	{
		//DEBUG_BREAKPOINT(4);
		//std::cout << "RAW = " << raw_value << std::endl;
		has_subbed = string_utils.complex_replacement(raw_value, "size($)", prestring, poststring, wildcards, "() ", false, true);
		//std::cout << raw_value << " SUBBED = " << has_subbed << std::endl;
		//DEBUG_BREAKPOINT(5);
		if (has_subbed)
		{
			std::string arr = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			size_t existing_index;
			for (existing_index = 0; existing_index < variable_names.size(); ++existing_index)
			{
				//std::cout << "INDEX = " << existing_index << ", VAR NAME='" << variable_names[existing_index] << "'" << std::endl;
				if (variable_names[existing_index] == arr)
					break;
			}
			if (existing_index == variable_names.size())
			{
				//std::cout << existing_index << std::endl;
				if (is_array_format(arr))
				{
					size_t size = 0;
					std::string whole_arr = arr;
					if (is_array_format(whole_arr))
					{
						whole_arr = whole_arr.substr(1);
						whole_arr.resize(whole_arr.size() - 1);
						const std::vector<std::string>& elements = string_utils.extract_tokens(whole_arr, ",");
						for (size_t i = 0; i < elements.size(); ++i)
						{
							if (elements[i] != ",")
								++size;
						}
						raw_value = prestring + std::to_string(size) + poststring;
					}
					else if (whole_arr == "{}" || whole_arr == "{ }")
					{
						raw_value = prestring + "0" + poststring;
					}
					else
					{
						raw_value = prestring + std::to_string(whole_arr.size()) + poststring;
					}
				}
				else if (arr == "{}" || arr == "{ }")
				{
					raw_value = prestring + "0" + poststring;
				}
				else
					raw_value = prestring + "NO_SUCH_VAR" + poststring;
			}
			else
			{
				size_t size = 0;
				std::string whole_arr = variable_values[existing_index];
				if (is_array_format(whole_arr))
				{
					whole_arr = whole_arr.substr(1);
					whole_arr.resize(whole_arr.size() - 1);
					const std::vector<std::string>& elements = string_utils.extract_tokens(whole_arr, ",");
					for (size_t i = 0; i < elements.size(); ++i)
					{
						if (elements[i] != ",")
							++size;
					}
					raw_value = prestring + std::to_string(size) + poststring;
				}
				else if (whole_arr == "{}" || whole_arr == "{ }")
				{
					raw_value = prestring + "0" + poststring;
				}
				else
				{
					raw_value = prestring + std::to_string(whole_arr.size()) + poststring;
				}
			}
		}
	}

	//DEBUG_BREAKPOINT(7);
	//							THIS HANDLES STRING MANIPULATION
	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "concat( $ , $ )", prestring, poststring, wildcards, "(), ", false, true);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			raw_value = prestring + left + right + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "to_uppercase( $ )", prestring, poststring, wildcards, "(), ", false, true);
		if (has_subbed)
		{
			std::string str = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			string_utils.make_uppercase(str);
			raw_value = prestring + str + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "to_lowercase( $ )", prestring, poststring, wildcards, "(), ", false, true);
		if (has_subbed)
		{
			std::string str = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			string_utils.make_lowercase(str);
			raw_value = prestring + str + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "substr( $ , $ , $ )", prestring, poststring, wildcards, "(), ", false, true);
		if (has_subbed)
		{
			std::string start_str = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string length_str = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string string = resolve_expression(wildcards[2], variable_names, variable_values, game_instance);
			if (string_utils.is_integer(start_str) && string_utils.is_integer(length_str))
			{
				int start = std::stoi(start_str);
				int length = std::stoi(length_str);
				int last_index = start + length - 1;
				if (last_index >= string.size())
				{
					int characters_past_end = (last_index - string.size()) + 1;
					length -= characters_past_end;
				}
				raw_value = prestring + string.substr(start, length) + poststring;
			}
			else
			{
				raw_value = prestring + "ERR" + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "substr( $ , $ )", prestring, poststring, wildcards, "(), ", false, true);
		if (has_subbed)
		{
			std::string start = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string string = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			if(string_utils.is_integer(start))
			{
				raw_value = prestring + string.substr(std::stoi(start)) + poststring;
			}
			else
			{
				raw_value = prestring + "ERR" + poststring;
			}
		}
	}

	//DEBUG_BREAKPOINT(8);
	//							THIS HANDLES TYPE CHECKING
	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "is_int($)", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string val = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (string_utils.is_integer(val))
			{
				raw_value = prestring + "true" + poststring;
			}
			else
			{
				raw_value = prestring + "false" + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "is_decimal($)", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string val = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (string_utils.is_decimal(val))
			{
				raw_value = prestring + "true" + poststring;
			}
			else
			{
				raw_value = prestring + "false" + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "is_number($)", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string val = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (string_utils.is_numeric(val))
			{
				raw_value = prestring + "true" + poststring;
			}
			else
			{
				raw_value = prestring + "false" + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "is_array($)", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string val = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (is_array_format(val))
			{
				raw_value = prestring + "true" + poststring;
			}
			else
			{
				raw_value = prestring + "false" + poststring;
			}
		}
	}


	//							THIS HANDLES TYPE CASTING
	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "int($)", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string val = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (string_utils.is_decimal(val))
			{
				double num = std::stod(val);
				int casted_val = static_cast<int>(num);

				raw_value = prestring + std::to_string(casted_val) + poststring;
			}
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "decimal($)", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string val = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (string_utils.is_integer(val))
			{
				raw_value = prestring + val+".0" + poststring;
			}
		}
	}



	//							THIS HANDLES LANGUAGE SUBSTITUTION

	auto register_language_output_substitution_type = [&](const std::string& name)
	{
		has_subbed = true;
		while (has_subbed)
		{
			//std::cout << "RAW = " << raw_value << std::endl;
			has_subbed = string_utils.complex_replacement(raw_value, name+"_substitution ( $ , $ )", prestring, poststring, wildcards, ".(), ", false, true);
			//std::cout << raw_value << " SUBBED = " << has_subbed << std::endl;
			if (has_subbed)
			{
				std::string strength = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				std::string sentence = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				int percent_chance = 0;
				if(string_utils.is_integer(strength))
				{
					percent_chance = std::stoi(strength);
					sentence = game_instance->get_engine()->output_substitution(name, sentence, percent_chance);
					raw_value = prestring + sentence + poststring;
				}
				else
				{

				}
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, name+"_substitution($)", prestring, poststring, wildcards, ".() ", false, true);
			if (has_subbed)
			{
				std::string sentence = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
				sentence = game_instance->get_engine()->output_substitution(name, sentence, 40);
				raw_value = prestring + sentence + poststring;
			}
		}
	};

	register_language_output_substitution_type("generic");
	register_language_output_substitution_type("casual");
	register_language_output_substitution_type("formal");
	register_language_output_substitution_type("technical");
	register_language_output_substitution_type("medieval");

	/////////////////////////////////////////////////////////////////////////////////

	//				THIS HANDLES RESOLUTION TECHNIQUES OF SUBCLASSES, ALLOWING THEM TO DEFINE THEIR OWN INNATE GETTERS

	raw_value = resolve_expression_custom(raw_value, variable_names, variable_values, game_instance);

	/////////////////////////////////////////////////////////////////////////////////

	//										THIS HANDLES CALLS TO FUNCTIONS IN THE SAME FILE
											//DEBUG_BREAKPOINT(9);
	for (size_t i = 0; i < scripted_functions.size(); ++i)
	{
		const std::string& scripted_func_name = scripted_functions[i];

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, scripted_func_name + "()", prestring, poststring, wildcards, "() ", false, true);
			if (has_subbed)
			{
				//const auto& func_name_tokens = string_utils.extract_tokens(wildcards[0], " ");
				//const std::string func_name = scripted_funfunc_name_tokens[func_name_tokens.size() - 1];
				std::vector<std::string> args;
				std::string return_val;
				const std::string& err = call_function(game_instance, scripted_func_name, args, return_val);
				if (err == "")
					raw_value = prestring + return_val + poststring;
				else
					raw_value = prestring + err + poststring;
			}
		}

		has_subbed = true;
		while (has_subbed)
		{
			has_subbed = string_utils.complex_replacement(raw_value, scripted_func_name+"($)", prestring, poststring, wildcards, "() ", false, true);
			if (has_subbed)
			{
				//const auto& func_name_tokens = string_utils.extract_tokens(wildcards[0], " ");
				//const std::string func_name = wildcards[0];// func_name_tokens[func_name_tokens.size() - 1];
				//std::cout << "WAEHSRG$HRN " << raw_value << std::endl;
				const std::string& args_string = wildcards[0];
				std::vector<std::string> args = extract_args_from_token(args_string, variable_names, variable_values, game_instance);
				if (!(args.size() == 1 && args[0][0] == '('))
				{
					std::string return_val;
					const std::string& err = call_function(game_instance, scripted_func_name, args, return_val);
					if (err == "")
						raw_value = prestring + return_val + poststring;
					else
						raw_value = prestring + err + poststring;
				}
				else
				{
					has_subbed = false;
				}
			}
		}
	}


	//					THIS HANDLES RNG

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "random ( $ , $ )", prestring, poststring, wildcards, ",() ", false, true);
		if (has_subbed)
		{
			std::string min_value = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			if (string_utils.is_integer(min_value))
			{
				std::string max_value = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
				if (string_utils.is_integer(max_value))
				{
					int min = std::stoi(min_value);
					int max = std::stoi(max_value);
					if (max < min)
						std::swap(min, max);

					if (min == max)
						raw_value = prestring + min_value + poststring;
					else
					{
						int range = (max - min) + 1;
						int num = rand() % range;
						num += min;
						raw_value = prestring + std::to_string(num) + poststring;
					}

				}
			}
		}
	}

	//							USED TO DO SUB EXPRESSIONS RIGHT HERE

	/*THIS HANDLES SUB-EXPRESSIONS */
	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "( $ )", prestring, poststring, wildcards, "() ", false, true);
		if (has_subbed)
		{
			std::string sub_expression = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			raw_value = prestring + sub_expression + poststring;
		}
	}

	/*THIS HANDLES BOOLEAN OPERATORS*/
										//DEBUG_BREAKPOINT(10);
	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ > $", prestring, poststring, wildcards, " >", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			if (string_utils.is_numeric(left) && string_utils.is_numeric(right))
			{
				int L = std::stoi(left);
				int R = std::stoi(right);

				std::string resolution;
				if (L > R)
					resolution = "true";
				else
					resolution = "false";
				
				raw_value = prestring + resolution + poststring;
			}
			else
				break;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ < $", prestring, poststring, wildcards, " <", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			if (string_utils.is_numeric(left) && string_utils.is_numeric(right))
			{
				int L = std::stoi(left);
				int R = std::stoi(right);

				std::string resolution;
				if (L < R)
					resolution = "true";
				else
					resolution = "false";

				raw_value = prestring + resolution + poststring;
			}
			else
				break;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ >= $", prestring, poststring, wildcards, " >=", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			if(string_utils.is_numeric(left) && string_utils.is_numeric(right))
			{
				int L = std::stoi(left);
				int R = std::stoi(right);

				std::string resolution;
				if (L >= R)
					resolution = "true";
				else
					resolution = "false";

				raw_value = prestring + resolution + poststring;
			}
			else
				break;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ <= $", prestring, poststring, wildcards, " <=", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			if(string_utils.is_numeric(left) && string_utils.is_numeric(right))
			{
				int L = std::stoi(left);
				int R = std::stoi(right);

				std::string resolution;
				if (L <= R)
					resolution = "true";
				else
					resolution = "false";

				raw_value = prestring + resolution + poststring;
			}
			else
				break;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ == $", prestring, poststring, wildcards, " =", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string equality;
			if (left == right)
				equality = "true";
			else
				equality = "false";
			raw_value = prestring + equality + poststring;
		}
		else
			break;
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ != $", prestring, poststring, wildcards, " !=", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);
			std::string equality;
			if (left != right)
				equality = "true";
			else
				equality = "false";
			raw_value = prestring + equality + poststring;
		}
		else
			break;
	}

	//							HANDLES MATHEMATICAL SUBSTITUTION

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ % $", prestring, poststring, wildcards, " %", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);

			bool left_is_int = false;
			bool left_is_decimal = false;
			bool right_is_int = false;
			bool right_is_decimal = false;
			long long int left_int, right_int;
			long double left_decimal, right_decimal;

			if (string_utils.is_integer(left))
			{
				left_int = std::stoi(left);
				left_is_int = true;
			}
			else if (string_utils.is_decimal(left))
			{
				left_decimal = std::stod(left);
				left_is_decimal = true;
			}
			else
			{
				break;
			}

			if (string_utils.is_integer(right))
			{
				right_int = std::stoi(right);
				right_is_int = true;
			}
			else if (string_utils.is_decimal(right))
			{
				right_decimal = std::stod(right);
				right_is_decimal = true;
			}
			else
			{
				break;
			}

			std::string sum;

			if (left_is_int && right_is_int)
			{
				sum = std::to_string(left_int % right_int);
			}
			else
			{
				sum = left + " % " + right;
			}

			raw_value = prestring + sum + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ * $", prestring, poststring, wildcards, " *", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);

			bool left_is_int = false;
			bool left_is_decimal = false;
			bool right_is_int = false;
			bool right_is_decimal = false;
			long long int left_int, right_int;
			long double left_decimal, right_decimal;

			if (string_utils.is_integer(left))
			{
				left_int = std::stoi(left);
				left_is_int = true;
			}
			else if (string_utils.is_decimal(left))
			{
				left_decimal = std::stod(left);
				left_is_decimal = true;
			}
			else
			{
				break;
			}

			if (string_utils.is_integer(right))
			{
				right_int = std::stoi(right);
				right_is_int = true;
			}
			else if (string_utils.is_decimal(right))
			{
				right_decimal = std::stod(right);
				right_is_decimal = true;
			}
			else
			{
				break;
			}

			std::string sum;

			if (left_is_int && right_is_int)
			{
				sum = std::to_string(left_int * right_int);
			}
			else if (left_is_decimal && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal * right_decimal));
			}
			else if (left_is_int && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_int * right_decimal));
			}
			else if (left_is_decimal && right_is_int)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal * right_int));
			}

			raw_value = prestring + sum + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ / $", prestring, poststring, wildcards, " /", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);

			bool left_is_int = false;
			bool left_is_decimal = false;
			bool right_is_int = false;
			bool right_is_decimal = false;
			long long int left_int, right_int;
			long double left_decimal, right_decimal;

			if (string_utils.is_integer(left))
			{
				left_int = std::stoi(left);
				left_is_int = true;
			}
			else if (string_utils.is_decimal(left))
			{
				left_decimal = std::stod(left);
				left_is_decimal = true;
			}
			else
			{
				break;
			}

			if (string_utils.is_integer(right))
			{
				right_int = std::stoi(right);
				right_is_int = true;
			}
			else if (string_utils.is_decimal(right))
			{
				right_decimal = std::stod(right);
				right_is_decimal = true;
			}
			else
			{
				break;
			}

			std::string sum;

			if (left_is_int && right_is_int)
			{
				sum = string_utils.shorten_decimal(std::to_string(static_cast<long double>(left_int) / static_cast<long double>(right_int)));
			}
			else if (left_is_decimal && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal / right_decimal));
			}
			else if (left_is_int && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(static_cast<long double>(left_int) / right_decimal));
			}
			else if (left_is_decimal && right_is_int)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal / static_cast<long double>(right_int)));
			}

			raw_value = prestring + sum + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ + $", prestring, poststring, wildcards, " +", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);

			bool left_is_int = false;
			bool left_is_decimal = false;
			bool right_is_int = false;
			bool right_is_decimal = false;
			long long int left_int, right_int;
			long double left_decimal, right_decimal;

			if (string_utils.is_integer(left))
			{
				left_int = std::stoi(left);
				left_is_int = true;
			}
			else if (string_utils.is_decimal(left))
			{
				left_decimal = std::stod(left);
				left_is_decimal = true;
			}
			else
			{
				break;
			}

			if (string_utils.is_integer(right))
			{
				right_int = std::stoi(right);
				right_is_int = true;
			}
			else if (string_utils.is_decimal(right))
			{
				right_decimal = std::stod(right);
				right_is_decimal = true;
			}
			else
			{
				break;
			}

			std::string sum;

			if (left_is_int && right_is_int)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_int + right_int));
			}
			else if (left_is_decimal && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal + right_decimal));
			}
			else if (left_is_int && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_int + right_decimal));
			}
			else if (left_is_decimal && right_is_int)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal + right_int));
			}

			raw_value = prestring + sum + poststring;
		}
	}

	has_subbed = true;
	while (has_subbed)
	{
		has_subbed = string_utils.complex_replacement(raw_value, "$ - $", prestring, poststring, wildcards, " -", false);
		if (has_subbed)
		{
			std::string left = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
			std::string right = resolve_expression(wildcards[1], variable_names, variable_values, game_instance);

			bool left_is_int = false;
			bool left_is_decimal = false;
			bool right_is_int = false;
			bool right_is_decimal = false;
			long long int left_int, right_int;
			long double left_decimal, right_decimal;

			if (string_utils.is_integer(left))
			{
				left_int = std::stoi(left);
				left_is_int = true;
			}
			else if (string_utils.is_decimal(left))
			{
				left_decimal = std::stod(left);
				left_is_decimal = true;
			}
			else
			{
				break;
			}

			if (string_utils.is_integer(right))
			{
				right_int = std::stoi(right);
				right_is_int = true;
			}
			else if (string_utils.is_decimal(right))
			{
				right_decimal = std::stod(right);
				right_is_decimal = true;
			}
			else
			{
				break;
			}

			std::string sum;

			if (left_is_int && right_is_int)
			{
				sum = std::to_string(left_int - right_int);
			}
			else if (left_is_decimal && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal - right_decimal));
			}
			else if (left_is_int && right_is_decimal)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_int - right_decimal));
			}
			else if (left_is_decimal && right_is_int)
			{
				sum = string_utils.shorten_decimal(std::to_string(left_decimal - right_int));
			}

			raw_value = prestring + sum + poststring;
		}
	}
	//DEBUG_BREAKPOINT(11);
	//							HANDLES BOOLEAN INVERSION
	string_utils.strip(raw_value);
	while (string_utils.contains(raw_value, "! true", false) || string_utils.contains(raw_value, "! false", false))
	{
		raw_value = string_utils.replace_all(raw_value, "! true", "false", false);
		raw_value = string_utils.replace_all(raw_value, "! false", "true", false);
	}

	while (string_utils.contains(raw_value, "!true", false) || string_utils.contains(raw_value, "!false", false))
	{
		raw_value = string_utils.replace_all(raw_value, "!true", "false", false);
		raw_value = string_utils.replace_all(raw_value, "!false", "true", false);
	}

	while (string_utils.contains(raw_value, "! 0", false) || string_utils.contains(raw_value, "! 1", false))
	{
		raw_value = string_utils.replace_all(raw_value, "! 1", "false", false);
		raw_value = string_utils.replace_all(raw_value, "! 0", "true", false);
	}

	while (string_utils.contains(raw_value, "!1", false) || string_utils.contains(raw_value, "!0", false))
	{
		raw_value = string_utils.replace_all(raw_value, "!1", "false", false);
		raw_value = string_utils.replace_all(raw_value, "!0", "true", false);
	}
	//DEBUG_BREAKPOINT(12);
	return raw_value;
}

std::string	res_file::resolve_expression_custom(std::string raw_value, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance) const
{
	//Meant to be overridden by subclasses
	return raw_value;
}

std::string res_file::substitute_variables(const std::string& original, const std::vector<std::string>& variable_names, const std::vector<std::string>& variable_values, game* game_instance)
{
	string_utils string_utils;
	 //Untypable character used by array variables to pair keys with values hash-map style.
	const static char element_delimeter = ',';


	std::string delimeter = "0";
	std::string pair_delimeter = "0";
	delimeter[0] = element_delimeter;
	pair_delimeter[0] = pair_delimeter_character;

	auto is_array_format = [&](const std::string& val) -> bool
	{
		if (val.size() > 2 && val[0] == '{' && val[val.size() - 1] == '}')
		{
			return true;
		}
		else
		{
			return false;
		}
	};
	
	auto get_val_from_array = [&](const std::string& whole_array, const std::string& index) -> std::string
	{
		std::string arr = whole_array.substr(1);
		arr.resize(arr.size() - 1);
		string_utils.strip(arr);

		const std::vector<std::string>& elements = string_utils.extract_tokens(arr, delimeter);
		size_t delimeters_found = 0;
		for (size_t i = 0; i < elements.size(); ++i)
		{
			const std::string& element = elements[i];
			const std::vector<std::string>& pair = string_utils.extract_tokens(element, pair_delimeter);
			if (pair[0] != ",")
			{
				std::string current_index;
				std::string current_value;
				if (pair.size() == 1)
				{
					current_index = std::to_string(i - delimeters_found);
					current_value = pair[0];
				}
				else if (pair.size() == 3)
				{
					current_index = pair[0];
					current_value = pair[2];
				}
				else
				{
					std::cout << "A grave error has been encountered in res_file.cpp on line " << __LINE__ << std::endl;
					for (size_t i = 0; i < pair.size(); ++i)
					{
						std::cout << pair[i] << std::endl;
					}
				}
				//std::cout << "ARRAY = " << arr << "; INDEX = " << index << "; CURRENT_INDEX = " << current_index << "; CURRENT VALUE = " << current_value << std::endl;
				if (current_index == index)
				{
					return current_value;
				}
			}
			else
			{
				++delimeters_found;
			}
		}
		return "NULL";
	};

	std::string base = " " + original + " ";
	std::vector<std::string> wildcards;
	std::string prestring, poststring;
	for (size_t i = 0; i < variable_names.size(); ++i)
	{
		const std::string& var_name = variable_names[i];
		const std::string& var_val = variable_values[i];
		
		if (is_array_format(var_val))
		{
			bool subbed = true;
			while (subbed)
			{
				subbed = string_utils.complex_replacement(base, var_name + "[ $ ]", prestring, poststring, wildcards, " []", false, true);
				if (subbed)
				{
					std::string index = resolve_expression(wildcards[0], variable_names, variable_values, game_instance);
					base = prestring + get_val_from_array(var_val, index) + poststring;
				}
				else
				{
					string_utils.replace_all(base, " " + var_name + " ", var_val, false);
				}
			}

			base = string_utils.replace_all(base, " " + var_name + " ", var_val, false);
		}
		else
		{
			base = string_utils.replace_all(base, " " + var_name + " ", var_val, false);
		}
	}
	string_utils.strip(base);
	return base;
}