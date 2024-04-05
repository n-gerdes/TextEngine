#ifndef ADJUDICATOR_H
#define ADJUDICATOR_H
#include <string>
#include "engine/headers/instance/game_obj.h"
class game;
class entity;
class adjudicator : public game_obj
{
	GAME_OBJ_SUBCLASS(adjudicator, 0)

	//std::map<std::string, std::map<std::string, uint32_t>> npc_command_registry; //Pairs AI settings with maps that pair commands to weights
public:
	virtual bool	resolve_input(game* game_instance, entity* user, const std::string& input, std::string& return_val); //This resolves input received from an entity in the same scene.
	//void			expose_input(entity* npc, const std::string& possible_input, uint32_t weight); //This exposes potential inputs to CPU-controlled entities.
	//void			register_npc_option(const std::string& ai_type, const std::string& option, uint32_t weight);
	//void			give_entity_scripted_options(game* game_instance, entity* entity_in_scene); //Used to give entities options provided by the registered options, registered via register_npc_option
	//Options registered under less specific AI settings are give first, but weights are adjusted using the more specific settings if necessary
	//virtual void	give_entity_custom_options(game* game_instance, entity* entity_in_scene);
};

#endif