#include "Server.h"

void create_chr_set(chr_set& c_set, const std::string& chars){

    for (auto c = std::begin(chars); c != std::end(chars); ++c)
        c_set.insert(*c); 
    
}

bool check_exists_in(const std::string& str, chr_set& check_set){
    for ( auto chr = std::begin(str); chr != std::end(str); ++chr ){
            chr_itr has_digit = check_set.find(*chr);

            if ( has_digit != check_set.end() ){
                return 1; 
            }
        }
        return 0;
}

bool match_pattern(const std::string& input_line, const std::string& pattern) {
    if (pattern.length() == 1) {
        return input_line.find(pattern) != std::string::npos;

    } else if (pattern.compare("\\d") == 0){
        return check_exists_in(input_line, DIGITS);
        
    } else if (pattern.compare("\\w") == 0){
        return check_exists_in(input_line, ALPHA_NUMERIC);

    } else {
        throw std::runtime_error("Unhandled pattern " + pattern);
        
    }
}

int main(int argc, char* argv[]) {

    create_chr_set(DIGITS, CHAR_DIGITS);
    create_chr_set(ALPHA_NUMERIC, CHAR_UPPER + CHAR_LOWER + UNDER_SCORE + CHAR_DIGITS);

    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }
    
    std::string input_line;
    std::getline(std::cin, input_line);
    
    try {
        if (match_pattern(input_line, pattern)) {
            std::cout << "match found" << std::endl;
            return 0;
        } else {
            std::cout << "match NOT found" << std::endl;
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
