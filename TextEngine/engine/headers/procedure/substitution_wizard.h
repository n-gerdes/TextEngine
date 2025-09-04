#ifndef SUBSTITUTION_WIZARD_H
#define SUBSTITUTION_WIZARD_H
#include <string>
#include <map>
#include <vector>


class substitution_wizard
{
	std::vector<std::string> raw_input_removal;
	std::vector<std::string> input_removal; //This is a list of possible inputs to remove from player inputs, reducing the number of permutations it has to be capable of processing.
	std::map <std::string, std::string> input_substitution; //The key is a word that can be replaced, the value is what replaces it.
	std::map<std::string, std::map<std::string, std::vector<std::string>>> output_substitution_map;
	std::map<std::string, bool> known_thes;
public:

	void		load_input_substitution(const std::string& filename);
	void		apply_input_substitution(std::string& modifiable_string_ref) const;
	bool		load_output_substitution_file(const std::string& directory, const std::string& modifier); //Loads an output substitution file from the given directory, 
	std::string output_substitution(const std::string& thesuarus, std::string sentence, int replacement_percent_chance);
	bool		has_thesaurus(const std::string& thesaurus, const std::string& directory); //Checks to see if this substitution wizard has the specified thesaurus; if it doesn't, it tries to load it from the given directory.
};

#endif