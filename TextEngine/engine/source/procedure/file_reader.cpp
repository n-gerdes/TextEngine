#include "engine/headers/procedure/file_reader.h"
#include "engine/headers/engine.h"
#include <iostream>
#include "engine/headers/procedure/internal_file_builder.h"

std::map < std::string, std::vector<std::string> > file_reader::internal_files;

void file_reader::initialize_internal_file(const std::string& filename, size_t reserve)
{
	if(internal_files.find(filename) == internal_files.end())
		internal_files.emplace(filename, std::vector<std::string>());
	internal_files[filename].reserve(reserve);
}

std::vector<std::string>* file_reader::get_internal_file_lines(const std::string& filename)
{
	auto found = internal_files.find(filename);
	if (found == internal_files.end())
		return nullptr;
	else
		return &(internal_files[filename]);
}

bool file_reader::has_internal_file(const std::string& filename)
{
	return internal_files.find(filename) != internal_files.end();
}

bool file_reader::is_open()
{
	return fs_reader.is_open() || reading_internal_file;
}

void file_reader::reset()
{
	fs_reader.close();
	reading_internal_file = false;
	internal_filename = "";
	internal_line = 0;
	internal_lines = nullptr;
}

void file_reader::open(const std::string& filename)
{
	if (!is_open())
	{
		internal_lines = get_internal_file_lines(filename);
		if (internal_lines)
		{
			internal_line = 0;
			internal_filename = filename;
			reading_internal_file = true;
		}
		else if(!engine::forces_baked_scenario())
		{
			internal_line = 0;
			internal_filename = "";
			reading_internal_file = false;
			fs_reader.open(filename);
		}
	}
}

void file_reader::force_external_open(const std::string& filename)
{
	internal_line = 0;
	internal_filename = "";
	reading_internal_file = false;
	fs_reader.open(filename);
}

void file_reader::close()
{
	reset();
}

bool file_reader::good()
{
	if (fs_reader.is_open())
	{
		return fs_reader.good();
	}
	else
	{
		return !eof();
	}
}

void file_reader::getline(std::string& destination)
{
	if (fs_reader.is_open())
	{
		std::getline(fs_reader, destination);
		if (destination[destination.size() - 1] == 13)
		{
			destination.resize(destination.size() - 1);
		}
	}
	else if (internal_lines && reading_internal_file)
	{
		std::vector<std::string>& v = *internal_lines;
		destination = v[internal_line];
		if (destination[destination.size() - 1] == 13)
		{
			destination.resize(destination.size() - 1);
		}
		++internal_line;
	}
	else
	{
		destination = "";
	}
}

bool file_reader::eof()
{
	if (fs_reader.is_open())
	{
		return fs_reader.eof();
	}
	else if(internal_lines && reading_internal_file)
	{
		return internal_line >= internal_lines->size();
	}
	else
	{
		return true;
	}
}

void file_reader::clear_internal_files()
{
	internal_files.clear();
}

void file_reader::bake()
{
	internal_file_builder::build_internal_files();
}
