#ifndef INTERNAL_FILE_READER_H
#define INTERNAL_FILE_READER_H
#include "engine/headers/procedure/file_reader.h"
class internal_file_builder
{
	friend class file_reader;
	static void	build_internal_files();
};

#endif