#ifndef FILE_READER_H
#define FILE_READER_H

#include "engine/headers/MASTER.h"
#include <string>
#include <fstream>
#include <vector>
#include <map>

class internal_file_builder;

class file_reader
{
	friend class internal_file_builder;
private:
	void			reset();
	std::ifstream	fs_reader;
	std::string		internal_filename = "";
	std::vector<std::string>* internal_lines = nullptr;
	bool			reading_internal_file = false;
	int				internal_line = 0;
	static std::map < std::string, std::vector<std::string> > internal_files;
	static void		initialize_internal_file(const std::string& filename, size_t reserve);
	static std::vector<std::string>* get_internal_file_lines(const std::string& filename);

	bool has_internal_file(const std::string& filename);
public:
	bool		is_open();
	void		open(const std::string& filename);
	void		force_external_open(const std::string& filename);
	void		getline(std::string& destination);
	void		close();
	bool		good();
	bool		eof();

	static void clear_internal_files();
	static void bake();
};



#endif