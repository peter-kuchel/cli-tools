#ifndef LGREP_CLI_H
#define LGREP_CLI_H

#include <iostream>
#include <string>
#include <unordered_set>

typedef std::unordered_set<char> chr_set; 
typedef std::unordered_set<char>::const_iterator chr_itr;

void create_chr_set(chr_set& c_set, std::string& chars);

const std::string CHAR_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string CHAR_LOWER = "abcdefghijklmnopqrstuvwxyz";
const std::string UNDER_SCORE = "_";
const std::string CHAR_DIGITS = "0123456789";

chr_set DIGITS;
chr_set ALPHA_NUMERIC;  


#endif 