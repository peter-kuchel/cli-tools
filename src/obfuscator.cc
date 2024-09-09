#include "obfuscator.h"

// get_file_contents(std::string)

// void obs_js(std::string &fext, std::string &fname){

// }

void obfuscate(std::string &fext, std::string &fname){

	// check if ext is not supported 
	if (supported_exts.find(fext) == supported_exts.end()){
		std::cout << "File extention: (." << fext << ") is currently not supported\n"
				  << "see usage with -h to see currently supported file extensions" << std::endl; 
		exit(1);
	}

	std::string fcontent;
	std::string fline; 
	std::ifstream f (fname); 

	if (f.is_open()){
		
		while ( std::getline(f, fline) ){

			fcontent += fline; 
			fcontent.push_back('\n');
		}

	} else {
		std::cout << "File could not be opened" << std::endl; 
		exit(1);
	}

	// remove last \n from content
	fcontent.pop_back();

	std::cout << fcontent << std::endl;
		
}

int main(int argc, char* argv[]){

	if (argc < 2){
		usage();
		exit(1);
	}

	// get file supplied 
	std::string fname (argv[1]); 

	// find extention
	std::size_t ext_pos = fname.find('.'); 

	if (ext_pos == std::string::npos){
		std::cout << "No file extension found" << std::endl; 
		usage(); 
		exit(1);
	}

	std::string ext (fname.begin() + ext_pos +1, fname.end());
	obfuscate(ext, fname); 

	// std::cout << ext << std::endl; 
}