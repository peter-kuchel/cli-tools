#include "obfuscator.h"

void add_to_set(str_set &_set, const str_set &resv_words){

    for (auto w = std::begin(resv_words); w != std::end(resv_words); ++w)
        _set.insert(*w); 
}

bool capturable_char(char c, struct obfusdata &data, char_set &non_captures){ 

	if (non_captures.find( c ) == non_captures.end())
		return true;

	if (c == '.')
		data.next_token_is_attr = true;  

	return false; 
}

void process_token(std::string &token, std::string &new_fcontent, str_set &resv_words, str_map &token_map, struct obfusdata &data){

	// std::cout << "Token found: " << token << std::endl;

	std::regex re_magic_num ("[^A-Za-z_]");
	std::regex re_hex_num ("0x[0-9a-fA-F]+");
	

	// check if token is a constant number or hex value 
	bool const_val = std::regex_match(token, re_magic_num) || std::regex_match(token, re_hex_num);

	// check if token cannot be altered or is some const value not to be mapped
	if ( resv_words.find( token ) != resv_words.end() || const_val){
		new_fcontent += token; 

	
	}  else {

		str_map::const_iterator map_val = token_map.find(token);
		// otherwise check if needs to be added to token map
		if ( map_val == token_map.end()){

			// to avoid standard attributes and functions of built-ins 
			if ( !data.next_token_is_attr ){

				// std::cout << "adding " << token << " to map" << std::endl;

				// create obfusacted variable name
				std::string val ("__obs_n" + std::to_string(data.var_count));
			
				token_map.insert( {token, val} );
				data.var_count++;
				new_fcontent += val;
				
			} else {
				data.next_token_is_attr = false;
				new_fcontent += token; 
			}
			
		} else {

			// else the token already has a map value
			new_fcontent += map_val->second; 
		}
	}
}

bool token_captured(std::string &token, char_set &non_captures, char c){
 
	bool capt_prereq = false;  

	if (non_captures.find( c ) != non_captures.end())
		capt_prereq = true; 

	capt_prereq &= token.size() > 0;

	return capt_prereq;
}

void init_obfusdata(struct obfusdata &data){
	data.var_count = 0;
	data.next_token_is_attr = false; 
	data.last_char = '\0'; 
}

void avoid_imports(struct obfusdata &data, std::string &new_fcontent, std::string::iterator &f_ptr, char c){
	std::string str; 
	int i = 1;

	if (c == '#'){

		while (*(f_ptr + i ) != ' '){
			str.push_back( *( f_ptr + i) );
			i++;
		}
	}

	if (
		data.last_token.compare("import") == 0 || 
		str.compare("include") == 0
	){
		while (*f_ptr != '\n'){
			new_fcontent.push_back( *f_ptr );
			f_ptr++; 
		}
	} 
	
}

void obfuscate(std::string &fcontent, std::string &new_fcontent, str_set &resv_words, char_set &non_captures){ 

	std::string token;  
	std::string::iterator f_ptr = fcontent.begin(), f_end = fcontent.end();

	str_map token_map; 

	std::regex re_non_space ("[ \t\r\f]");
	std::regex re_eol_ids ("[{};]");

	obfusdata data; 
	init_obfusdata(data);
	std::string _c, tmp;					// char as a string for regex matching

	while (f_ptr != f_end){

		char c = *f_ptr;

		_c.push_back(c);

		bool is_space_char = std::regex_match(_c, re_non_space); 

		if ( token_captured(token, non_captures, c) ){
			process_token(token, new_fcontent, resv_words, token_map, data);

			data.last_token.clear();
			data.last_token.append(token); 
			token.clear();
		}


		if ( capturable_char(c, data, non_captures) ){
			token.push_back( c );

		} else {

			// avoid tokenizing strings 
			if (c == '\"' || c == '\'' || c == '`'){
				
				do {
					new_fcontent.push_back(*f_ptr);
					f_ptr++; 

				} while (*f_ptr != c);

			// avoid tokenizing comments (only handles c-style comments atm if to expand supported langs)
			} else if (c == '/'&& *(f_ptr + 1) == '/'){

				while (*f_ptr != '\n'){
					new_fcontent.push_back(*f_ptr); 
					f_ptr++;
				}

				c = *f_ptr;
			} 



			// avoid tokenizing imports (assumes only from standard libraries atm)
			avoid_imports(data, new_fcontent, f_ptr, c);
			
				
			if ( c == '\n' ){

				tmp.push_back(data.last_char);

				if ( !std::regex_match(tmp, re_eol_ids) && data.last_char != '\n' && new_fcontent.size() > 0)
					new_fcontent.push_back(';');

				tmp.clear();
			} else if ( !is_space_char )
				new_fcontent.push_back(c);
		}

		if (!is_space_char)
			data.last_char = c;

		f_ptr++;
		_c.clear();
		
	}

	// std::cout << "Tokens in map: " << std::endl; 
	// for (auto t = token_map.begin(); t != token_map.end(); ++t){
	// 	std::cout << t->first << " : " << t->second << std::endl;
	// }

	// std::cout << fcontent << std::endl; 
	std::cout << new_fcontent << std::endl; 
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

	std::string fcontent, new_fcontent;
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
	char_set non_captures (non_cap_chars); 

	find_lang_resv_words(reserve_words, fext);

	obfuscate(fcontent, new_fcontent, reserve_words, non_captures);

	std::string new_fname ("ob_");
	new_fname += fname; 

		
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