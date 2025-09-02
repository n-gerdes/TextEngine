#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H
#include "engine/headers/instance/game_obj.h"
#include "engine/headers/instance/res_file.h"
#include "engine/headers/procedure/adjudicator.h"
#include <string>
#include <map>
class room;
class game;
class scene;
class entity : public adjudicator, public res_file
{
	GAME_OBJ_SUBCLASS(entity, 3)

public:
	typedef int32_t hp_t;
private:
	mutable std::mutex waiting_for_pc_mutex;
	std::string current_scene = "NULL";
	std::string current_command = "NULL";
	std::string last_command_return_value = "done";
	mutable scene* scene_ptr = nullptr;
	bool interrupted = false;
	bool waiting_for_pc = false;
	bool has_never_entered_scene = true;
	bool is_in_transfer_queue = false;
	std::string name_of_interrupter = "NULL";
	std::string attached_to = "NULL";
	int32_t hp = 10;
	int32_t max_hp = 10;
	uint64_t command_repeats = 0;
	uint64_t current_turn = 0;
	std::list<std::string> attached_entity_names;
	std::string narrative_queue = ""; //At the start of each turn, it prints the queue to the screen for the perspective character before resetting it, and simply discards it for non-perspective entities.
	std::map<std::string, std::map<std::string, std::vector<std::string>>> known_aliases; //This hashmap pairs true names with maps that pair alias types to vectors of known aliases for other entities.
	std::mutex known_aliases_mutex;
	mutable std::mutex hp_mutex;
	mutable std::mutex max_hp_mutex;
public:
	bool							in_transfer_queue();
	void							set_in_transfer_queue(bool val);
	void							attach(entity* follower);
	void							attach_to(entity* entity_to_attach_to);
	virtual std::string				call_innate_function(game* game_instance, const std::string& function_name, std::vector<std::string>& args) override;
	void							copy_data_from(entity* c);
	void							damage(game* game_instance, game_obj* source, hp_t amount);
	void							damage(game* game_instance, hp_t amount);
protected:
	virtual void					execute_line_from_class(game* game_instance, line_num& line, std::string& err_msg, std::vector<std::string>& wildcards, std::vector<std::string>& variable_names, std::vector<std::string>& variable_values) override;
public:
	void							forget_aliases_for(const std::string& other_entity_name);
	const std::list<std::string>&	get_attached_entity_names() const;
	const std::string&				get_attachment_name() const;
	uint64_t						get_consecutive_command_repeats() const;
	virtual std::string				get_display_name(bool randomize, bool allow_titles, const std::vector<std::string>& known_names) const override;
	virtual std::string				get_display_name(bool randomize, bool allow_titles) const override;
	std::string						get_display_name_of_other_entity(bool randomize, bool allow_titles, const std::string& category, entity* other_entity) const;
	std::string						get_display_name_of_other_entity(bool randomize, bool allow_titles, entity* other_entity) const;
private:
	game*							get_game_instance() const;
public:
	int32_t							get_hp() const;
	int32_t							get_max_hp() const;
	const std::string&				get_interrupter() const;
	const std::string&				get_last_command() const;
	const std::string&				get_scene_name() const;
	scene*							get_scene() const;
	uint64_t						get_turn_number() const;
	bool							idle() const;
	void							interrupt();
	bool							knows_alias(const std::string& potential_alias, const entity* other_entity) const;
protected:
	virtual void					on_destroyed();
public:
	void							learn_alias_for_entity(const std::string& entity_true_name, const std::string& alias_category, const std::string& alias);
private:
	template <typename T>
	void							pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int number_of_args);

	template <typename T>
	void							pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int min_args, int max_args);
public:
	virtual void					process_line_from_file(const std::string& line) override;
	void							print(game* game_instance, const std::string& text);
	void							println(game* game_instance, const std::string& text);
	//void							refresh_ai_options(game* game_instance);
	//void							set_ai_controller(const std::string& ai_controller);
	hp_t							recover(hp_t amount);
	hp_t							recover(game_obj* source, hp_t amount);
	void							remove_from_scene();
	virtual bool					resolve_input(game* game_instance, entity* user, const std::string& input, std::string& return_val);

	scene*							set_to_scene(const std::string& scene_name);
	void							set_turn_number(uint64_t turn_num);
	hp_t							set_hp(hp_t new_hp);
	void							set_max_hp(hp_t new_max_hp);
	bool							take_turn(game* game_instance, std::string& reason_for_failure);
	void							unattach();
	void							unattach(const std::string& follower);
	bool							was_interrupted() const;
	void							wipe_aliases(game* game_instance);
};
#endif