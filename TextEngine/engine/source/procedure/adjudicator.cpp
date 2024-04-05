#include "engine/headers/procedure/adjudicator.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/util/string_utils.h"

adjudicator::adjudicator()
{

}

bool adjudicator::resolve_input(game* game_instance, entity* user, const std::string& input, std::string& return_val)
{
	//Meant to be overridden by subclasses.
	return false;
}

/*
void adjudicator::expose_input(entity* npc, const std::string& possible_input, uint32_t weight)
{
	npc->add_ai_option(possible_input, weight);
}

void adjudicator::register_npc_option(const std::string& ai_type, const std::string& option, uint32_t weight)
{
	if (npc_command_registry.find(ai_type) == npc_command_registry.end())
		npc_command_registry.insert_or_assign(ai_type, std::map<std::string, uint32_t>());

	npc_command_registry.at(ai_type).insert_or_assign(option, weight);
}

void adjudicator::give_entity_scripted_options(game* game_instance, entity* entity)
{
	if (npc_command_registry.size() > 0)
	{
		string_utils string_utils;
		std::string entity_ai = string_utils.replace_all(entity->get_ai_controller(), " ", "");
		auto entity_ai_tokens = string_utils.extract_tokens(entity_ai, ".");
		if (entity_ai_tokens.size() > 0)
		{
			std::vector<std::string> ai_hierarchy;
			ai_hierarchy.reserve(entity_ai_tokens.size());
			for (size_t i = 0; i < entity_ai_tokens.size(); ++i)
			{
				if (entity_ai_tokens[i] != ".")
					ai_hierarchy.push_back(entity_ai_tokens[i]);
			}
			for (size_t i = ai_hierarchy.size() - 1; i != 0; --i)
			{
				std::string previous_tokens = "";
				for (size_t j = 0; j < i; ++j)
				{
					previous_tokens += ai_hierarchy[j] + ".";
				}
				ai_hierarchy[i] = previous_tokens + ai_hierarchy[i];
			}
			for (size_t ai_hierarchy_level = 0; ai_hierarchy_level < ai_hierarchy.size(); ++ai_hierarchy_level)
			{
				std::string& ai = ai_hierarchy[ai_hierarchy_level];
				std::map<std::string, uint32_t>& option_weight_pairs = npc_command_registry.at(ai);
				for (auto i = option_weight_pairs.begin(); i != option_weight_pairs.end(); ++i)
				{
					const std::string& option = i->first;
					uint32_t weight = i->second;
					entity->add_ai_option(option, weight);
				}
			}
		}
	}
}

void adjudicator::give_entity_custom_options(game* game_instance, entity* ent)
{

}
*/
void adjudicator::load_variables(std::ifstream& file, const std::string& scenario_name, engine* engine) {
	
}

void adjudicator::save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const {
	
}