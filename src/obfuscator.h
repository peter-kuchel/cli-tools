#ifndef OBFUS_CLI_H
#define OBFUS_CLI_H


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <unordered_set>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string> str_map; 
typedef std::unordered_set<std::string> str_set;
typedef std::unordered_set<char> char_set; 

// enum ATTR_STATUS {

// 	ATTR,
// 	NON_ATR

// };

struct obfusdata {
	std::string last_token;
	int var_count; 
	bool next_token_is_attr; 
	char last_char; 
	 
};

const char_set non_cap_chars ( {

	'\n', ' ', '\t',
	'=', '+', '-', '*', '/', '<', '>', '%',
	'.', ',', ';', ':',
	'[', ']', '{', '}', '(', ')',
	'!', '&', '|', '^', '$',
	'#'
} );

const str_set supported_exts ( {"js"} );


// includes words that were also removed from the ECMAScript 5/6 standard
const str_set js_reserved_words ( {
	"abstract", "arguments", "await", "boolean",
	"break", "byte", "case", "catch",
	"char", "class", "const", "continue",
	"debugger",	"default", "delete", "do",
	"double", "else", "enum", "eval",
	"export", "extends", "false", "final",
	"finally", "float", "for", "function",
	"goto", "if", "implements", "import",
	"in", "instanceof", "int", "interface",
	"let", "long", "native", "new",
	"null", "package", "private", "protected",
	"public", "return", "short", "static",
	"super", "switch", "synchronized", "this",
	"throw", "throws", "transient", "true",
	"try", "typeof", "var", "void",
	"volatile", "while", "with", "yield",

	"Array", "Date",
	"hasOwnProperty", "Infinity", "isFinite", "isNaN",
	"isPrototypeOf", "length", "Math", "NaN",
	"name", "Number", "Object", "prototype",
	"String", "toString", "undefined", "valueOf",

	"console"
} );


void usage(){
	std::cout << "obfuscator <filename>\n" <<
				 "\tfiles currently supported:\n" << 
				 "\t\t.js\n" <<
				 ""
			  << std::endl;   
}

#endif 