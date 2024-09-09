#include "grepl.h"

void create_chr_set(chr_set &c_set, const std::string &chars){

    for (auto c = std::begin(chars); c != std::end(chars); ++c)
        c_set.insert(*c); 
}

void init_regex(struct regex &re, struct regex_input &re_in){

    re.one_or_more = false; 
    re.prev_matched = false; 
    re.skip_char = false; 
    re.capturing_group = false;

    re.begin_group_capture = false;
    re.end_group_capture = false; 

    // adding the input line just so it takes index 0 
    re.captured_groups.push_back( {std::begin(re_in.pattern), "", (int)re_in.pattern.size()} );

    re.proc_stack.push_back( {std::begin(re_in.pattern), std::end(re_in.pattern), true} );
    re.curr = re.proc_stack.size() - 1;
}

void reset_regex(struct regex &re){

    re.char_set.clear();
    re.negative_group = false;  
    re.start_of_line  = false; 
    re.end_of_line = false; 

}

void build_alternation(str_itr &input_str, struct regex &re, struct regex_input &re_in){

    bool current_group_match = re.proc_stack[re.curr].group_matched;

    if (DEBUG)
        std::cout << "Alternation needed: " << !current_group_match << std::endl; 

    if ( !current_group_match ){   
        
        int i = 0; 
        str_itr current_position = re.proc_stack[re.curr].pos; 

        while ( (*(current_position + i) != '(' ) && (current_position + i) != std::begin(re_in.pattern) )
                i--; 

        if (input_str != std::begin(re_in.input_line))
            input_str += (i + 1);             // plus one to account for '(' (possible bug here)

        if (DEBUG)
            std::cout << "[ ALTERNATION ]\nmoving input back: " << ( (i+1) * -1) << " spaces" << std::endl;

        // reset the capture group if the previous alternation was false
        if (re.capturing_group)
            re.captured_groups[ re.captured_groups.size() - 1].captured_pattern.clear();

        
        // re-try for the alternation
        re.proc_stack[re.curr].group_matched = true;

        if (DEBUG){
            std::cout << "Input at: " << *input_str << std::endl;
            std::cout << "Pattern at: " << *current_position << std::endl;
        } 
    } 

    // else exit group if matched and return to next group
    else {

        if (re.capturing_group)
            re.capturing_group = false; 

        re.proc_stack.pop_back();

        if (re.proc_stack.size() != 0){
            
            re.curr = re.proc_stack.size() - 1;
            --re.proc_stack[re.curr].pos;   // to cancel out the inc coming up
        
        }
    }

    // possible bug with re.skip_char ? 
    re.skip_char = true;
    
}

void build_regex_match(str_itr &input_str, struct regex &re, struct regex_input &re_in){

    // keep state for these cases
    if (re.one_or_more)
        return; 
    
    reset_regex(re); 

    if (DEBUG){
        std::cout << "building regex pattern" << std::endl; 
    }

    switch(re.current_pattern){

        case REGXCASE::WILDCARD:
            break;

        case REGXCASE::ANY_SINGLE:
            re.char_set.insert( *re.proc_stack[re.curr].pos );
            break; 

        case REGXCASE::ALPHA_ANY_SINGLE:
            re.char_set = ALPHA_NUMERIC; 
            break; 

        case REGXCASE::DIGIT_ANY_SINGLE:
            re.char_set = DIGITS;  
            break; 

        case REGXCASE::GROUPING:
            
            if (*(re.proc_stack[re.curr].pos + 1) == '^' && *(re.proc_stack[re.curr].pos) == '['){
                re.negative_group = true;
                ++re.proc_stack[re.curr].pos; 
            } 
                    
            while (*(++re.proc_stack[re.curr].pos) != ']')
                re.char_set.insert( *(re.proc_stack[re.curr].pos) );

            break; 

        case REGXCASE::START_OF_LINE: 
            re.start_of_line = true; 
            break; 

        case REGXCASE::END_OF_LINE:
            re.end_of_line = true; 
            break;

        case REGXCASE::BEGIN_GROUP_CAP:
            re.begin_group_capture = true; 
            break;

        case REGXCASE::END_GROUP_CAP:
            --re.proc_stack[re.curr].pos;               // do this so that we can keep it if at end of pattern expression for check_end_of_group()?
            re.capturing_group = false;
            break;

        case REGXCASE::ALTERNATION:

            // assume alternation is either between () or seperates the entire pattern
            build_alternation(input_str, re, re_in);
            
            break; 

        case REGXCASE::SINGLE_BACKREF:
            // the iter after breaking from the switch should go past the '('
            re.skip_char = true;
            return;

        default:
            if (DEBUG) std::cout << "AT DEFAULT" << std::endl; 
            break; 
    }
    
    if (DEBUG) std::cout << "inc pos" << std::endl; 
    ++re.proc_stack[re.curr].pos;
}

bool parse_group_backreference(struct regex &re){

    char p = *(re.proc_stack[re.curr].pos); 

    if ( p <= '9' && p >= '0'){

        // possible bug if there are more than 9 backreferences
        int group_ref = p - '0';

        if ( group_ref > (int)re.captured_groups.size() )
            return false; 

        if (DEBUG)
            std::cout << "group ref is: " << group_ref << std::endl; 

        // inc past the reference number in the original group
        ++re.proc_stack[re.curr].pos;

        struct capture_group &g = re.captured_groups[group_ref];

        if (DEBUG){
            std::cout << "[========================================================]" << std::endl; 
            std::cout << "captured pattern is: " << g.captured_pattern << std::endl; 
        }


        re.proc_stack.push_back( { g.captured_pattern.begin(), g.captured_pattern.end(), true } );
        // re.curr = group_ref; 
        re.curr = re.proc_stack.size() - 1;

        re.current_pattern = REGXCASE::SINGLE_BACKREF;

        return true; 
    }

    return false; 
}

void parse_new_group(struct regex &re){
    int i = 1; 

    str_itr current_position = re.proc_stack[re.curr].pos; 

        // capture where the group is at
    while (*(current_position + i) != ')')
        i++; 

    // should be ordered by when they were pushed which will correspond to back reference
    re.captured_groups.push_back( { current_position, "", i } );

    // need to move iterator of current group past the )
    re.proc_stack[re.curr].pos += (i + 1);

    re.current_pattern = REGXCASE::BEGIN_GROUP_CAP; 

    re.proc_stack.push_back( {current_position, current_position + i, true} );
    re.curr = re.proc_stack.size() - 1;

    if (DEBUG){
        std::cout << "[New Group Added]: " << re.captured_groups.size() - 1 << std::endl; 
        std::cout<< "Current group being processed is: " << re.curr << std::endl;
    }

    re.capturing_group = true; 
}

void parse_next_pattern(str_itr &input_str, struct regex &re, struct regex_input &re_in){
    
    if (DEBUG)
        std::cout << "[Parsing next pattern] for ( "<< re.curr << " ) in the process stack" << std::endl; 

    str_itr pattern_pos = re.proc_stack[re.curr].pos;

    if (DEBUG)
        std::cout << "Next in pattern: " << *pattern_pos << std::endl; 

    if (*pattern_pos == '\\'){

        ++pattern_pos; ++re.proc_stack[re.curr].pos;
        switch(*pattern_pos){

            case 'w':
                re.current_pattern = REGXCASE::ALPHA_ANY_SINGLE;
                break;
            case 'd':
                re.current_pattern =  REGXCASE::DIGIT_ANY_SINGLE;
                break;

            // need to check for escape characters too 
            default: 

                if ( !parse_group_backreference(re) )
                    re.current_pattern = REGXCASE::NOT_RECOGNIZED;

                break;
        }

    // capture groups
    } else if ( *pattern_pos == '|'){

        re.current_pattern = REGXCASE::ALTERNATION;

    } else if ( *pattern_pos == '(' ){
        
        parse_new_group( re );

    } else if ( *pattern_pos == ')'){

        re.current_pattern = REGXCASE::END_GROUP_CAP; 

    } else if ( *pattern_pos == '.'){

        re.current_pattern = REGXCASE::WILDCARD; 

    } else if (*pattern_pos == '+'){ 

        re.one_or_more = true; 
        
    } else if (*pattern_pos == '['){

        re.current_pattern = REGXCASE::GROUPING; 

    } else if (*pattern_pos == '^'){

        re.current_pattern = REGXCASE::START_OF_LINE;

    } else if (*pattern_pos == '$'){

        re.current_pattern = REGXCASE::END_OF_LINE; 

    } else if ( ALL_CHARS.find( *pattern_pos ) != ALL_CHARS.end() ){

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

    build_regex_match(input_str, re, re_in); 
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

        if (DEBUG)
            std::cout << "[Beginning of group capture]" << std::endl; 

        re.begin_group_capture = false; 
        return true;
    }

    return false;
}

bool check_current_group_finished(struct regex &re){

    char last_char = *re.proc_stack[re.curr].pos;

    if ( re.proc_stack[re.curr].pos == re.proc_stack[re.curr].group_end )
        return true; 
    else if (last_char == '+' || last_char == '$')
        return true; 
    return false; 
}

bool check_one_or_more(struct regex &re, str_itr &input_str){

    if (re.one_or_more && !re.prev_matched){

            if (DEBUG){
                std::cout << "[Exiting from 1 or more]" << std::endl; 
            }

            re.one_or_more = false; 
            ++re.proc_stack[re.curr].pos;
            --input_str; 

            return true; 
    }

    return false; 
}

bool check_optional(struct regex &re, str_itr &input_str, bool current_matched){

    if (DEBUG)
        std::cout << "[Checking for optional]: " << std::endl;

    if (*(re.proc_stack[re.curr].pos) == '?'){

        // might be some issues here when checking for a group
        if ( !current_matched )
            --input_str; 

        ++re.proc_stack[re.curr].pos; 
        return true;
    }

    return false;  
}

bool check_for_alternation(struct regex &re){

    while (re.proc_stack[re.curr].pos != re.proc_stack[re.curr].group_end){

        if ( *(re.proc_stack[re.curr].pos) == '|' )
            return true; 

        ++re.proc_stack[re.curr].pos; 
    }

    return false; 
}

bool check_skip_char(struct regex &re){

    
    if (re.skip_char){

        if (DEBUG)
            std::cout << "Skipping this char in the pattern" << std::endl;

        re.skip_char = false; 
        return true;
    }

    return false;
}

bool check_current_process(struct regex &re, struct return_status &end_result){

    if (re.proc_stack[re.curr].pos == re.proc_stack[re.curr].group_end){

        if (DEBUG)
            std::cout << " --[ current pattern is finished ]-- " << std::endl; 

        end_result.result &= re.proc_stack[re.curr].group_matched; 

        if (DEBUG)
            std::cout << " group result was: " << re.proc_stack[re.curr].group_matched << std::endl; 

        if (re.capturing_group)
            re.capturing_group = false;

        re.proc_stack.pop_back();

        // if nothing in the process stack or group result was false, then finished in main loop
        if (re.proc_stack.size() == 0 || !end_result.result)
            return true;  

        re.curr = re.proc_stack.size() - 1;

        // skip over '(' if starts with that (since group is already being accounted for?)
        if ( *(re.proc_stack[re.curr].pos) == '(')
            ++re.proc_stack[re.curr].pos;

    } 

    return false;
}


bool match_pattern(struct regex_input &re_in){

    return_status end_result;
    end_result.result = true; 
    bool entry_position = false;

    regex re;  
    init_regex(re, re_in);

    if (DEBUG){
        std::cout << "\nInput string: " << re_in.input_line << std::endl; 
        std::cout << "Pattern string: " << re_in.pattern << "\n" << std::endl; 
    }

    str_itr input_str = std::begin(re_in.input_line); 
    str_itr input_end = std::end(re_in.input_line);

    // match with first pattern that appears in the regex somewhere in the string

    do {

        if (DEBUG)
            std::cout << "parsing entry pattern to account for '('" << std::endl; 

        parse_next_pattern(input_str, re, re_in);

    } while ( check_begin_group_capture(re) );
    

    
    // if pattern does not start with ^ find an entry point in the input
    if (!re.start_of_line){
         
        while ( input_str != input_end){

            entry_position = check_regex_match(*input_str, re);

            if (DEBUG){
                std::cout << "At: " << *input_str << " -- entry found: " << entry_position << std::endl;
            } 

            
            if (entry_position) break; 

            ++input_str;
            
        }

        if (input_str == input_end && !entry_position){

            if (DEBUG)
                std::cout << "{ No entry point found }" << std::endl; 
            

            // set as false since no entry was found, and check if next char would be alternation
            re.proc_stack[re.curr].group_matched = false; 
            if ( !check_for_alternation(re) ){
                return false; 

            } else {
                input_str = std::begin(re_in.input_line);
            }
        }

        if (entry_position){
            if (re.capturing_group)
                re.captured_groups[ re.captured_groups.size() - 1].captured_pattern.append(input_str, input_str+1);

            re.prev_matched = true;
            ++input_str;
        }
    }

    // continue matching

    if (DEBUG)
        std::cout << "[checking rest of string]" << std::endl; 
    
    bool current_matched, is_optional;

    // str_itr pattern_iter; 
    while (end_result.result && input_str != input_end){

        // if true then either stack is empty or last group result was false
        if ( check_current_process(re, end_result) )
            break; 

        parse_next_pattern(input_str, re, re_in);

        // * check for one or more and if failed to match for previous
        // * if beginning of a group capture then skip over '(' and goto next char
        
        if ( ! check_begin_group_capture(re) &&
             ! check_one_or_more(re, input_str) &&
             ! check_skip_char(re)
        ) {

            current_matched = check_regex_match(*input_str, re);

            if (DEBUG)
                std::cout << "Matched? -- " << current_matched << std::endl; 

            is_optional = check_optional(re, input_str, current_matched);

            if (!re.one_or_more && !is_optional){

                if (DEBUG)
                    std::cout << "not optional or one or more" << std::endl; 

                re.proc_stack[re.curr].group_matched &= current_matched;
            }
                
            re.prev_matched = current_matched;

            // possible future bug here if were to implement nested backreferences (?)
            if (re.capturing_group && current_matched)
                re.captured_groups[ re.captured_groups.size() - 1 ].captured_pattern.append(input_str, input_str+1);

            ++input_str;
            
        }

        // check if anything toggled while hitting the end of the string
        if (input_str == input_end){

            if (DEBUG)
                std::cout << "INPUT STRING FINISHED" << std::endl; 
            
            check_one_or_more(re, input_str);

            // check if next char is alternation 
            if ( *(re.proc_stack[re.curr].pos) == '|'){

                bool group_match = re.proc_stack[re.curr].group_matched;

                if (re.proc_stack.size() -1 != 0 && !group_match){

                    if (DEBUG)
                        std::cout << "ALTERNATION TO BE APPLIED" << std::endl; 

                    parse_next_pattern(input_str, re, re_in);
                    re.skip_char = false;
                }  
            }
        }      
    }

    bool last_group_match = re.proc_stack[re.curr].group_matched;

    if (DEBUG)
        std::cout << "last group matched: " << last_group_match << std::endl; 

    // check if input exhausted before pattern (since process stack has 2 or more groups)
    if (input_str == input_end && re.proc_stack.size() -1 != 0){

        if (DEBUG){
            std::cout << "Input exhausted before pattern -- proc stack size > 1" << std::endl; 
        }

        // keep checking if last group finished
        do {

            // return the status if either last group was false or were at the end of the current_process
            if ( check_current_process(re, end_result) )
                return end_result.result; 

        } while (re.proc_stack[re.curr].pos == re.proc_stack[re.curr].group_end);


        // get last pattern to check for $, |, or +  
        parse_next_pattern(input_str, re, re_in);
        
        // possible bug with the alternation case (?)
        if (re.end_of_line || re.current_pattern == REGXCASE::ALTERNATION || re.one_or_more){
            
            if ( check_current_process(re, end_result) ) 
                end_result.result &= true;

        } else 
            end_result.result &= false;

    } 

    // if input was exhausted and last pattern group hasn't finished 
    else if (input_str == input_end && re.proc_stack.size() == 1 ){

        if (DEBUG){
            std::cout << "Input exhausted before pattern -- last proc unfinished" << std::endl; 
        }
        bool proc_fini = check_current_group_finished(re); 

        if (DEBUG)
            std::cout << "Finished? : " << proc_fini << std::endl; 

        // check if proc finished or if in a mode such as matching one or more
        if ( proc_fini  || re.one_or_more)
            end_result.result &= last_group_match;
        else
            end_result.result &= false; 
    }
    
    return end_result.result;
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
