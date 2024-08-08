#include "Server.h"

void create_chr_set(chr_set &c_set, const std::string &chars){

    for (auto c = std::begin(chars); c != std::end(chars); ++c)
        c_set.insert(*c); 
    
}

bool check_exists_in(const std::string &str, chr_set &check_set){
    for ( auto chr = std::begin(str); chr != std::end(str); ++chr ){
            chr_itr has_digit = check_set.find(*chr);

            if ( has_digit != check_set.end() ){
                return 1; 
            }
        }
        return 0;
}

void reset_regex(struct regex &re){
    re.substr.clear();
    re.char_set.clear();
    re.negative_group = false;  
    re.start_of_line  = false; 
    re.end_of_line = false;
}

void build_regex_match(std::string::const_iterator &pattern_iter, struct regex &re){

     reset_regex(re);

    // std::cout << "got: "<< re.pattern << std::endl;

    switch(re.pattern){
        case REGXCASE::ANY_SINGLE: 
            re.char_set.insert(*pattern_iter);
            break; 

        case REGXCASE::ALPHA_ANY_SINGLE:
            re.char_set = ALPHA_NUMERIC; 
            break; 

        case REGXCASE::DIGIT_ANY_SINGLE:
            re.char_set = DIGITS;  
            break; 

        case REGXCASE::GROUPING:
            
            if (*(pattern_iter + 1) == '^' && *pattern_iter == '['){
                re.negative_group = true;
                ++pattern_iter; 
            } 
                    
            while (*(++pattern_iter) != ']')
                re.char_set.insert( *pattern_iter );

            break; 

        case REGXCASE::START_OF_LINE: 
            re.start_of_line = true; 
            break; 
        case REGXCASE::END_OF_LINE:
            re.end_of_line = true; 
            break;
        default:
            break; 
    }

    ++pattern_iter; 
}

void parse_pattern_next(std::string::const_iterator &pattern_iter, struct regex &re){
    
    if (*pattern_iter == '\\'){

        ++pattern_iter;
        switch(*pattern_iter){

            case 'w':
                re.pattern = REGXCASE::ALPHA_ANY_SINGLE;
                break;
            case 'd':
                re.pattern =  REGXCASE::DIGIT_ANY_SINGLE;
                break;
            default: 
                re.pattern = REGXCASE::NOT_RECOGNIZED;
                break;
        }

    } else if (*pattern_iter == '['){

        re.pattern = REGXCASE::GROUPING; 

    } else if (*pattern_iter == '^'){

        re.pattern = REGXCASE::START_OF_LINE;

    } else if (*pattern_iter == '$'){

        // std::cout << "END OF LINE" << std::endl;
        re.pattern = REGXCASE::END_OF_LINE; 

    }else if ( ALL_CHARS.find( *pattern_iter ) != ALL_CHARS.end() ){

        re.pattern = REGXCASE::ANY_SINGLE;

    } else {

        re.pattern = REGXCASE::NOT_RECOGNIZED; 
    }

    build_regex_match(pattern_iter, re); 
    
}

bool check_regex_match(char c, struct regex &re){

    // std::cout << "char: " << c << " with pattern: " << re.pattern << std::endl;
    // debug_chr_set(re);
    bool result; 
    switch (re.pattern){
        case REGXCASE::ANY_SINGLE:
        case REGXCASE::ALPHA_ANY_SINGLE:
        case REGXCASE::DIGIT_ANY_SINGLE:
            return ( re.char_set.find(c) != re.char_set.end() );

        case REGXCASE::GROUPING:
            
            result = re.char_set.find(c) != re.char_set.end();
            // std::cout << "result is: "<< result << std::endl; 
            if (re.negative_group) 
                return !result; 
            return result; 

        case REGXCASE::NOT_RECOGNIZED:
        default:
            return false; 

    }
}

bool match_pattern(const std::string &input_line, const std::string &pattern){

    bool result = true, entry_position = false; 
    regex re; 

    std::string::const_iterator pattern_iter = std::begin(pattern);
    std::string::const_iterator input_str = std::begin(input_line); 

    // match with first pattern that appears in the regex somewhere in the string
    parse_pattern_next(pattern_iter, re);

    // if pattern does not start with ^ find an entry in the input
    if (!re.start_of_line){

        // std::cout << "finding entry" << std::endl;
        while ( input_str != std::end(input_line) ){

            entry_position = check_regex_match(*input_str, re);

            // std::cout << "entry found:" << entry_position << std::endl; 
            ++input_str;
            if (entry_position) break; 
            
        }

        if (input_str == std::end(input_line) && !entry_position)
            return false; 
    }

    bool non_matching;
    while (result && input_str != std::end(input_line)){

        if (pattern_iter == std::end(pattern))
            break; 

        parse_pattern_next(pattern_iter, re);

        result &= check_regex_match(*input_str, re);
        ++input_str;
    

        
    }

    // input exhausted before pattern --> return false 
    if (input_str == std::end(input_line) && pattern_iter != std::end(pattern)){

        // get last pattern to check for $ 
        parse_pattern_next(pattern_iter, re); 
        if (re.end_of_line)
            return true; 
        
        return false;
    } 
    
    
    return result;
}

int main(int argc, char* argv[]) {

    create_chr_set(DIGITS, CHAR_DIGITS);
    create_chr_set(ALPHA_NUMERIC, CHAR_UPPER + CHAR_LOWER + CHAR_DIGITS);
    create_chr_set(ALL_CHARS, CHAR_UPPER + CHAR_LOWER + CHAR_NON_SPEC + CHAR_DIGITS);

    debug_regexcase();

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
