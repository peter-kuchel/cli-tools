#ifndef LGREP_CLI_H
#define LGREP_CLI_H

#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <tuple>

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
// chr_set WILDCARD_ALL; 

enum REGXCASE {

	NOT_RECOGNIZED,

	ANY_SINGLE, 

	ALPHA_ANY_SINGLE, 
	DIGIT_ANY_SINGLE,

	GROUPING,
	START_OF_LINE,
	END_OF_LINE, 

	ONE_OR_MORE,
	WILDCARD,
	ALTERNATION,

	BEGIN_GROUP_CAP, 
	END_GROUP_CAP,

};

struct capture_group {
	str_itr group_start; 
	int group_size; 
	int last_group;
	bool group_matched; 
}; 

struct regex_input {
	std::string pattern;
	std::string input_line;
};


struct regex {

	chr_set char_set; 										// set of chars to capture
	std::vector<capture_group> captured_groups; 			// captured groups from ()

	// std::string substr; 

	int current_group;										// index into current capture group, 0 is by default the input_str
	REGXCASE current_pattern; 
	
	bool prev_matched; 										// flag if the previous pattern matched
	bool negative_group;									// check for negative grouping with []

	bool start_of_line; 									// if match occurs at the start of the line
	bool end_of_line; 										// if match occurs at the end of the line 

	bool one_or_more; 										// flag to toggle for matching one or more 
	bool capturing_group; 
	bool begin_group_capture; 								// start of group capture
	bool end_group_capture; 								// end of group capture (to return result)

	bool skip_char;											// skip matching char after parsing

}; 

void debug_regexcase(){
	std::cout << "NOT_RECOGNIZED: " << REGXCASE::NOT_RECOGNIZED << "\n"
			  << "ANY_SINGLE: " << REGXCASE::ANY_SINGLE << "\n"
			  << "ALPHA_ANY_SINGLE: " << REGXCASE::ALPHA_ANY_SINGLE << "\n"
			  << "DIGIT_ANY_SINGLE: " << REGXCASE::DIGIT_ANY_SINGLE << "\n"
			  << "GROUPING: " << REGXCASE::GROUPING << "\n" 
			  << "START_OF_LINE: " << REGXCASE::START_OF_LINE << "\n"  
			  << "END_OF_LINE: " << REGXCASE::END_OF_LINE << "\n" 
			  << "ONE_OR_MORE: " << REGXCASE::ONE_OR_MORE << "\n"
			  << "WILDCARD: " << REGXCASE::WILDCARD << "\n"
			  << "ALTERNATION: " << REGXCASE::ALTERNATION << "\n"
			  << "BEGIN_GROUP_CAP: " << REGXCASE::BEGIN_GROUP_CAP << "\n" 
			  << "END_GROUP_CAP: " << REGXCASE::END_GROUP_CAP << "\n"
	<< std::endl; 
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