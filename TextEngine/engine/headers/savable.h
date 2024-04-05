#pragma once

#include<fstream>
#include<string>
#include <mutex>
#include "engine/headers/engine.h"

class savable
{
private:
	template<typename data_t> inline void save_raw(std::ofstream& file, data_t data) const
	{
		char* address = reinterpret_cast<char*>(&data);
		for (size_t bytes_written = 0; bytes_written < sizeof(data_t); ++bytes_written)
		{
			char raw_byte_data = address[bytes_written];
			file.put(raw_byte_data);
		}
	}

	template<typename data_t> inline data_t get_raw(std::ifstream& file)
	{
		data_t return_val;
		char* address = reinterpret_cast<char*>(&return_val);
		for (size_t bytes_written = 0; bytes_written < sizeof(data_t); ++bytes_written)
		{
			address[bytes_written] = file.get();
		}
		return return_val;
	}
public:
	
	inline void save_uint8_t(std::ofstream& file, uint8_t var) const { save_raw(file, var); };
	inline void save_uint_16_t(std::ofstream& file, uint16_t var) const { save_raw(file, var); };
	inline void save_uint32_t(std::ofstream& file, uint32_t var) const { save_raw(file, var); };
	inline void save_uint64_t(std::ofstream& file, uint64_t var) const { save_raw(file, var); };

	inline void save_char(std::ofstream& file, char var) const { save_raw(file, var); };
	inline void save_int8_t(std::ofstream& file, int8_t var) const { save_raw(file, var); };
	inline void save_int16_t(std::ofstream& file, int16_t var) const { save_raw(file, var); };
	inline void save_int32_t(std::ofstream& file, int32_t var) const { save_raw(file, var); };
	inline void save_int64_t(std::ofstream& file, int64_t var) const { save_raw(file, var); };

	inline void save_float(std::ofstream& file, float var) const { save_raw(file, var); };
	inline void save_double(std::ofstream& file, double var) const { save_raw(file, var); };
	inline void save_long_double(std::ofstream& file, long double var) const { save_raw(file, var); };
	inline void save_bool(std::ofstream& file, bool var) const { save_raw(file, var); };

	inline uint8_t load_uint8_t(std::ifstream& file, uint8_t& var) { return var = get_raw<uint8_t>(file); }
	inline uint16_t load_uint16_t(std::ifstream& file, uint16_t& var) { return var = get_raw<uint16_t>(file); }
	inline uint32_t load_uint32_t(std::ifstream& file, uint32_t& var) { return var = get_raw<uint32_t>(file); }
	inline uint64_t load_uint64_t(std::ifstream& file, uint64_t& var) { return var = get_raw<uint64_t>(file); }

	inline char load_char(std::ifstream& file, char& var) { return var = get_raw<char>(file); }
	inline int8_t load_int8_t(std::ifstream& file, int8_t& var) { return var = get_raw<int8_t>(file); }
	inline int16_t load_int16_t(std::ifstream& file, int16_t& var) { return var = get_raw<int16_t>(file); }
	inline int32_t load_int32_t(std::ifstream& file, int32_t& var) { return var = get_raw<int32_t>(file); }
	inline int64_t load_int64_t(std::ifstream& file, int64_t& var) { return var = get_raw<int64_t>(file); }

	inline float load_float(std::ifstream& file, float& var) { return var = get_raw<float>(file); }
	inline double load_double(std::ifstream& file, double& var) { return var = get_raw<double>(file); }
	inline long double load_long_double(std::ifstream& file, long double& var) { return var = get_raw<long double>(file); }
	inline bool load_bool(std::ifstream& file, bool& var) { return var = get_raw<bool>(file); }

	//inline void save_savable(std::ofstream& file, const savable& savable_instance) const;

	void save_string(std::ofstream& file, const std::string& string) const;
	void save_string_vector(std::ofstream& file, const std::vector<std::string>& vec) const;
	void save_uint32_t_vector(std::ofstream& file, const std::vector<uint32_t>& vec) const;
	std::string load_string(std::ifstream& file, std::string& string);
	void load_string_vector(std::ifstream& file, std::vector<std::string>& vec);
	void load_uint32_t_vector(std::ifstream& file, std::vector<uint32_t>& vec);

	//void load_savable(std::ifstream& file);

public:
	virtual void save_to_file(std::ofstream& file, const std::string& scenario_name, engine* engine) const {};

	virtual void load_from_file(std::ifstream& file, engine& game_engine, const std::string& scenario_name, engine* engine) {};
};
