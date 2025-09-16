#include "engine/headers/util/string_utils.h"
#include <iostream>
#include <mutex>
#include <map>

bool string_utils::contains(const std::string& full_string, const std::string& substring) const
{
	return full_string.find(substring) != std::string::npos;
}

bool string_utils::contains(const std::string& full_string, const std::string& substring, bool case_sensitive) const
{
	if (case_sensitive)
	{
		return full_string.find(substring) != std::string::npos;
	}
	else
	{
		return get_lowercase(full_string).find(get_lowercase(substring)) != std::string::npos;
	}
}

bool string_utils::complex_replacement(const std::string& original, std::string search_pattern, std::string& extracted_prestring, std::string& extracted_poststring, std::vector<std::string>& wildcards) const
{
	std::string& prestring = extracted_prestring = "";
	std::string& poststring = extracted_poststring = "";
	wildcards.clear();
	while (search_pattern.find("$$") != std::string::npos) //Any consecutive duplicate wildcards are redundant
		search_pattern = replace_first(search_pattern, "$$", "$");

	if (search_pattern == "$")
	{
		prestring = "";
		poststring = original;
		wildcards.push_back(original);
		return true;
	}

	const auto& search_pattern_tokens = extract_tokens(search_pattern, "$");
	
	size_t first_index = 0;
	size_t initial_offset = search_pattern_tokens[0].size();
	if (search_pattern_tokens[0] != "$")
	{
		first_index = original.find(search_pattern_tokens[0]);
	}
	else
	{ 
		if (original.find(search_pattern_tokens[1], 0) == 0 && original.find(search_pattern_tokens[1], 1)==std::string::npos)
		{
			prestring = "";
			poststring = original;
			return false;
		}
		else if (original.find(search_pattern_tokens[1]) == std::string::npos)
		{
			prestring = original;
			poststring = "";
			return false;
		}
		else
		{
			prestring = original.substr(0, original.find(search_pattern_tokens[1], 1));
			wildcards.push_back(prestring);
			first_index = prestring.size();
			prestring = "";
			initial_offset = 0;
		}
	}
	if (first_index == std::string::npos) //If I can scan the whole string for the very first token of the search pattern and not find it, then technically the whole thing is a "prestring"
	{
		prestring = original;
		poststring = ""; //And nothing is the post-string.
		return false;
	}
	else
	{
		size_t index = first_index + initial_offset;
		if(search_pattern_tokens[0] != "$")
			prestring = original.substr(0, first_index);
		size_t token = 1;
		while (index < original.size() && token < search_pattern_tokens.size()) //Scan thru 'till the end
		{
			if (search_pattern_tokens[token] != "$")
			{
				index = original.find(search_pattern_tokens[token], index) + search_pattern_tokens[token].size();
			}
			else
			{
				if (token == search_pattern_tokens.size() - 1) //If the last token is a wildcard
				{
					break;
				}
				else //If this token is a wildcard but not the last token
				{
					const std::string& next_token = search_pattern_tokens[token + 1]; //I need to know the next 
					size_t next_token_index = original.find(next_token, index);

					if (next_token_index == std::string::npos) //Needs to find the WHOLE search pattern to return true.
					{
						prestring = original;
						poststring = "";
						return false;
					}

					std::string wildcard = substring(original, index, next_token_index - 1);
					wildcards.push_back(wildcard);
				}
			}
			++token;
		}
		size_t post_index = index;
		if (post_index >= original.size())
		{
			poststring = "";
		}
		else
		{
			poststring = original.substr(post_index);
		}
		return true;
	}

}

bool string_utils::complex_replacement(const std::string& original, const std::string& search_pattern, std::string& extracted_prestring, std::string& extracted_poststring, std::vector<std::string>& wildcards, const std::string& delimeters, bool case_sensitive) const
{
	return complex_replacement(original, search_pattern, extracted_prestring, extracted_poststring, wildcards, delimeters, case_sensitive, false);
}
//'Prestring' is everything before the found pattern, 'Poststring' is everything after.
bool string_utils::complex_replacement(const std::string& original, const std::string& search_pattern, std::string& extracted_prestring, std::string& extracted_poststring, std::vector<std::string>& wildcards, const std::string& delimeters, bool case_sensitive, bool multi_token_wildcards) const
{
	std::string& prestring = extracted_prestring = "";
	std::string& poststring = extracted_poststring = "";
	wildcards.clear();

	bool measure_parans = delimeters.find("(") != std::string::npos && delimeters.find(")") && std::string::npos;
	bool measure_brackets = delimeters.find("[") != std::string::npos && delimeters.find("]") && std::string::npos;
	bool measure_commas = delimeters.find(",") != std::string::npos;
	int paran_level = -1;
	int bracket_level = -1;
	if (search_pattern == "$")
	{
		prestring = "";
		poststring = original;
		wildcards.push_back(original);
		return true;
	}
	/*
	if (search_pattern == "generic_substitution ( $ , $ )")
		std::cout << "YESYESYESYES: " << original << std::endl;
		*/
	const auto& original_tokens = extract_tokens(original, delimeters);
	const auto& search_pattern_tokens = extract_tokens(search_pattern, delimeters + "$");

	/*
	auto print_tokens = [](const std::vector<std::string>& tokens)
	{
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::cout << tokens[i] << std::endl;
		}
	};

	std::cout << "OG TOKENS:" << std::endl;
	print_tokens(original_tokens);
	std::cout << "\nPATTERN TOKENS:\n";
	print_tokens(search_pattern_tokens);
	*/

	if (original_tokens.size() < search_pattern_tokens.size())
	{
		prestring = original;
		return false;
	}

	auto compare_tokens = [&](const std::string& token1, const std::string& token2) -> bool
	{
		//token1 is the sample token, token2 is the search pattern token
		if (measure_parans && paran_level!=-1)
		{
			if (measure_commas)
				if (token1 == "," && token2 == ",")
					if (paran_level == 0 || paran_level == 1)
						return true;
					else
						return false;

			if (token1 == "(")
			{
				//std::cout << "; Incrementing paran level to " << (paran_level + 1) << "; ";
				++paran_level;
			}
			else if (token1 == ")")
			{
				//std::cout << "; Decrementing paran level to " << (paran_level - 1) << "; ";
				--paran_level;
				if (token2 == ")")
					if (paran_level == 0)
					{
						//std::cout << "true) A\n";
						return true;
					}
					else
					{
						//std::cout << "false) B\n";
						return false;
					}
			}
		}

		if (measure_brackets && bracket_level != -1)
		{

			if (token1 == "[")
			{
				//std::cout << "; Incrementing paran level to " << (paran_level + 1) << "; ";
				++bracket_level;
			}
			else if (token1 == "]")
			{
				//std::cout << "; Decrementing paran level to " << (paran_level - 1) << "; ";
				--bracket_level;
				if (token2 == "]")
					if (bracket_level == 0)
					{
						//std::cout << "true) A\n";
						return true;
					}
					else
					{
						//std::cout << "false) B\n";
						return false;
					}
			}
		}

		if (token2 == "$")
		{
			//std::cout << "true) C\n";
			return true;
		}
		if (case_sensitive)
		{
			//std::cout << (token1==token2) << ") D\n";
			return token1 == token2;
		}
		else
		{
			//std::cout << (get_lowercase(token1) == get_lowercase(token2)) << ") E\n";
			return get_lowercase(token1) == get_lowercase(token2);
		}
	};

	size_t first_matching_token_index = 0;
	bool found_first_match = false;
	for (size_t original_token_index = 0; original_token_index < original_tokens.size(); ++original_token_index) //Original tokens must be at LEAST as many as the search pattern for them to match
	{
		const std::string& original_token = original_tokens[original_token_index];
		const std::string& pattern_token = search_pattern_tokens[0];
		if (compare_tokens(original_token, pattern_token))
		{
			first_matching_token_index = original_token_index;
			found_first_match = true;
			break;
		}
		else
		{
			prestring += original_token + " ";
		}
	}
	paran_level = 0;
	bracket_level = 0;
	if (found_first_match)
	{
		//Now I know when they start to match
		if (multi_token_wildcards)
		{
			//std::cout << "FIRST MATCH, SEARCH PATTERN = " << search_pattern << std::endl;
			size_t pattern_token_index = 0;
			for (size_t original_token_index = first_matching_token_index; original_token_index < original_tokens.size(); ++original_token_index)
			{
				const std::string& original_token = original_tokens[original_token_index];

				if (pattern_token_index < search_pattern_tokens.size())
				{
					const std::string& pattern_token = search_pattern_tokens[pattern_token_index];
					if (compare_tokens(original_token, pattern_token))
					{
						if (pattern_token == "$")
						{
							if (pattern_token_index == search_pattern_tokens.size() - 1)
							{
								wildcards.push_back(original_token);
							}
							else
							{
								std::string current_wildcard = "";
								++pattern_token_index;
								const std::string& next_pattern_token = search_pattern_tokens[pattern_token_index];
								//if(search_pattern == "if ( $ ) then")
									//std::cout << "NEXT PATTERN TOKEN = " << next_pattern_token << std::endl;
								while (
									(original_token_index < original_tokens.size())
									&&
									( !compare_tokens(original_tokens[original_token_index], next_pattern_token))
									)
								{
									current_wildcard += original_tokens[original_token_index] + " ";
									++original_token_index;
									//if (search_pattern == "if ( $ ) then")
										//std::cout << "CURRENT WILDCARD = " << current_wildcard << std::endl;
								}
								
								if (original_token_index == original_tokens.size()) //This means it got all the way to the end of the original tokens before it reached the end of the search pattern, indicating that it only contains part of a search pattern instead of all of it and thus should return false.
									return false;
								else
									wildcards.push_back(current_wildcard);
							}
						}
						
						//else
							//std::cout << "MATCHED " << original_token << " AND " << pattern_token << std::endl;
						
						++pattern_token_index;
					}
					else
					{
						return false;
					}
				}
				else
				{
					poststring += original_token + " ";
				}
			}
			//if (search_pattern == "if ( $ ) then")
				//std::cout << "HERE2\n";
			
			//At this time I know that the original tokens have been exhausted, and that so far they've been a match.
			if (pattern_token_index < search_pattern_tokens.size()) //If there are still remaining tokens in the search pattern, it is not a match.
				return false;

			return true;
		}
		else
		{
			size_t pattern_token_index = 0;
			for (size_t original_token_index = first_matching_token_index; original_token_index < original_tokens.size(); ++original_token_index)
			{
				const std::string& original_token = original_tokens[original_token_index];

				if (pattern_token_index < search_pattern_tokens.size())
				{
					const std::string& pattern_token = search_pattern_tokens[pattern_token_index];
					if (compare_tokens(original_token, pattern_token))
					{
						if (pattern_token == "$")
						{
							wildcards.push_back(original_token);
						}

						++pattern_token_index;
					}
					else
					{
						return false;
					}
				}
				else
				{
					poststring += original_token + " ";
				}
			}
			return true;
		}
	}
	else
	{
		//if(multi_token_wildcards)
			//std::cout << "NO MATCH, SEARCH PATTERN = " << search_pattern << std::endl;
		return false;
	}
}

std::vector<std::string> string_utils::extract_tokens(const std::string& input, const std::string& delimeters) const
{
	std::vector<std::string> tokens;

	std::string token;
	tokens.reserve(10);
	auto push_token = [&]()
	{
		if (token.size() > 0)
		{
			tokens.push_back(token);
			token = "";
		}
	};

	auto char_is_delimeter = [&](char entity) -> bool
	{
		for (size_t i = 0; i < delimeters.size(); ++i)
			if (entity == delimeters[i])
				return true;
		return false;
	};

	for (size_t i = 0; i < input.size(); i++)
	{
		char entity = input.at(i);
		if (char_is_delimeter(entity))
		{
			push_token();
			if (entity != ' ') //Non-space delimeters get their own associated tokens.
			{
				token = " ";
				token[0] = entity;
				push_token();
			}
		}
		else
		{
			token += entity;
		}
	}
	push_token();
	return tokens;
}

std::vector<std::string> string_utils::extract_tokens(const std::string& input, char delimeter) const
{
	std::string del = "0";
	del[0] = delimeter;
	return extract_tokens(input, del);
}

std::vector<std::string> string_utils::extract_tokens(const std::string& input) const
{
	return extract_tokens(input, " ");
}

std::string string_utils::format_as_name(const std::string& input) const
{
	if (input.size() < 1)
		return input;

	std::string base = input;
	base[0] = std::toupper(base[0]);
	for (size_t i = 1; i < base.size(); ++i)
	{
		if (base[i - 1] == ' ')
			base[i] = std::toupper(base[i]);
		else
			base[i] = std::tolower(base[i]);
	}
	
	return base;
}

std::string string_utils::get_lowercase(const std::string& str) const
{
	std::string returned = str;
	for (size_t i = 0; i < returned.size(); ++i)
		returned[i] = std::tolower(returned[i]);
	return returned;
}

bool string_utils::is_command_char(char c) const
{
	if (c >= 0 && c <= 31 && c!=9)
		return true;
	if (c >= 127)
		return true;

	return false;
}

bool char_is_numeric(char c)
{
	switch (c)
	{
		case '0': return true;
		case '1': return true;
		case '2': return true;
		case '3': return true;
		case '4': return true;
		case '5': return true;
		case '6': return true;
		case '7': return true;
		case '8': return true;
		case '9': return true;
	}
	return false;
}

bool string_utils::is_decimal(const std::string& str) const
{
	std::string stripped = str;
	strip(stripped);
	size_t start = 0;
	if (stripped[0] == '-')
		start = 1;

	bool found_decimal = false;
	for (size_t i = start; i < stripped.size(); ++i)
	{
		char cur = stripped[i];
		if (cur == '.')
		{
			if (found_decimal)
				return false;
			else
				found_decimal = true;
		}
		else
		{
			if (cur != ' ' && !char_is_numeric(cur))
			{
				return false;
			}
		}
	}
	return true;
}

bool string_utils::is_integer(const std::string& str) const
{
	std::string stripped = str;
	strip(stripped);
	size_t start = 0;
	if (stripped[0] == '-')
		start = 1;
	for (size_t i = start; i < stripped.size(); ++i)
	{
		char cur = stripped[i];
		if ( cur != ' ' && !char_is_numeric(cur) )
		{
			return false;
		}
	}
	return true;
}

bool string_utils::is_numeric(const std::string& str) const
{
	return is_integer(str) || is_decimal(str);
}

void string_utils::make_lowercase(std::string& str) const
{
	for (size_t i = 0; i < str.size(); ++i)
		str[i] = std::tolower(str[i]);
}

void string_utils::make_uppercase(std::string& str) const
{
	for (size_t i = 0; i < str.size(); ++i)
		str[i] = std::toupper(str[i]);
}


bool string_utils::matches_command(const std::string& command, const std::string& input)
{
	std::vector<std::string> dummy;
	return matches_command(command, input, dummy);
}

bool string_utils::matches_command(const std::string& command, const std::string& input, std::vector<std::string>& variable_tokens) const
{
	return matches_command(command, input, variable_tokens, " ().,:;+*/-=");
}

bool string_utils::matches_command(const std::string& command, const std::string& input, std::vector<std::string>& variable_tokens, const std::string& delimeters) const
{
	variable_tokens.clear();
	string_utils string_utils;
	auto command_tokens = string_utils.extract_tokens(command, delimeters);
	auto input_tokens = string_utils.extract_tokens(input, delimeters);

	/*
	std::cout << "INPUT TOKENS: ";
	for (int i = 0; i < input_tokens.size(); ++i)
	{
		std::cout << input_tokens[i] << "|";
	}
	std::cout << std::endl;
	std::cout << "COMMAND TOKENS: ";
	for (int i = 0; i < command_tokens.size(); ++i)
	{
		std::cout << command_tokens[i] << "|";
	}
	std::cout << std::endl;
	*/
	size_t num_of_command_tokens = command_tokens.size();
	size_t num_of_input_tokens = input_tokens.size();

	if (num_of_input_tokens < num_of_command_tokens) //There must be at LEAST as many input tokens as command tokens
		return false; //Potentially more to account for wildcards

	size_t max_tokens = num_of_command_tokens;
	size_t min_tokens = num_of_input_tokens;

	int input_parenthises_level = 0;
	int input_brackets_level = 0;

	if (min_tokens > max_tokens)
		std::swap(max_tokens, min_tokens);

	int input_tokens_iterator = 0; //It iterates through the command tokens rather than the input tokens for the checks
	for (int command_tokens_iterator = 0; command_tokens_iterator < num_of_command_tokens; ++command_tokens_iterator)
	{
		if (input_tokens_iterator >= input_tokens.size())
			return false;
		std::string command_token = string_utils.get_lowercase(command_tokens[command_tokens_iterator]);
		std::string input_token = string_utils.get_lowercase(input_tokens[input_tokens_iterator]);
		//std::cout << "COMP: '" << input_token << "' / '" << command_token << "'" << std::endl;
		if (command_token != input_token && command_token[0] != '$') //If the tokens ever don't match and the command token isn't a wildcard, they don't match.
		{
			//std::cout << "\tMISMATCH: '" << input_token << "' / '" << command_token << "'" << std::endl;
			return false;
		}

		//At this point it's assumed they do match
		if (command_token[0] == '$') //If a command token is a wild card, it has to process that.
		{
			std::string potential_wildcard = "";
			int next_command_token_index = command_tokens_iterator + 1;
			if (next_command_token_index == num_of_command_tokens) //If the wildcard is the final token, it just has to add out the rest of the input tokens to the final wildcard.
			{
				while (input_tokens_iterator < num_of_input_tokens)
				{
					potential_wildcard += input_tokens[input_tokens_iterator];
					++input_tokens_iterator;
					if (input_tokens_iterator != num_of_input_tokens)
						potential_wildcard += " ";
				}
			}
			else
			{
				std::string& next_command_token = command_tokens[next_command_token_index];
				if (next_command_token[0] == '$')
					throw; //If I accidentally include two wildcards in a row, it must throw an error.
				bool includes_parenthises_in_delimeters = delimeters.find("(") != std::string::npos && delimeters.find(")") != std::string::npos;
				bool includes_brackets_in_delimeters = delimeters.find("[") != std::string::npos && delimeters.find("]") != std::string::npos;
				auto input_token_does_not_match_next_command_token = [&](const std::string& input_token) -> bool
					{
						if (includes_parenthises_in_delimeters)
						{
							if (input_token == "(")
							{
								++input_parenthises_level;
								return input_token != next_command_token;
							}
							else if (input_token == ")")
							{
								if (input_parenthises_level == 0)
								{
									return input_token != next_command_token;
								}
								else
								{
									--input_parenthises_level;
									return true;
								}
							}
						}

						if (includes_brackets_in_delimeters)
						{
							if (input_token == "[")
							{
								++input_brackets_level;
								return input_token != next_command_token;
							}
							else if (input_token == "]")
							{
								if (input_brackets_level == 0)
								{
									return input_token != next_command_token;
								}
								else
								{
									--input_brackets_level;
									return true;
								}
							}
						}

						return input_token != next_command_token;

					};

				/*So if it finds a wildcard and there's a clearly defined "next" command token (because this isn't the last
				token so there has to be), just keep adding tokens from the input to the potential wildcard until it reaches
				a matching command token or the end of the input tokens (if the end of the input tokens are reached then it
				means this didn't include every token from the command pattern, is incomplete, and should return false).
				*/

				if (!input_token_does_not_match_next_command_token(input_tokens[input_tokens_iterator])) //If the input wildcard is actually the same as the next command token it gets confused, so this special check was added 9/15/2025 to fix that.
				{
					++input_tokens_iterator;
					potential_wildcard = input_tokens[input_tokens_iterator];
				}
				else
				{
					while (input_tokens_iterator < num_of_input_tokens && input_token_does_not_match_next_command_token(input_tokens[input_tokens_iterator]))
					{
						if (potential_wildcard.size() > 0)
							potential_wildcard += " ";
						potential_wildcard += input_tokens[input_tokens_iterator];
						++input_tokens_iterator;
					}
				}

				

				if (input_tokens_iterator == num_of_input_tokens) //If it got all the way to the end without finding a match for the next command token, they don't match.
				{
					return false;
				}
				else
				{
					--input_tokens_iterator; //It will be set forward again at the end, so it needs to be rewound to prevent it from going too far
				}
			}

			variable_tokens.push_back(potential_wildcard);
			input_parenthises_level = 0;
			input_brackets_level = 0;
		}
		++input_tokens_iterator;
	}

	return true; //If it passed all checks, it means the command is a match.
}

std::string string_utils::replace_all(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only) const
{
	if (whole_word_only)
	{
		std::string returned = " " + original + " ";
		std::string substring = " " + original_substring + " ";
		size_t index = returned.find(substring);
		while (index != std::string::npos)
		{
			std::string pre = returned.substr(0, index);
			std::string post = "";
			if (index + substring.length() < returned.length())
				post = returned.substr(index + substring.length());

			returned = pre + " " + new_substring + " " + post;

			index = returned.find(substring);
		}
		returned.resize(returned.size() - 1);
		returned = returned.substr(1);
		return returned;
	}
	else
	{
		std::string returned = original;
		size_t index = returned.find(original_substring);
		while (index != std::string::npos)
		{
			std::string pre = returned.substr(0, index);
			std::string post = "";
			if (index + original_substring.length() < returned.length())
				post = returned.substr(index + original_substring.length());

			returned = pre + new_substring + post;

			index = returned.find(original_substring);
		}
		return returned;
	}
}

std::string string_utils::replace_all(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only, bool case_sensitive) const
{
	if (case_sensitive)
	{
		return replace_all(original, original_substring, new_substring, whole_word_only);
	}
	else
	{
		std::string cur = original;
		if (whole_word_only)
			cur = " " + cur + " ";
		std::string lowercase_original_substring = get_lowercase(original_substring);
		if (whole_word_only)
			lowercase_original_substring = " " + lowercase_original_substring + " ";

		if (whole_word_only)
		{
			while (get_lowercase(cur).find(lowercase_original_substring) != std::string::npos)
			{
				cur = replace_first(cur, " " + original_substring + " ", " " + new_substring + " ", false);
			}


			cur = cur.substr(1);
			cur.resize(cur.size() - 1);
		}
		else
		{
			while (get_lowercase(cur).find(lowercase_original_substring) != std::string::npos)
				cur = replace_first(cur, original_substring, new_substring, false);
		}
		
		return cur;
	}
}

std::string string_utils::replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring) const
{
	size_t index = original.find(original_substring);
	if (index != std::string::npos)
	{
		std::string pre = original.substr(0, index);
		std::string post = "";
		if(index + original_substring.length() < original.length())
			post = original.substr(index + original_substring.length());

		return pre + new_substring + post;
	}
	else
	{
		return original;
	}
}

std::string string_utils::replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool case_sensitive) const
{
	if (case_sensitive)
	{
		return replace_first(original, original_substring, new_substring);
	}
	else
	{
		std::string lowercase_original = get_lowercase(original);
		std::string lowercase_original_substring = get_lowercase(original_substring);
		size_t index = lowercase_original.find(lowercase_original_substring);

		if (index != std::string::npos)
		{
			std::string prestring = original.substr(0, index);
			std::string post = "";
			if (index + original_substring.length() < original.length())
				post = original.substr(index + original_substring.length());

			return prestring + new_substring + post;
		}
		else
		{
			return original;
		}
	}
}

std::string string_utils::replace_from_offset(const std::string& original, const std::string& original_substring, const std::string& new_substring, size_t offset) const
{
	size_t index = original.find(original_substring, offset);
	if (index != std::string::npos)
	{
		std::string pre = original.substr(0, index);
		std::string post = "";
		if (index + original_substring.length() < original.length())
			post = original.substr(index + original_substring.length());

		return pre + new_substring + post;
	}
	else
	{
		return original;
	}
}

std::string string_utils::replace_from_offset(const std::string& original, const std::string& original_substring, const std::string& new_substring, size_t offset, bool case_sensitive) const
{
	if (case_sensitive)
	{
		return replace_from_offset(original, original_substring, new_substring, offset);
	}
	else
	{
		std::string lowercase_original = get_lowercase(original);
		std::string lowercase_original_substring = get_lowercase(original_substring);
		size_t index = lowercase_original.find(lowercase_original_substring, offset);

		if (index != std::string::npos)
		{
			std::string prestring = original.substr(0, index);
			std::string post = "";
			if (index + original_substring.length() < original.length())
				post = original.substr(index + original_substring.length());

			return prestring + new_substring + post;
		}
		else
		{
			return original;
		}
	}
}

std::string string_utils::shorten_decimal(std::string decimal)
{
	if (decimal.find(".") != std::string::npos)
	{
		size_t chars_to_remove = 0;
		size_t index = decimal.size() - 1;

		while (index != 0 && decimal[index] == '0')
		{
			++chars_to_remove;
			--index;
		}
		if (decimal[index] == '.')
			++chars_to_remove;
		decimal.resize(decimal.size() - chars_to_remove);
		return decimal;
	}
	else
	{
		return decimal;
	}
}

bool string_utils::starts_with(const std::string& text, const std::string& potential_beginning, bool case_sensitive) const
{
	if (case_sensitive)
	{
		size_t index = text.find(potential_beginning);
		if (index == 0)
			return true;
		else
			return false;
	}
	else
	{
		std::string lowercase = get_lowercase(text);
		std::string lowercase_start = get_lowercase(potential_beginning);
		size_t index = lowercase.find(lowercase_start);
		if (index == 0)
			return true;
		else
			return false;
	}
}

void string_utils::strip(std::string& str) const //This function strips extra spaces at the beginning and end of a string, and any duplicate spaces in between
{
	str = replace_all(str, "\t", "", false); //Remove all tabs.
	size_t new_start = 0;
	while (new_start < str.size() && str[new_start] == ' ')
		++new_start;

	if (new_start == str.size())
	{
		str = "";
	}
	else
	{
		size_t new_end = str.size() - 1;
		while (str[new_end] == ' ' && new_end > new_start)
			--new_end;

		size_t new_length = new_end - new_start + 1;

		str = str.substr(new_start, new_length); //This is the first pass - just removing leading and trailing spaces.

		size_t duplicate_index;
		do
		{
			duplicate_index = str.find("  ", 0);
			if (duplicate_index != std::string::npos) //If it finds duplicate spaces
			{
				str = str.substr(0, duplicate_index) + str.substr(duplicate_index + 1, str.size() - duplicate_index - 1);
			}
			else
			{
				break;
			}
		} while (true);
	}
}

std::string string_utils::substring(const std::string& str, size_t first_index, size_t second_index) const
{
	if (first_index > second_index)
		std::swap(first_index, second_index);
	return str.substr(first_index, (second_index - first_index) + 1);
}

bool string_utils::ends_with(const std::string& str, const std::string& tail) const
{
	if (tail.size() > str.size())
		return false;

	int str_iterator = str.size() - 1;
	for (int tail_iterator = tail.size() - 1; tail_iterator != -1; --tail_iterator)
	{
		char str_c = str[str_iterator];
		char tail_c = tail[tail_iterator];
		if (str_c != tail_c)
			return false;
		--str_iterator;
	}
	return true;
}
