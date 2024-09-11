#include "obfuscator.h"

void add_to_set(str_set &_set, const str_set &resv_words){

    for (auto w = std::begin(resv_words); w != std::end(resv_words); ++w)
        _set.insert(*w); 
}

bool capturable_char(char c, struct obfusdata &data){

	switch(c){
		case '.':
			data.last_char_is_attr = true; 
			break; 
		case ' ':
		case '\n':
		case '=':
			break; 
		default:
			return true; 		
	}

	return false; 
}

void process_token(std::string &token, std::string &new_fcontent, str_set &resv_words, str_map &token_map, struct obfusdata &data){

	std::cout << "Token found: " << token << std::endl;
	// check if token cannot be altered 
	if ( resv_words.find( token ) != resv_words.end() ){
		new_fcontent += token; 

	} else {

		// otherwise check add to token map
		if ( token_map.find(token) == token_map.end()){

			std::string val ("x" + std::to_string(data.var_count)) ;
			
			token_map.insert( {token, val} );
			data.var_count++;
		}
	}

	token.clear();
}

bool token_captured(std::string &token, char c){
 
	bool capt_prereq = false;  
	switch (c){
		case ' ':
		case '=':
		case '\n':
		case ',':
		case '.':
			capt_prereq = true;
			break; 

		default:
			break;
	}

	capt_prereq &= token.size() > 0;

	return capt_prereq;
}

void init_obfusdata(struct obfusdata &data){
	data.var_count = 0;
	data.last_char_is_attr = false; 
}

void obfuscate(std::string &fcontent, str_set &resv_words){

	// std::cout << fcontent << std::endl; 

	std::string new_fcontent, token;  
	std::string::iterator f_ptr = fcontent.begin(), f_end = fcontent.end();

	str_map token_map; 

	obfusdata data; 
	 

	while (f_ptr != f_end){

		char c = *f_ptr;

		if ( token_captured(token, c) )

			process_token(token, new_fcontent, resv_words, token_map, data);

		 
		if ( capturable_char(c, data) )
			token.push_back( c );
		else 
			new_fcontent.push_back(c);

		f_ptr++;
		
	}

	for (auto t = token_map.begin(); t != token_map.end(); ++t){
		std::cout << t->first << " : " << t->second << std::endl;
	}

	std::cout << fcontent << std::endl; 
	std::cout << "hello\n";
}

void find_lang_resv_words(str_set &resv_words, std::string &fext){

	if (fext.compare("js") == 0){
		add_to_set(resv_words, js_reserved_words);
		// resv_words (js_reserved_words);
	} else if (1){

	}
}

void setup_obfuscate(std::string &fext, std::string &fname){

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

	str_set reserve_words; 

	find_lang_resv_words(reserve_words, fext);

	obfuscate(fcontent, reserve_words);

		
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
	setup_obfuscate(ext, fname); 
}