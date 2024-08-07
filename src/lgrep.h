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
const std::string CHAR_DIGITS = "0123456789";
const std::string CHAR_SPACE = " \t\n\r";

const std::string CHAR_NON_SPEC = "_- ";
const std::string CHAR_ALL_SPEC = "!@#$%^&*()-=+{}[]|;:\'\"\\.,?/`~><";

chr_set DIGITS;
chr_set SPACES; 
chr_set ALPHA_NUMERIC;  
chr_set ALL_CHARS; 

enum REGXCASE {

	NOT_RECOGNIZED,

	ALPHA_SINGLE, 

	ALPHA_ANY_SINGLE, 
	DIGIT_ANY_SINGLE,

	GROUPING

};

struct regex {

	chr_set char_set; 
	std::string substr; 
	REGXCASE pattern; 
	bool negative_group;  

}; 

void debug_regexcase(){
	std::cout << "NOT_RECOGNIZED: " << REGXCASE::NOT_RECOGNIZED << "\n"
			  << "ALPHA_SINGLE: " << REGXCASE::ALPHA_SINGLE << "\n"
			  << "ALPHA_ANY_SINGLE: " << REGXCASE::ALPHA_ANY_SINGLE << "\n"
			  << "DIGIT_ANY_SINGLE: " << REGXCASE::DIGIT_ANY_SINGLE << "\n"
			  << "GROUPING: " << REGXCASE::GROUPING << "\n" << 
	std::endl; 
}

void debug_chr_set(struct regex &re){
	std::cout << "{ "; 
	for (auto c = re.char_set.begin(); c != re.char_set.end(); ++c){
		std::cout << *c << " "; 
	}

	std::cout << "}" << std::endl; 

	std::cout << "negative group: " << re.negative_group << std::endl; 
}


#endif 