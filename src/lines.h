#ifndef CLI_LINES_H
#define CLI_LINES_H 

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

typedef struct {
	std::string path;
	int options; 

} linespath;

typedef struct {
	uint8_t f_type;
	std::string name; 
	std::string dir; 
} dircont; 

void usage(){
	std::cout 	<< "lines [path] [options]\n" 
				<< "\tpath: \n"
				<< "\toptions: \n"
				<< std::endl; 
}

const std::string ext_delim = ".";
std::unordered_set<std::string> file_extensions (
	{
	"asm", "s", "c", "h", "cpp", "cc", "cxx", "hxx", "hpp"				// asm/c/c++ file extensions
	"js", "html", "css", "ts"											// js/ts and web files 
	"py", "hs", "java"													// other language files
	"sh"
});

#endif 