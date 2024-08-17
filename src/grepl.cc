#include "Server.h"

void create_chr_set(chr_set &c_set, const std::string &chars){

    for (auto c = std::begin(chars); c != std::end(chars); ++c)
        c_set.insert(*c); 
    
}

void init_regex(struct regex &re, struct regex_input &re_in){
    re.one_or_more = false; 
    re.prev_matched = false; 
    re.skip_char = false; 

    // adding the input line just so it takes index 0 
    re.captured_groups.push_back( {std::begin(re_in.input_line), (int)re_in.input_line.size(), 0, false} );
    re.current_group = 0; 
    re.capturing_group = false; 
}

void reset_regex(struct regex &re){
    // re.substr.clear();
    re.char_set.clear();
    re.negative_group = false;  
    re.start_of_line  = false; 
    re.end_of_line = false;
    // re.wildcard = false; 
}

void build_regex_match(str_itr &input_str, str_itr &pattern_iter, struct regex &re, struct regex_input &re_in){

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

        case REGXCASE::WILDCARD:
            // re.wildcard = true;
            break;

        case REGXCASE::BEGIN_GROUP_CAP:
            re.begin_group_capture = true; 
            break;

        case REGXCASE::END_GROUP_CAP:
            re.end_group_capture = true; 
            break;

        case REGXCASE::ALTERNATION:

            // assume alternation is either between () or seperates the entire pattern
            // get current group if there is one (that isn't already 0)
            {
                bool current_group_match = re.captured_groups[re.current_group].group_matched;

                if (DEBUG)
                    std::cout << "Alternation needed: " << !current_group_match << std::endl; 

                if ( !current_group_match ){   
                    
                    int i = 0; 
                    while ( (*(pattern_iter + i) != '(' ) && (pattern_iter + i) != std::begin(re_in.pattern) )
                            i--; 

                    if (input_str != std::begin(re_in.input_line))
                        input_str += (i + 1);             // plus one to account for '(' (possible bug here)

                    if (DEBUG)
                        std::cout << "[ ALTERNATION ]\nmoving input back: " << ( (i+1) * -1) << " spaces" << std::endl;

                    // re-try for the alternation
                    re.captured_groups[re.current_group].group_matched = true;
                    
                }
            }
            re.skip_char = true;
            if (DEBUG){
                std::cout << "Input at: " << *input_str << std::endl;
                std::cout << "Pattern at: " << *pattern_iter << std::endl;
            } 
            break; 

        default:
            if (DEBUG) std::cout << "AT DEFAULT" << std::endl; 
            break; 
    }

    
    ++pattern_iter; 
}

void parse_pattern_next(str_itr &input_str, str_itr &pattern_iter, struct regex &re, struct regex_input &re_in){
    
    if (DEBUG)
        std::cout << "-- Parsing next pattern --" << std::endl; 

    // // if alternation
    // if (re.skip_char)
    //     re.skip_char = false;

    if (*pattern_iter == '\\'){

        ++pattern_iter;
        switch(*pattern_iter){

            case 'w':
                re.current_pattern = REGXCASE::ALPHA_ANY_SINGLE;
                break;
            case 'd':
                re.current_pattern =  REGXCASE::DIGIT_ANY_SINGLE;
                break;

            // need to check for escape characters too 
            default: 
                re.current_pattern = REGXCASE::NOT_RECOGNIZED;
                break;
        }

    // capture groups
    } else if ( *pattern_iter == '|'){

        re.current_pattern = REGXCASE::ALTERNATION;

    } else if ( *pattern_iter == '(' ){
        
        int i = 1; 

        // capture where the group is at
        while (*(pattern_iter + i) != ')')
            i++; 

        re.captured_groups.push_back( { pattern_iter, i, re.current_group, true} );
        
        // get current group pos in vector
        re.current_group = re.captured_groups.size() - 1;

        re.current_pattern = REGXCASE::BEGIN_GROUP_CAP; 
        re.capturing_group = true; 

        if (DEBUG)
            std::cout<< "[New Group Added] current group is: " << re.current_group << std::endl;

    } else if ( *pattern_iter == ')'){

        re.current_pattern = REGXCASE::END_GROUP_CAP; 

    } else if ( *pattern_iter == '.'){

        re.current_pattern = REGXCASE::WILDCARD; 

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

    build_regex_match(input_str, pattern_iter, re, re_in); 
    
}

bool check_regex_match(char c, struct regex &re){

    if (DEBUG){
        std::cout << "char: " << c << " with pattern: " << re.current_pattern << std::endl;
        debug_chr_set(re);
    }
    

    bool result; 
    switch (re.current_pattern){
        case REGXCASE::WILDCARD:
            return true;

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

bool check_begin_group_capture(struct regex &re){
    if (re.begin_group_capture){

        re.begin_group_capture = false; 
        return true;
    }

    return false;
}

bool check_end_group_capture(struct regex &re){

    if (re.end_group_capture){

        re.end_group_capture = false;
        return true; 
    }

    return false; 
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

    if (DEBUG)
        std::cout << "[Checking for optional]: " << std::endl;

    if (*(pattern_iter) == '?'){

        // might be some issues here when checking for a group
        if ( !current_matched )
            --input_str; 

        ++pattern_iter; 
        return true;
    }

    return false;  
}

bool check_for_alternation(str_itr &pattern_iter, str_itr &pattern_end){

    while (pattern_iter != pattern_end){

        if ( *pattern_iter == '|' )
            return true; 

        ++pattern_iter; 
    }

    return false; 
}

bool check_skip_char(struct regex &re){

    if (DEBUG)
        std::cout << "Skipping this char in the pattern" << std::endl;
    if (re.skip_char){

        re.skip_char = false; 
        return true;
    }

    return false;
}


bool match_pattern(struct regex_input &re_in){

    bool end_result = true, entry_position = false; 
    regex re;  
    init_regex(re, re_in);

    str_itr pattern_iter = std::begin(re_in.pattern);
    str_itr input_str = std::begin(re_in.input_line); 

    str_itr input_end = std::end(re_in.input_line);
    str_itr pattern_end = std::end(re_in.pattern);

    // match with first pattern that appears in the regex somewhere in the string
    parse_pattern_next(input_str, pattern_iter, re, re_in);

    
    // if pattern does not start with ^ find an entry point in the input
    if (!re.start_of_line){

        // if first parsed pattern is beginning of a group --> parse next pattern 
        if ( check_begin_group_capture(re) )
            parse_pattern_next(input_str, pattern_iter, re, re_in);
         
        while ( input_str != input_end){

            entry_position = check_regex_match(*input_str, re);

            if (DEBUG){
                std::cout << "At: " << *input_str << " -- entry found: " << entry_position << std::endl;
            } 

            ++input_str;
            if (entry_position) break; 
            
        }

        if (input_str == input_end && !entry_position){
            if (DEBUG)
                std::cout << "{ No entry point found }" << std::endl; 
            // check if next char would be alternation
            if ( !check_for_alternation(pattern_iter, pattern_end) ){
                return false; 
            } else {
                input_str = std::begin(re_in.input_line);
            }
        }

        if (entry_position)
            re.prev_matched = true; 
    }

    // continue matching

    std::cout << "[checking rest of string]" << std::endl; 
    
    bool current_matched, is_optional;
    while (end_result && input_str != input_end){

        if (pattern_iter == pattern_end){

            if (DEBUG)
                std::cout << " --[pattern is finished]-- " << std::endl; 

            break;
        } 

        parse_pattern_next(input_str, pattern_iter, re, re_in);

        // * check for one or more and if failed to match for previous
        // * if beginning of a group capture then skip over '(' and goto next char
        
        if ( ! check_begin_group_capture(re) &&
             ! check_one_or_more(re, pattern_iter, input_str) &&
             ! check_skip_char(re)
        ) {

            if (DEBUG)
                std::cout << "checking for end of capture group" << std::endl;  

            if ( !check_end_group_capture(re) ){

                if (DEBUG)
                    std::cout << "checking for match" << std::endl;

                current_matched = check_regex_match(*input_str, re);

            } else {
                struct capture_group cap_g = re.captured_groups[re.current_group]; 
                current_matched = cap_g.group_matched;
                re.current_group = cap_g.last_group; 

                if (re.current_group == 0)
                    re.capturing_group= false; 
            }

            if (DEBUG)
                std::cout << "Matched? -- " << current_matched << std::endl; 

            is_optional = check_optional(re, pattern_iter, input_str, current_matched);

            if (!re.one_or_more && !is_optional){

                if (DEBUG)
                    std::cout << "not optional or one or more" << std::endl; 


                if (re.capturing_group){

                    re.captured_groups[re.current_group].group_matched &= current_matched;

                    if (DEBUG)
                        std::cout << "Current group match status: " << re.captured_groups[re.current_group].group_matched << std::endl; 
                
                } else {
                    end_result &= current_matched;
                }
            }
                
            re.prev_matched = current_matched;

            ++input_str;
            
        }

        // check if anything toggled while hitting the end of the string
        if (input_str == input_end){

            if (DEBUG)
                std::cout << "INPUT STRING FINISHED" << std::endl; 
            
            check_one_or_more(re, pattern_iter, input_str);

            // check if next char is alternation 
            if ( *(pattern_iter) == '|'){

                bool group_match = re.captured_groups[re.current_group].group_matched;
                if (re.current_group != 0 && !group_match){
                    if (DEBUG)
                        std::cout << "ALTERNATION TO BE APPLIED" << std::endl; 
                    parse_pattern_next(input_str, pattern_iter, re, re_in);
                    re.skip_char = false;
                }

                
            }

        }
             
    }

    // check if input exhausted before pattern
    if (input_str == input_end && pattern_iter != pattern_end){

        if (DEBUG)
            std::cout << "Input exhausted before pattern" << std::endl; 

        // get last pattern to check for $ 
        parse_pattern_next(input_str, pattern_iter, re, re_in);

        // possible bug with the alternation case
        if (re.end_of_line || re.current_pattern == REGXCASE::ALTERNATION)
            end_result &= true; 
        else if (re.end_group_capture)
            end_result &= re.captured_groups[re.current_group].group_matched;
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

    struct regex_input re_in; 
    re_in.pattern = pattern;
    re_in.input_line = input_line; 
    
    try {

        if ( match_pattern(re_in) ) {
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
