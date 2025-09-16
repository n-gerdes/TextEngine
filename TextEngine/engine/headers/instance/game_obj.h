#pragma once

#include <list>

#include <vector>

#include <string>
#include <map>
#include <iostream>
#include <mutex>
#include "../savable.h"

class game_obj_save_registry;
class game;
class game_obj : public savable
{
private:
	typedef game_obj parent_t;
	typedef game_obj child_t;
	typedef std::list<child_t*> game_obj_storage_t;
	game_obj_storage_t children;

	parent_t* parent{ nullptr };
	std::string game_obj_name{ "game_obj" };
	std::vector<std::string> aliases;
	std::map<std::string, std::string> variables;
	bool was_instanced_from_file = false;
	friend class game_obj_save_registry;
	mutable std::mutex save_mutex;
	mutable std::mutex var_mutex;
	mutable std::mutex alias_mutex;
public:

	child_t*					add_child(child_t* new_child);
	void						add_alias(const std::string& new_alias);
	void						add_title(const std::string& new_title);
	void						copy_values_and_aliases_from(game_obj* o);
	void						copy_values_from(game_obj* o);
	virtual void				child_added(child_t* new_child) {}
	virtual void				child_removed(child_t* removed_child) {}
	void						delete_aliases();
	void						destroy();
	game_obj*					detach();

	std::vector<game_obj*>		find_children(game* game_instance, const std::string& child_name) const;

	template <class T>
	std::vector<T*>				find_children(game* game_instance, const std::string& child_name) const
	{
		std::vector<T*> list;
		list.reserve(children.size());
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			T* child = dynamic_cast<T*>(*i);
			if (child && (child->get_name() == child_name || child->has_alias(game_instance, child_name)))
				list.push_back(child);
		}
		return list;
	}

	template <class T>
	T*							find_first_child(game* game_instance, const std::string& child_name) const
	{
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			T* child = dynamic_cast<T*>(*i);
			if (child && (child->get_name() == child_name || child->has_alias(game_instance, child_name)))
				return child;
		}
		return nullptr;
	}

	game_obj*					find_first_child(game* game_instance, const std::string& child_name) const;

	template <class T>
	T* find_first_child(game* game_instance, const std::string& child_name, bool can_use_true_name, bool can_use_alias) const
	{
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			T* child = dynamic_cast<T*>(*i);
			if (child && ((can_use_true_name && child->get_name() == child_name) || (can_use_alias && child->has_alias(game_instance, child_name))))
				return child;
		}
		return nullptr;
	}

	game_obj* find_first_child(game* game_instance, const std::string& child_name, bool can_use_true_name, bool can_use_alias) const;

protected:
								game_obj(const std::string& name) : game_obj_name(name){}
								game_obj() {} //Subclasses MUST define!
	virtual						~game_obj();
public:
	game_obj*					get(game* game_instance, const std::string& path); //Will get a given game resource by true name, not an alias.
	const game_obj_storage_t&	get_children() const;
	virtual std::string			get_display_name(bool randomize, bool allow_titles, const std::vector<std::string>& known_names) const;
	virtual std::string			get_display_name(bool randomize, bool allow_titles) const;
	virtual uint32_t			get_save_id() const;
	const std::string&			get_name() const;
	parent_t*					get_parent() const;
	std::string					get_value(const std::string& variable_name) const;
	bool						has_alias(game* game_instance, const std::string& potential_alias) const;
	virtual game_obj*			instance() const;
	void						load_children(std::ifstream& file, engine& game_engine, const std::string& scenario_name, engine* engine);
	void						load_from_file(std::ifstream& file, engine& game_engine, const std::string& scenario_name, engine* engine) override;
protected:
	virtual void				load_variables(std::ifstream& file, const std::string& name, engine* engine) {} //Subclasses MUST define!
	virtual void				on_added(parent_t* new_parent) {}
	virtual void				on_removed(parent_t* old_parent) {}
	virtual void				on_destroyed() {}
public:
	void						print_hierarchy() const;
private:
	void						print_hierarchy(int layer) const;
public:
	child_t*					remove(child_t* current_child);
	void						save_children(std::ofstream& file, const std::string& scenario_name, engine* engine) const;
	void						save_to_file(std::ofstream& file, const std::string& scenario_name, engine* engine) const override;
protected:
	virtual void				save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const {} //Subclasses MUST define!
public:
	std::string&				set_name(const std::string& name);
	void						set_value(const std::string& variable_name, const std::string& new_value);
};

#define GAME_OBJ_SUBCLASS(name, save_id) public:\
name();\
virtual name* instance() const override {return new name();}\
virtual uint32_t get_save_id() const override {return static_cast<uint32_t>(save_id);}\
protected:\
virtual void save_variables(std::ofstream& file, const std::string& scenario_name, engine* engine) const override;\
virtual void load_variables(std::ifstream& file, const std::string& scenario_name, engine* engine) override;\
virtual ~name(){}\
private:
