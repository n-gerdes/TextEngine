#include "engine/headers/savable.h"
#include <iostream>

/*
void savable::save_savable(std::ofstream& file, const savable& other_savable) const
{
	other_savable.save_to_file(file);
}

void savable::load_savable(std::ifstream& file)
{

}
*/

void savable::save_string(std::ofstream& file, const std::string& string) const
{
	uint64_t str_len = static_cast<uint64_t>(string.length());
	//save_uint64_t(file, str_len);
	for (uint64_t i = 0; i < str_len; ++i)
	{
		char character = string[i];
		save_char(file, character);
	}
	save_char(file, 0);
}

void savable::save_string_vector(std::ofstream& file, const std::vector<std::string>& vec) const
{
	uint64_t vec_len = static_cast<uint64_t>(vec.size());
	for (uint64_t i = 0; i < vec_len; ++i)
	{
		save_string(file, vec[i]);
	}
}

void savable::save_uint32_t_vector(std::ofstream& file, const std::vector<uint32_t>& vec) const
{
	uint64_t vec_len = static_cast<uint64_t>(vec.size());
	for (uint64_t i = 0; i < vec_len; ++i)
	{
		save_uint32_t(file, vec[i]);
	}
}

std::string savable::load_string(std::ifstream& file, std::string& string)
{
	string = "";
	uint64_t str_len = 0;
	//load_uint64_t(file, str_len);
	char loaded_char;
	load_char(file, loaded_char);
	while (loaded_char != 0)
	{
		string.push_back(loaded_char);
		load_char(file, loaded_char);
	}
	/*
	for (uint64_t i = 0; i < str_len; ++i)
	{
		char loaded_char = '0';
		load_char(file, loaded_char);
		string += loaded_char;
	}
	*/
	return string;
}

void savable::load_string_vector(std::ifstream& file, std::vector<std::string>& vec)
{
	vec.clear();
	uint64_t size = 0;
	std::string loaded_string;
	load_uint64_t(file, size);
	vec.reserve(size);
	for (uint64_t i = 0; i < size; ++i)
	{
		load_string(file, loaded_string);
		vec.push_back(loaded_string);
	}
}

void savable::load_uint32_t_vector(std::ifstream& file, std::vector<uint32_t>& vec)
{
	vec.clear();
	uint64_t size = 0;
	uint32_t loaded_int = 0;
	load_uint64_t(file, size);
	vec.reserve(size);
	for (uint64_t i = 0; i < size; ++i)
	{
		load_uint32_t(file, loaded_int);
		vec.push_back(loaded_int);
	}
}