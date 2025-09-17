#ifndef MASTER_HEADER_FILE
#define MASTER_HEADER_FILE

#include <string>

const static char dummy_command_character = 1; //Using the ASCII-reserved "Start of Header" character to replace content in quotes so it doesn't get processed, then gonna swap it back in later. I use this character because it's untypable, usually invisible in text, and not really used by modern systems for anything else, making it ideal as a control character.
const static char pair_delimeter_character = 1; //Used by arrays to denote a pair between an index-value and its value - much like a hashmap.
const static char func_pair_character = 1; //Used by the script loading code to pair user command functions with their input-substituted variations (which are automatically generated at load-time)

const static char dummy_left_paran = 2;
const static char dummy_right_paran = 3;
const static char dummy_comma = 4; //Stuff with commas inside of quotes has to be swapped out for a dummy character, and then swapped back in at display-time - otherwise it bugs out and separates each half of the quotations into different arguments.
const static char dummy_equals = 5;
const static char dummy_slash = 6;
const static char dummy_plus = 128; //ASCII only uses 0 - 127, but 8-bit char values go much higher, so that space is open for me to use as I see fit.
const static char dummy_star = 129;
const static char dummy_dash = 130;
const static char dummy_percent = 131;
const static char dummy_space = 132;
const static char dummy_colon = 133;
const static char dummy_exclamation = 134;
const static char dummy_period = 135;
const static char dummy_left_brace = 139;
const static char dummy_right_brace = 140;

const static char dummy_array_delimeter = 136;

const static char delay_marker_char = 137; //Acts like braces around a number in order to make it delay during a print.

const static char set_print_color_header = 138; //This character denotes a change in text color. The following character is not printed, but determines what the new color is.

const static bool FULL_CLEAR = true;

const static std::string PRINT_PREAMBLE = "  ";

const static int MILLISECONDS_BETWEEN_CHARACTERS = 1;

//Dev mode requires C++17 or newer.
#define DEV_MODE false
//Dev mode allows for scenarios in folders to be "pre-baked", which means it writes a C++ code file with a function in it that hard-codes the scenario files into
//the executable, thus allowing for a self-contained text adventure to be written with this engine.
//Dev mode also allows for scenarios to be loaded from folders even if it would ordinarily force scenarios to be chosen from the list of pre-baked scenarios.

//Whether or not to compile with hard-coded scenario code, which is generated in dev mode using normal scenario folders
#define INCLUDE_BAKED_SCENARIOS true

const static bool FORCE_BAKED_SCENARIOS = true; //Whether or not it limits the user to pre-baked scenarios. If no baked scenarios are available then it falls back to allowing loaded scenarios too.
#endif