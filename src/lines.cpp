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

std::string get_ext(std::string fname){
	std::string ext;
	std::stringstream strstream (fname); 

	int num = 0; 
	while (std::getline( strstream, ext, '.')){
		if (num == 0){
			num++; 
			continue; 
		} else {
			break;
		}
	}

	return ext; 
} 


bool ext_valid(std::string fname, extset& extensions){
	
	std::string ext = get_ext(fname);

	if (extensions.count( ext ) > 0) 
		return true; 

	return false; 
}

void read_proj_dir(linespath& lp, std::vector<std::string>& paths, extset& extensions){

	std::queue<dircont> dir_q;  

	dircont init = { lp.path, "", DT_DIR }; 
	dir_q.push(init); 

	DIR* dir; 
	struct dirent* ent; 

	while (dir_q.size() > 0){

		dircont f = dir_q.front(); 
		dir_q.pop();

		// std::cout << f.name << std::endl;

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

				// we don't want it to recurse on itself and left the current dir scope
				if ( _f_name == ".." || _f_name == ".") continue; 
				
				dircont _f = { _f_name, full_dir_path, _type };
				dir_q.push(_f);

			} else {
				 
				bool recognized = ext_valid(_f_name, extensions);
				 
				if (recognized){

					std::string file_path = full_dir_path + _f_name;
					paths.push_back(file_path);

					size_t fsize = file_path.size();

					if (fsize > lp.longest_path_name)
						lp.longest_path_name = fsize;
				}
			}
		}
	}
}

// assumes that files won't have more than 100,000 lines in them 
size_t get_digits(int n){
	if 		(n < 10) 		return 1; 
	else if (n < 100) 		return 2;
	else if (n < 1000)		return 3; 
	else if (n < 10000) 	return 4; 
	else if (n < 100000) 	return 5;
	else 					return 6; 
}

void inspect_files(std::vector<std::string>& paths, linespath& lp){
	filemap fmap; 

	std::string _space (lp.longest_path_name - 11 + 5, ' ');
	std::string _row_space (ROW_SPACE, ' ');
	std::cout << "[file name]" << _space << "[lines]" << _row_space <<"[size in bytes]" << std::endl; 
	
	std::string line_token; 
	std::ifstream f;
	for (auto& p : paths){
		
		f.open(p);

		if (f.is_open()){
			int c = 0; 
			while (std::getline(f, line_token, '\n')) c++;  

			f.close();

			struct stat stat_obj;
			int res; 
			if ( (res = stat(p.c_str(), &stat_obj) ) < 0){
				std::cout << "[Error] unable to get file size for " << p << " from stat" << std::endl; 
				exit(1);
			}

			size_t fsize = stat_obj.st_size; 

			size_t num_digits = get_digits(c);

			std::string _fname_space (lp.longest_path_name - p.size() + 5, ' ');
			std::string _linespace (lp.longest_path_name - num_digits, ' ');

			std::cout 	<< p 
						<< _fname_space
						<< c 
						<< _linespace
						<< fsize
						<< std::endl;

		} else {
			std::cout << "[Error] uable to open file: " << p << std::endl; 
			f.close();
		}

		std::string fext = get_ext(p);
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
	// std::cout << "input path: " << lp.path << std::endl;

	std::vector<std::string> paths;
	extset& extensions = file_extensions;

	lp.longest_path_name = 0; 

	read_proj_dir(lp, paths, extensions);

	inspect_files(paths, lp); 

	return 0; 
}