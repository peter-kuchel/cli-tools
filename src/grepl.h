#ifndef LGREP_CLI_H
#define LGREP_CLI_H

#include <iostream>
#include <string>
#include <unordered_set>

#define DEBUG 1

typedef std::unordered_set<char> chr_set; 
typedef std::unordered_set<char>::const_iterator chr_itr;

typedef std::string::const_iterator str_itr; 

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

	ANY_SINGLE, 

	ALPHA_ANY_SINGLE, 
	DIGIT_ANY_SINGLE,

	GROUPING,
	START_OF_LINE,
	END_OF_LINE, 

	ONE_OR_MORE,
	ZERO_OR_ONE,

};

struct regex {

	chr_set char_set; 
	std::string substr; 
	REGXCASE current_pattern; 
	
	bool prev_matched; 
	bool negative_group;

	bool start_of_line; 
	bool end_of_line; 

	bool one_or_more; 

}; 

void debug_regexcase(){
	std::cout << "NOT_RECOGNIZED: " << REGXCASE::NOT_RECOGNIZED << "\n"
			  << "ANY_SINGLE: " << REGXCASE::ANY_SINGLE << "\n"
			  << "ALPHA_ANY_SINGLE: " << REGXCASE::ALPHA_ANY_SINGLE << "\n"
			  << "DIGIT_ANY_SINGLE: " << REGXCASE::DIGIT_ANY_SINGLE << "\n"
			  << "GROUPING: " << REGXCASE::GROUPING << "\n" 
			  << "START_OF_LINE: " << REGXCASE::START_OF_LINE << "\n"  
			  << "END_OF_LINE: " << REGXCASE::END_OF_LINE << "\n" 
			  << "ONE_OR_MORE: " << REGXCASE::ONE_OR_MORE << "\n" <<
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