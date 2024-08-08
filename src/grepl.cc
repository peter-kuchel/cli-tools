#include "Server.h"

void create_chr_set(chr_set &c_set, const std::string &chars){

    for (auto c = std::begin(chars); c != std::end(chars); ++c)
        c_set.insert(*c); 
    
}

void init_regex(struct regex &re){
    re.one_or_more = false; 
    // re.zero_or_one = false; 
}

void reset_regex(struct regex &re){
    re.substr.clear();
    re.char_set.clear();
    re.negative_group = false;  
    re.start_of_line  = false; 
    re.end_of_line = false;
}

void build_regex_match(std::string::const_iterator &pattern_iter, struct regex &re){

    

    // keep state for these cases
    if (re.one_or_more)
        return; 
    
    reset_regex(re); 

    if (DEBUG){
        std::cout << "building regex pattern" << std::endl; 
    }

    switch(re.current_pattern){

        case REGXCASE::ANY_SINGLE:
            re.char_set.insert( *pattern_iter );
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
            if (DEBUG) std::cout << "AT DEFAULT" << std::endl; 
            break; 
    }

    
    ++pattern_iter; 
}

void parse_pattern_next(std::string::const_iterator &pattern_iter, struct regex &re){
    
    if (DEBUG)
        std::cout << "-- Parsing next pattern --" << std::endl; 

    if (*pattern_iter == '\\'){

        ++pattern_iter;
        switch(*pattern_iter){

            case 'w':
                re.current_pattern = REGXCASE::ALPHA_ANY_SINGLE;
                break;
            case 'd':
                re.current_pattern =  REGXCASE::DIGIT_ANY_SINGLE;
                break;
            default: 
                re.current_pattern = REGXCASE::NOT_RECOGNIZED;
                break;
        }

    } else if (*pattern_iter == '+'){ 

        re.one_or_more = true; 
        
    } else if (*pattern_iter == '['){

        re.current_pattern = REGXCASE::GROUPING; 

    } else if (*pattern_iter == '^'){

        re.current_pattern = REGXCASE::START_OF_LINE;

    } else if (*pattern_iter == '$'){

        re.current_pattern = REGXCASE::END_OF_LINE; 

    } else if ( ALL_CHARS.find( *pattern_iter ) != ALL_CHARS.end() ){

        re.current_pattern = REGXCASE::ANY_SINGLE;

    } else {

        re.current_pattern = REGXCASE::NOT_RECOGNIZED; 
    }

    if (DEBUG){
        if (re.one_or_more)
            std::cout << "[toggled one or more]" << std::endl;
        else 
            std::cout << "got regex case: " << re.current_pattern << std::endl; 
    }

    build_regex_match(pattern_iter, re); 
    
}

bool check_regex_match(char c, struct regex &re){

    if (DEBUG){
        std::cout << "char: " << c << " with pattern: " << re.current_pattern << std::endl;
        debug_chr_set(re);
    }
    

    bool result; 
    switch (re.current_pattern){
        case REGXCASE::ANY_SINGLE:
        case REGXCASE::ALPHA_ANY_SINGLE:
        case REGXCASE::DIGIT_ANY_SINGLE:
            return ( re.char_set.find(c) != re.char_set.end() );

        case REGXCASE::GROUPING:
            
            result = re.char_set.find(c) != re.char_set.end();

            if (re.negative_group) 
                return !result; 

            return result; 

        case REGXCASE::NOT_RECOGNIZED:
        default:
            return false; 

    }
}

bool check_one_or_more(struct regex &re, str_itr &pattern_iter, str_itr &input_str){

    if (re.one_or_more && !re.prev_matched){

            if (DEBUG){
                std::cout << "[Exiting from 1 or more]" << std::endl; 
            }

            re.one_or_more = false; 
            ++pattern_iter;
            --input_str; 

            return true; 
    }

    return false; 
}

bool check_optional(struct regex &re, str_itr &pattern_iter, str_itr &input_str, bool current_matched){

    std::cout << "[Checking for optional]: " << std::endl;

    if (*(pattern_iter) == '?'){

        if ( !current_matched )
            --input_str; 

        ++pattern_iter; 
        return true;
    }

    return false;  
}

bool match_pattern(const std::string &input_line, const std::string &pattern){

    bool end_result = true, entry_position = false; 
    regex re;  
    init_regex(re);

    str_itr pattern_iter = std::begin(pattern);
    str_itr input_str = std::begin(input_line); 

    auto input_end = std::end(input_line);
    auto pattern_end = std::end(pattern);

    // match with first pattern that appears in the regex somewhere in the string
    parse_pattern_next(pattern_iter, re);

    // if pattern does not start with ^ find an entry in the input
    if (!re.start_of_line){

        while ( input_str != input_end){

            entry_position = check_regex_match(*input_str, re);

            if (DEBUG){
                std::cout << "At: " << *input_str << " -- entry found: " << entry_position << std::endl;
            } 

            ++input_str;
            if (entry_position) break; 
            
        }

        if (input_str == input_end && !entry_position)
            return false; 

        if (entry_position)
            re.prev_matched = true; 
    }

    // continue matching
    
    bool current_matched, is_optional;
    while (end_result && input_str != input_end){

        if (pattern_iter == pattern_end)
            break; 

        parse_pattern_next(pattern_iter, re);

        // check for one or more and if failed to match for previous
        
        if ( !check_one_or_more(re, pattern_iter, input_str) ) {

            current_matched = check_regex_match(*input_str, re);

            if (DEBUG)
                std::cout << "Matched? -- " << current_matched << std::endl; 

            is_optional = check_optional(re, pattern_iter, input_str, current_matched);

            // issue is here
            if (!re.one_or_more && !is_optional)
                end_result &= current_matched;

            re.prev_matched = current_matched;

            ++input_str;
        }

        // check if anything toggled while hitting the end of the string
        if (input_str == input_end){

            if (DEBUG)
                std::cout << "INPUT STRING FINISHED" << std::endl; 
            
            check_one_or_more(re, pattern_iter, input_str);

        }
             
    }

    // check if input exhausted before pattern
    if (input_str == input_end && pattern_iter != pattern_end){

        if (DEBUG)
            std::cout << "Input exhausted before pattern" << std::endl; 

        // get last pattern to check for $ 
        parse_pattern_next(pattern_iter, re);

        if (re.end_of_line)
            end_result &= true; 
        else 
            end_result &= false; 
    } 
    
    
    return end_result;
}

int main(int argc, char* argv[]) {

    create_chr_set(DIGITS, CHAR_DIGITS);
    create_chr_set(ALPHA_NUMERIC, CHAR_UPPER + CHAR_LOWER + CHAR_DIGITS);
    create_chr_set(ALL_CHARS, CHAR_UPPER + CHAR_LOWER + CHAR_NON_SPEC + CHAR_DIGITS);

    if (DEBUG)
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
