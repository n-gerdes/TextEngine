#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include <vector>
#include <string>
#include <map>
#include <mutex>
//tokenizer class is responsible for taking raw input from the user and turning it into a list of tokens
class string_utils
{
public:

	//This function is used to perform complex replacements, in which the data being replaced is used to determine the replacement. It spits out the string before the found pattern and the string after, as well as any wildcards found inside, which allows you to perform complex substitutions. It returns whether or not the pattern is found.
	bool						contains(const std::string& full_string, const std::string& substring) const;
	bool						contains(const std::string& full_string, const std::string& substring, bool case_sensitive) const;
	bool						complex_replacement(const std::string& original, std::string search_pattern, std::string& extracted_prestring, std::string& extracted_poststring, std::vector<std::string>& wildcards) const;
	bool						complex_replacement(const std::string& original, const std::string& search_pattern, std::string& extracted_prestring, std::string& extracted_poststring, std::vector<std::string>& wildcards, const std::string& delimeters, bool case_sensitive) const; //Same as the first but instead of entity-by-entity comparisons, it does token-by-token comparisons.
	bool						complex_replacement(const std::string& original, const std::string& search_pattern, std::string& extracted_prestring, std::string& extracted_poststring, std::vector<std::string>& wildcards, const std::string& delimeters, bool case_sensitive, bool allow_multi_token_wildcards) const;
	std::vector<std::string>	extract_tokens(const std::string& input, const std::string& delimeters) const;
	std::vector<std::string>	extract_tokens_with_potential_subfunctions(const std::string& input, const std::string& delimeters) const;
	std::vector<std::string>	extract_tokens(const std::string& input, char delimeter) const;
	std::vector<std::string>	extract_tokens(const std::string& input) const;
	std::string					format_as_name(const std::string& input) const;
	std::string					get_lowercase(const std::string& str) const;
	bool						is_command_char(char c) const;
	bool						is_decimal(const std::string& str) const;
	bool						is_integer(const std::string& str) const;
	bool						is_numeric(const std::string& str) const;
	void						make_lowercase(std::string& str) const;
	void						make_uppercase(std::string& str) const;
	bool						matches_command(const std::string& command, const std::string& input);
	bool						matches_command(const std::string& command, const std::string& input, std::vector<std::string>& wildcards) const;
	bool						matches_command(const std::string& command, const std::string& input, std::vector<std::string>& wildcards, const std::string& delimeters, bool already_knows_answer) const;
	std::string					replace_all(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only) const;
	std::string					replace_all(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only, bool case_sensitive) const;
	std::string					replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring) const;
	std::string					replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool case_sensitive) const;
	std::string					replace_from_offset(const std::string& original, const std::string& original_substring, const std::string& new_substring, size_t offset) const;
	std::string					replace_from_offset(const std::string& original, const std::string& original_substring, const std::string& new_substring, size_t offset, bool case_sensitive) const;
	std::string					shorten_decimal(std::string decimal);
	bool						starts_with(const std::string& text, const std::string& potential_beginning, bool case_sensitive) const;
	void						strip(std::string& str) const;
	std::string					substring(const std::string& str, size_t first_index, size_t second_index) const; //Alternate substring function to get a substring via two indicies [inclusive]
	bool						ends_with(const std::string& str, const std::string& tail) const;
protected:

private:
};

#endif