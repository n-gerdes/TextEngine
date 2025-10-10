#ifndef GAME_H
#define GAME_H
#include "game_obj.h"
#include "engine/headers/procedure/adjudicator.h"
#include "engine/headers/procedure/substitution_wizard.h"
#include <map>

class entity;
class scene; //Just need this forward declaration so it can keep a pointer to it
class game : public adjudicator
{
	GAME_OBJ_SUBCLASS(game, 1)
private:
	engine* game_engine {nullptr};
	std::string perspective_entity = ""; //The player controls the actions of the perspective entity, and the game
	//narrates everything from the perspective entity's POV
	//The string value is the true name of the perspective entity, never an alias. True names are assumed to be unique.

	mutable entity* perspective_entity_ptr = nullptr; //This holds a pointer to the current perspective entity.

	uint32_t game_version = 0;

	bool game_going = true;

	mutable std::mutex name_to_entity_mutex;
	mutable std::map<std::string, entity*> name_to_entity;
	std::mutex meta_variable_mutex;


	substitution_wizard subs;
	
	bool clear_on_scene_change = false;
	bool save_any_time = true;
	bool custom_input_substitution_overrides_engine = false; //Determines whether input substitution from the scenario is cumulative with the engine's base substitution or mutually exclusive with it

public:
	void					set_input_substitution_override(bool val) { custom_input_substitution_overrides_engine = val; };
	bool					has_input_substitution_override() const { return custom_input_substitution_overrides_engine; };
	substitution_wizard*	get_substitution_wizard() { return &subs; };

	inline void				set_save_any_time(bool can_save_on_command) { save_any_time = can_save_on_command; };
	inline void				set_clear_on_scene_change(bool clears_when_scene_changes) { clear_on_scene_change = clears_when_scene_changes; };

	inline bool				get_save_any_time() const { return save_any_time; };
	inline bool				get_clear_on_scene_change() const {return clear_on_scene_change;};

	bool					entity_exists(const std::string& name);

							game(const std::string& scenario_name, engine* engine_instance) : game_engine(engine_instance) { set_name(scenario_name); initialize(); }
	bool					game_is_active() const;
	entity*					get_any_entity(std::string name, const std::string& source);
	entity*					get_any_entity_in_scene(std::string entity_name, const std::string& scene_name, const std::string& source);

	std::vector<entity*>	get_entities();

	entity*					get_entity(std::string name, bool allow_alias, const std::string& source);
	entity*					get_entity_in_scene(std::string entity_name, const std::string& scene_name, bool allow_alias, const std::string& source);

	entity*					get_first_entity(std::string name, const std::string& source);
	entity*					get_first_entity_in_scene(std::string entity_name, const std::string& room_name, const std::string& source);
	std::string				get_scenario_directory() const;
	//							NOW FOR VARIATIONS THAT GET ENTITIES FROM ALIASES PROVIDED BY ENTITIES

	entity*					get_any_entity(std::string alias, entity* seeker, const std::string& source);
	entity*					get_any_entity_in_scene(std::string alias, const std::string& scene_name, entity* seeker, const std::string& source);

	entity*					get_entity(std::string alias, entity* seeker, const std::string& source);
	entity*					get_entity_in_scene(std::string alias, const std::string& scene_name, entity* seeker, const std::string& source);

	entity*					get_entity_by_name(std::string name, const std::string& source);

	uint64_t				get_current_turn();

	entity*					get_first_entity(std::string alias, entity* seeker, const std::string& source);
	entity*					get_first_entity_in_scene(std::string alias, const std::string& room_name, entity* seeker, const std::string& source);
	

	void					game_loop();
	void					end_game();
	//std::vector<entity*> get_entities(const std::string& name) const;
	//std::vector<entity*> get_entities_in_current_scene(const std::string& name) const;
	//entity*				get_entity(const std::string& entity_true_name);
	scene*					get_current_scene();
	engine*					get_engine() const;

	void					describe_scene(scene* s);
private:
	std::string				get_meta_directory(const std::string& variable_name);
public:
	std::string				get_meta_value(const std::string& variable_name);
	scene*					get_scene(std::string name);
	
	entity*					get_perspective_entity();
	void					initialize();
	entity*					load_entity_from_file(const std::string& entity_name, const std::string& filename, const std::string& source);
	//entity*					load_entity_from_file(const std::string& entity_name, const std::string& filename);
	entity*					load_entity_from_file(const std::string& entity_name, const std::string& source, bool dummy);
	scene*					load_scene_from_file(const std::string& scene_name);
	scene*					load_scene_from_file(const std::string& scene_name, const std::string& filename);
	virtual bool			resolve_input(game* game_instance, entity* user, const std::string& input, std::string& return_val) override;
	void					save_game_to_file(); //This is what actually triggers a saving sequence.
	void					set_meta_value(const std::string& var_name, const std::string& var_value);
	void					set_perspective_entity(entity* pc);
};
#endif
