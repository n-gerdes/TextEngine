#ifndef SCENE_H
#define SCENE_H
#include "game_obj.h"
#include "../engine.h"
#include "engine/headers/util/string_utils.h"
#include "engine/headers/instance/game.h"
#include "engine/headers/instance/entity.h"
#include "engine/headers/instance/res_file.h"
#include "engine/headers/procedure/adjudicator.h"
#include <vector>
#include <mutex>

/*
This class provides the framework for defining different types of scenes. The general gameplay loop works like this:
1) Text is displayed
2) Player enters text
3) The scene processes that input in accordance with the type of scene it is (i.e., an exploration scene provides detailed
descriptions about the environment and clues about where to go, combat resolves data for each fighter and narrates the output
before allowing the player to choose their next action, etc).
4) Depending on what happens in the scene, the state may change and new actions become possible, or old ones cease to be options.
5) Text is displayed again regarding the current state of the scene, thus restarting the loop.
This is resolved by the game by calling game_loop_iteration on the scene currently loaded. game_loop_iteration first displays
text based on the current state of the scene. Then it gets input from the user and passes it to process_input, which is where the
meat of the code is for each subclass.
Each scene is essentially a state machine with the ability to switch to other state machines, while preserving some information
about the current context (i.e., the player entity).

Intended scene types:
Room (exploration mode)
Encounter (combat mode; definitely needs to know which room you were in when combat started so it can integrate that into combat
descriptions).
Scripted (story mode, can be used for dialog; may offer discrete choices or simply advance whenever the player hits 'enter').
*/

namespace scene_friend_funcs
{
	void game_loop(game* game_instance, scene* this_scene, int* threads_launched);
};


class scene : public adjudicator, public res_file
{
	GAME_OBJ_SUBCLASS(scene, 2)

private:
	
	bool						thread_active = false;
	std::vector<entity*>		transfer_queue;
	std::mutex					queuetex;
	bool						loaded_from_file = false;
	bool						has_read_description_after_loading_from_file = false;
	bool						transferred_perspective_character = false;
public:	
	friend						void scene_friend_funcs::game_loop(game* game_instance, scene* this_scene, int* threads_launched);
	virtual std::string			call_innate_function(game* game_instance, const std::string& function_name, std::vector<std::string>& args) override;
	void						display_text();
	entity*						get_entity(game* game_instance, const std::string& name, bool allow_alias) const;
	entity*						get_first_entity(game* game_instance, const std::string& name) const;
	const std::vector<entity*>	get_entities_in_scene() const;
	void						launch(game* game_instance, int* threads_launched);
	void						load_transfer_entities(game* game_instance);
protected:
	virtual void				on_destroyed();
	
	template <typename T>
	void						pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int number_of_args);

	template <typename T>
	void						pair_innate_function(T internal_func, const std::string& checked_func_name, const std::string& innate_function_name, std::vector<std::string>& args, game* game_instance, std::string& err, int min_args, int max_args);
public:
	virtual void				process_line_from_file(const std::string& line) override;
	virtual bool				resolve_input(game* game_instance, entity* user, const std::string& input, std::string& return_val);
	void						queue_transfer(entity* ent);
};

#endif