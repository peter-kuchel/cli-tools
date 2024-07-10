#include "lines.h"

void handle_args(int argc, char** argv, linespath& lp){

	lp.path = *(argv + 1);

	argc -= 2; 
	argv += 2;

	// handle potential options
	while (argc){

		argc--; argv++; 
	}
}

bool ext_valid(std::string fname, std::unordered_set<std::string>& extensions){
	std::string token;
	std::stringstream strstream (fname); 
	std::vector<std::string> str_parts; 
	while (std::getline( strstream, token, '.')) str_parts.push_back(token);

	// std::cout << "file name: " << fname << ", file ext: " << str_parts.back() << std::endl;
	if (extensions.count( str_parts.back() ) > 0) 
		return true; 

	return false; 
}

void read_proj_dir(linespath& lp, std::vector<std::string>& paths, std::unordered_set<std::string>& extensions){

	std::queue<dircont> dir_q;  

	dircont init = {DT_DIR, lp.path, ""}; 
	dir_q.push(init); 

	DIR* dir; 
	struct dirent* ent; 

	while (dir_q.size() > 0){

		dircont f = dir_q.front(); 
		dir_q.pop();

		std::cout << f.name << std::endl;

		std::string full_dir_path = f.dir + f.name + "/"; 

		const char* f_name = full_dir_path.c_str(); 
		dir = opendir(f_name);

		if (dir == NULL){
			// figure out error if there is any
		}

		while ( (ent = readdir(dir)) != NULL){
			std::string _f_name(ent->d_name);
			uint8_t _type = ent->d_type; 
			
			if (_type == DT_DIR){

				if ( _f_name == ".." || _f_name == ".") continue; 
				
				dircont _f = { _type, _f_name, full_dir_path};
				dir_q.push(_f);

			} else {
				 
				bool recognized = ext_valid(_f_name, extensions);
				 
				if (recognized){

					std::string file_path = full_dir_path + _f_name;
					paths.push_back(file_path);
				}

					
			}
		}
	}

	std::cout << "[files got]: \n";
	for (auto p : paths){
		std::cout << p << std::endl; 
	}
}

int main(int argc, char* argv[]){

	linespath lp; 

	// there needs to be a path as arg
	if (argc == 1){
		std::cout << "[Error]: see usage with --help" << std::endl;
		exit(0);
	}

	handle_args(argc, argv, lp);
	std::cout << "input path: " << lp.path << std::endl;

	std::vector<std::string> paths;
	std::unordered_set<std::string>& extensions = file_extensions;

	read_proj_dir(lp, paths, extensions);

	return 0; 
}