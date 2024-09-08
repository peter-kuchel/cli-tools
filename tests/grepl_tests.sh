#!/bin/sh

make mk-bin-dir grepl
echo ""

# GREPL_BIN='./bin/grepldebug'
GREPL_BIN='./bin/grepl'

# Tests taken from code crafters tests for each stage

#test result 
MATCH_FOUND="match found"
MATCH_NOT_FOUND="match NOT found"

check_match()
{

	str=$1
	pat=$2
	
	eval exp="$3"

	res=$(echo -n $str | $GREPL_BIN -E $pat)
	printf "Input is: " && printf $str && printf " [&&] Regex is: " && printf $pat && printf "\n"

	printf "Expected: " && printf "${exp}" && printf " | got: " && printf "${res}\n"

	if [ "${res}" != "${exp}" ]
	then 
		echo "exiting tests early..."
		exit 1
	fi 	

	echo ""


}

# Backreferences tests 
# printf "input str is: " && printf "cat and cat\n" && printf "regex pattern is: " && printf "(cat) and \\1 " && printf "\n"
# (printf "Expected: match found | got: ") && 
# RESULT=$(echo -n "cat and cat" | $GREPL_BIN -E "(cat) and \1")

# echo $RESULT
# echo ""
# check_match $RESULT $MATCH_FOUND

# printf "input str is: " && printf "cat and dog\n" && printf "regex pattern is: " && printf "(cat) and \\ 1 " && printf "\n"
# (printf "Expected: match found | got: ") &&
# RESULT=$(echo -n "cat and dog" | $GREPL_BIN -E "(cat) and \1")

# echo $RESULT

# echo -n "3 red squares and 3 red circles" | ./your_program.sh -E "(\d+) (\w+) squares and \1 \2 circles"
# echo -n "3 red squares and 4 red circles" | ./your_program.sh -E "(\d+) (\w+) squares and \1 \2 circles"

# check_match "$RESULT" "$MATCH_NOT_FOUND"

# echo -n "grep 101 is doing grep 101 times" | $GREPL_BIN -E "(\w\w\w\w \d\d\d) is doing \1 times"
# echo -n "$?! 101 is doing $?! 101 times" | $GREPL_BIN -E "(\w\w\w \d\d\d) is doing \1 times"
# echo -n "grep yes is doing grep yes times" | $GREPL_BIN -E "(\w\w\w\w \d\d\d) is doing \1 times"
# echo -n "abcd is abcd, not efg" | $GREPL_BIN -E "([abcd]+) is \1, not [^xyz]+"
# echo -n "efgh is efgh, not efg" | $GREPL_BIN -E "([abcd]+) is \1, not [^xyz]+"
# echo -n "abcd is abcd, not xyz" | $GREPL_BIN -E "([abcd]+) is \1, not [^xyz]+"
# echo -n "this starts and ends with this" | $GREPL_BIN -E "^(\w+) starts and ends with \1$"
# echo -n "that starts and ends with this" | $GREPL_BIN -E "^(this) starts and ends with \1$"
# echo -n "this starts and ends with this?" | $GREPL_BIN -E "^(this) starts and ends with \1$"
# echo -n "once a dreaaamer, always a dreaaamer" | $GREPL_BIN -E "once a (drea+mer), alwaysz? a \1"
# echo -n "once a dremer, always a dreaaamer" | $GREPL_BIN -E "once a (drea+mer), alwaysz? a \1"
# echo -n "once a dreaaamer, alwayszzz a dreaaamer" | $GREPL_BIN -E "once a (drea+mer), alwaysz? a \1"
# echo -n "bugs here and bugs there" | $GREPL_BIN -E "(b..s|c..e) here and \1 there"
# echo -n "bugz here and bugs there" | $GREPL_BIN -E "(b..s|c..e) here and \1 there"

# # Alternation tests 
# echo -n "a cat" | $GREPL_BIN -E "a (cat|dog)"
# echo -n "a dog" | $GREPL_BIN -E "a (cat|dog)"
# echo -n "a cow" | $GREPL_BIN -E "a (cat|dog)"

printf "--- Wildcard tests ---\n"
check_match "cat" "c.t" "\${MATCH_FOUND}"
check_match "cot" "c.t" "\${MATCH_FOUND}"
check_match "car" "c.t" "\${MATCH_NOT_FOUND}"

# echo -n "cot" | $GREPL_BIN -E "c.t"
# echo -n "car" | $GREPL_BIN -E "c.t"

# # Match 0 or 1 times 
# echo -n "cat" | $GREPL_BIN -E "ca?t"
# echo -n "act" | $GREPL_BIN -E "ca?t"
# echo -n "dog" | $GREPL_BIN -E "ca?t"
# echo -n "cag" | $GREPL_BIN -E "ca?t"

# # Match 1 or more times 
# echo -n "caaats" | $GREPL_BIN -E "ca+t"
# echo -n "cat" | $GREPL_BIN -E "ca+t"
# echo -n "act" | $GREPL_BIN -E "ca+t"

# # End of string anchor
# echo -n "cat" | $GREPL_BIN -E "cat$"
# echo -n "cats" | $GREPL_BIN -E "cat$"

# # Beginning of string anchor
# echo -n "log" | $GREPL_BIN -E "^log"
# echo -n "slog" | $GREPL_BIN -E "^log"

# # Combining character classes
# echo -n "sally has 3 apples" | $GREPL_BIN -E "\d apple"
# echo -n "sally has 1 orange" | $GREPL_BIN -E "\d apple"
# echo -n "sally has 124 apples" | $GREPL_BIN -E "\d\d\d apples"
# echo -n "sally has 12 apples" | $GREPL_BIN -E "\d\\d\\d apples"			# might have passed for the wrong reason
# echo -n "sally has 3 dogs" | $GREPL_BIN -E "\d \w\w\ws"
# echo -n "sally has 4 dogs" | $GREPL_BIN -E "\d \w\w\ws"
# echo -n "sally has 1 dog" | $GREPL_BIN -E "\d \w\w\ws"

# # Negative character groups
# echo -n "apple" | $GREPL_BIN -E "[^xyz]"
# echo -n "banana" | $GREPL_BIN -E "[^anb]"

# # Positive character groups
# echo -n "a" | $GREPL_BIN -E "[abcd]"
# echo -n "efgh" | $GREPL_BIN -E "[abcd]"

# # Match alphanumeric 
# echo -n "word" | $GREPL_BIN -E "\w"
# echo -n "$!?" | $GREPL_BIN -E "\w"

# # Match digits
# echo -n "123" | $GREPL_BIN -E "\d"
# echo -n "apple" | $GREPL_BIN -E "\d"

# # Match character literal 
# echo -n "dog" | $GREPL_BIN -E "d"
# echo -n "dog" | $GREPL_BIN -E "f"
