#include "engine/headers/procedure/substitution_wizard.h"
#include "engine/headers/MASTER.h"
#include "engine/headers/util/string_utils.h"
#include "engine/headers/procedure/file_reader.h"
#include <fstream>
#include <iostream>

void substitution_wizard::apply_input_substitution(std::string& modified) const
{
	if (has_loaded_input_substitution)
	{
		string_utils string_utils;
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
	}
}

bool substitution_wizard::load_output_substitution_file(const std::string& directory, const std::string& modifier)
{
	string_utils string_utils;
	std::string filename; 
	if (directory.size() == 0)
		return false;
	if(directory[directory.size()-1]=='/')
		filename = directory + modifier + "_output_substitution.dat";
	else
		filename = directory + "/" + modifier + "_output_substitution.dat";

	//std::ifstream substitution_file;
	file_reader substitution_file;
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
			//std::getline(substitution_file, line);
			substitution_file.getline(line);
			string_utils.strip(line);
			auto tokens = string_utils.extract_tokens(line, ":,");
			if (tokens.size() < 3)
			{
				std::cout << "ERROR: Line " << line_num << " in " << filename << " is malformed." << std::endl;
				known_thes.insert_or_assign(modifier, false);
				return false;
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
						std::cout << "ERROR: Line " << line_num << " in " << filename << " is malformed.";
						known_thes.insert_or_assign(modifier, false);
						return false;
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
					std::cout << "ERROR: Line " << line_num << " in " << filename << " is malformed." << std::endl;
					known_thes.insert_or_assign(modifier, false);
					return false;
				}
			}
		}
		substitution_file.close();
	}
	else
	{
		//std::cout << "ERROR: Could not open " << filename << std::endl;
		known_thes.insert_or_assign(modifier, false);
		return false;
	}
	known_thes.insert_or_assign(modifier, true);
	return true;
}

void substitution_wizard::load_input_substitution(const std::string& filename)
{
	//std::ifstream substitution_file;
	file_reader substitution_file;
	substitution_file.open(filename);
	string_utils string_utils;
	if (substitution_file.is_open())
	{
		std::string line;
		while (substitution_file.good() && !substitution_file.eof())
		{
			//std::getline(substitution_file, line);
			substitution_file.getline(line);
			string_utils.strip(line);
			if (string_utils.starts_with(line, "remove)", true) && !string_utils.contains(line, ":"))
			{
				if (line == "remove)")
				{
					std::cout << "ERROR: " << filename << " incorrectly formatted : " << line << "; no string specified for removal." << std::endl;
					throw;
				}
				else
					input_removal.push_back(line.substr(7));
			}
			else if (string_utils.starts_with(line, "remove_raw)", true) && !string_utils.contains(line, ":"))
			{
				if (line == "remove_raw)")
				{
					std::cout << "ERROR: " << filename << " incorrectly formatted : " << line << "; no string specified for removal." << std::endl;
					throw;
				}
				
				else
					raw_input_removal.push_back(line.substr(11));
			}
			else
			{
				auto tokens = string_utils.extract_tokens(line, ":");
				if (tokens.size() != 3)
				{
					std::cout << "ERROR: " << filename << " incorrectly formatted : " << line << "; found " << tokens.size() << " tokens, expected 3" << std::endl;
					throw;
				}
				else
				{
					input_substitution.insert_or_assign(tokens[2], tokens[0]);
				}
			}
		}
		substitution_file.close();
		has_loaded_input_substitution = true;
	}
	else
	{
		std::cout << "ERROR: Could not open " << filename << std::endl;
		throw;
	}
}


std::string substitution_wizard::output_substitution(const std::string& thesuarus, std::string sentence, int replacement_percent_chance)
{
	if (replacement_percent_chance < 1 || thesuarus.size()==0)
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
			if (replacement_roll <= replacement_percent_chance)
				new_word = synonyms[rand() % synonyms.size()];

			sentence = string_utils.replace_from_offset(sentence, source_word, new_word, location, false);
			offset = location + 1;
			location = lowercase_sentence.find(source_word, offset);
		}
	}
	return sentence;
}

bool substitution_wizard::has_thesaurus(const std::string& thesaurus, const std::string& directory)
{
	if (thesaurus.size() == 0)
		return false;
	auto known = known_thes.find(thesaurus);
	if (known == known_thes.end())
	{
		return load_output_substitution_file(directory, thesaurus);
	}
	else
	{
		return known->second;
	}
}