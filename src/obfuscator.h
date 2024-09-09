#ifndef OBFUS_CLI_H
#define OBFUS_CLI_H


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>

const std::unordered_set<std::string> supported_exts ( {"js", "c"} );


void usage(){
	std::cout << "obfuscator <filename>\n" <<
				 "\tfiles currently supported:\n" << 
				 "\t\t.js\n" <<
				 ""
			  << std::endl;   
}

#endif 