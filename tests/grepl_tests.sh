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

	echo "echo -n" $str "|" $GREPL_BIN "-E" $pat

	res=$(echo -n "$str"| $GREPL_BIN -E "$pat")

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


# check_match <input str> <pattern> <expected result>


echo "---[ Alternation tests ]---"
check_match "a cat" "a (cat|dog)" "\${MATCH_FOUND}"
check_match "a dog" "a (cat|dog)" "\${MATCH_FOUND}"
check_match "a cow" "a (cat|dog)" "\${MATCH_NOT_FOUND}"

echo "---[ Wildcard tests ]---"
check_match "cat" "c.t" "\${MATCH_FOUND}"
check_match "cot" "c.t" "\${MATCH_FOUND}"
check_match "car" "c.t" "\${MATCH_NOT_FOUND}"

echo "---[ Match 0 or 1 times tests ]---"
check_match "cat" "ca?t" "\${MATCH_FOUND}"
check_match "act" "ca?t" "\${MATCH_FOUND}"
check_match "dog" "ca?t" "\${MATCH_NOT_FOUND}"
check_match "cag" "ca?t" "\${MATCH_NOT_FOUND}"

echo "---[ Match 1 or more times tests ]---"
check_match "caaats" "ca+t" "\${MATCH_FOUND}"
check_match "cat" "ca+t" "\${MATCH_FOUND}"
check_match "act" "ca+t" "\${MATCH_NOT_FOUND}"

echo "---[ End of string anchor tests]---"
check_match "cat" "cat$" "\${MATCH_FOUND}"
check_match "cats" "cat$" "\${MATCH_NOT_FOUND}"

echo "---[ Beginning of string anchor tests ]---"
check_match "log" "^log" "\${MATCH_FOUND}"
check_match "slog" "^log" "\${MATCH_NOT_FOUND}"

echo "---[ Combining character classes tests ]---"
check_match "sally has 3 apples" "\d apple" "\${MATCH_FOUND}"
check_match "sally has 1 orange" "\d apple" "\${MATCH_NOT_FOUND}"
check_match "sally has 124 apples" "\d\d\d apples" "\${MATCH_FOUND}"
check_match "sally has 12 apples" "\d\\d\\d apples"	"\${MATCH_NOT_FOUND}"		# might have passed for the wrong reason
check_match "sally has 3 dogs" "\d \w\w\ws" "\${MATCH_FOUND}"
check_match "sally has 4 dogs" "\d \w\w\ws" "\${MATCH_FOUND}"
check_match "sally has 1 dog" "\d \w\w\ws" "\${MATCH_NOT_FOUND}"

echo "---[ Negative character groups tests ]---"
check_match "apple" "[^xyz]" "\${MATCH_FOUND}"
check_match "banana" "[^anb]" "\${MATCH_NOT_FOUND}"

echo "---[ Positive character groups tests ]---"
check_match "a" "[abcd]" "\${MATCH_FOUND}"
check_match "efgh" "[abcd]" "\${MATCH_NOT_FOUND}"
 
echo "---[ Match alphanumeric tests ]---"
check_match "word" "\w" "\${MATCH_FOUND}"
check_match "\$!?" "\w" "\${MATCH_NOT_FOUND}"

echo "---[ Match digits tests ]---"
check_match "123" "\d" "\${MATCH_FOUND}"
check_match "apple" "\d" "\${MATCH_NOT_FOUND}"

echo "---[ Match character literal tests ]---"
check_match "dog" "d" "\${MATCH_FOUND}"
check_match "dog" "f" "\${MATCH_NOT_FOUND}"


echo "[=== All Tests Passed! ===]"