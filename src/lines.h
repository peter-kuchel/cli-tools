#ifndef CLI_LINES_H
#define CLI_LINES_H 

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define ROW_SPACE 10

// https://embeddedartistry.com/blog/2019/04/08/a-general-overview-of-what-happens-before-main/

typedef std::unordered_set<std::string> extset;
typedef std::unordered_map<std::string, std::string> filemap;

typedef struct {
	std::string path;
	int options; 

	size_t longest_path_name;

} linespath;

typedef struct {
	
	std::string name; 
	std::string dir;
	uint8_t f_type;

} dircont; 

void usage(){
	std::cout 	<< "lines [path] [options]\n" 
				<< "\tpath: \n"
				<< "\toptions: \n"
				<< std::endl; 
}

const std::string ext_delim = ".";
extset file_extensions (
	{
	"asm", "s", "c", "h", "cpp", "cc", "cxx", "hxx", "hpp"				// asm/c/c++ file extensions
	"js", "html", "css", "ts"											// js/ts and web files 
	"py", "hs", "java"													// other language files
	"sh"
});

#endif 