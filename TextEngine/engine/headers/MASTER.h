#pragma once

const static char dummy_command_character = 1; //Using the ASCII-reserved "Start of Header" character to replace content in quotes so it doesn't get processed, then gonna swap it back in later. I use this character because it's untypable, usually invisible in text, and not really used by modern systems for anything else, making it ideal as a control character.
const static char pair_delimeter_character = 1; //Used by arrays to denote a pair between an index-value and its value - much like a hashmap.
const static char func_pair_character = 1; //Used by the script loading code to pair user command functions with their input-substituted variations (which are automatically generated at load-time)

const static char dummy_left_paran = 2;
const static char dummy_right_paran = 3;
const static char dummy_comma = 4; //Stuff with commas inside of quotes has to be swapped out for a dummy character, and then swapped back in at display-time - otherwise it bugs out and separates each half of the quotations into different arguments.
const static char dummy_equals = 5;
const static char dummy_slash = 6;
const static char dummy_plus = 128;
const static char dummy_star = 129;
const static char dummy_dash = 130;
const static char dummy_percent = 131;
const static char dummy_space = 132;

const static bool FULL_CLEAR = false;