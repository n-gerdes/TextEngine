#include "engine/headers/instance/dummy.h"

dummy::dummy()
{

}

dummy::dummy(const std::string& dummy_name)
{
	set_name(dummy_name);
}

void dummy::save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const
{

}

void dummy::load_variables(std::ifstream& file, const std::string& scenario_name, engine* engine)
{

}