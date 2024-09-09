#!/bin/sh

# GREPL_BIN='./bin/grepldebug'
GREPL_BIN='./bin/grepl'

# Tests taken from code crafters tests for each stage

#test result 
MATCH_FOUND="match found"
MATCH_NOT_FOUND="match NOT found"

# check_match <input str> <pattern> <expected result>

check_match()
{

	str=$1
	pat=$2
	
	eval exp="$3"

	echo "echo -n \""$str"\" |" $GREPL_BIN "-E \""$pat"\""

	res=$(echo -n "$str"| $GREPL_BIN -E "$pat")

	# echo $res

	printf "Expected: " && printf "${exp}" && printf " | got: " && printf "${res}\n"

	if [ "${res}" != "${exp}" ]
	then 
		echo "exiting tests early..."
		exit 1
	fi 	

	echo ""


}

make mk-bin-dir grepl
echo ""

echo "[=== Code Crafter Tests ===]"
echo ""

echo "---[ Multiple Backreferences tests ]---"
check_match "3 red squares and 3 red circles" "(\d+) (\w+) squares and \1 \2 circles" "\${MATCH_FOUND}"
check_match "3 red squares and 4 red circles" "(\d+) (\w+) squares and \1 \2 circles" "\${MATCH_NOT_FOUND}"
check_match "grep 101 is doing grep 101 times" "(\w\w\w\w) (\d\d\d) is doing \1 \2 times" "\${MATCH_FOUND}"
check_match "\$?! 101 is doing \$?! 101 times" "(\w\w\w) (\d\d\d) is doing \1 \2 times" "\${MATCH_NOT_FOUND}"
check_match "grep yes is doing grep yes times" "(\w\w\w\w) (\d\d\d) is doing \1 \2 times" "\${MATCH_NOT_FOUND}"
check_match "abc-def is abc-def, not efg" "([abc]+)-([def]+) is \1-\2, not [^xyz]+" "\${MATCH_FOUND}"
check_match "efg-hij is efg-hij, not efg" "([abc]+)-([def]+) is \1-\2, not [^xyz]+" "\${MATCH_NOT_FOUND}"
check_match "abc-def is abc-def, not xyz" "([abc]+)-([def]+) is \1-\2, not [^xyz]+" "\${MATCH_NOT_FOUND}"
check_match "apple pie, apple and pie" "^(\w+) (\w+), \1 and \2$" "\${MATCH_FOUND}"
check_match "pineapple pie, pineapple and pie" "^(apple) (\w+), \1 and \2$" "\${MATCH_NOT_FOUND}"
check_match "apple pie, apple and pies" "^(\w+) (pie), \1 and \2$" "\${MATCH_NOT_FOUND}"
check_match "howwdy hey there, howwdy hey" "(how+dy) (he?y) there, \1 \2" "\${MATCH_FOUND}"
check_match "hody hey there, howwdy hey" "(how+dy) (he?y) there, \1 \2" "\${MATCH_NOT_FOUND}"
check_match "howwdy heeey there, howwdy heeey" "(how+dy) (he?y) there, \1 \2" "\${MATCH_NOT_FOUND}"
check_match "cat and fish, cat with fish" "(c.t|d.g) and (f..h|b..d), \1 with \2" "\${MATCH_FOUND}"
check_match "bat and fish, cat with fish" "(c.t|d.g) and (f..h|b..d), \1 with \2" "\${MATCH_NOT_FOUND}"

echo "---[ Single Backreferences tests ]---"
check_match "grep 101 is doing grep 101 times" "(\w\w\w\w \d\d\d) is doing \1 times" "\${MATCH_FOUND}"
check_match "\$?! 101 is doing \$?! 101 times" "(\w\w\w \d\d\d) is doing \1 times" "\${MATCH_NOT_FOUND}"
check_match "grep yes is doing grep yes times" "(\w\w\w\w \d\d\d) is doing \1 times" "\${MATCH_NOT_FOUND}"
check_match "abcd is abcd, not efg" "([abcd]+) is \1, not [^xyz]+" "\${MATCH_FOUND}"
check_match "efgh is efgh, not efg" "([abcd]+) is \1, not [^xyz]+" "\${MATCH_NOT_FOUND}"
check_match "abcd is abcd, not xyz" "([abcd]+) is \1, not [^xyz]+" "\${MATCH_NOT_FOUND}"
check_match "this starts and ends with this" "^(\w+) starts and ends with \1$" "\${MATCH_FOUND}"
check_match "that starts and ends with this" "^(this) starts and ends with \1$" "\${MATCH_NOT_FOUND}"
check_match "this starts and ends with this?" "^(this) starts and ends with \1$" "\${MATCH_NOT_FOUND}"
check_match "once a dreaaamer, always a dreaaamer" "once a (drea+mer), alwaysz? a \1" "\${MATCH_FOUND}"
check_match "once a dremer, always a dreaaamer" "once a (drea+mer), alwaysz? a \1" "\${MATCH_NOT_FOUND}"
check_match "once a dreaaamer, alwayszzz a dreaaamer" "once a (drea+mer), alwaysz? a \1" "\${MATCH_NOT_FOUND}"
check_match "bugs here and bugs there" "(b..s|c..e) here and \1 there" "\${MATCH_FOUND}"
check_match "bugz here and bugs there" "(b..s|c..e) here and \1 there" "\${MATCH_NOT_FOUND}"

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

echo "[=== Custom Tests ===]"
echo ""

check_match "yes yes and no with yes no but yes but no" "(\w+) \1 and (\w+) with \1 \2 (\w+) \1 \3 \2" "\${MATCH_FOUND}"
check_match "no nope nah and nor are the same as nah nor no and nope" "(\w+) (\w+) (\w+) and (\w+) are the same as \3 \4 \1 and \2" "\${MATCH_FOUND}"
check_match "thiszszsz is the same as thiszszsz" "thi([sz]+) is the same as thi\1" "\${MATCH_FOUND}"
check_match "dug and bund, dug with bund" "(c.t|d.g) (\w+) (f..h|b..d), \1 (\w+) \3" "\${MATCH_FOUND}"
check_match "3m is about 10ft" "(\d+[mft]+) is (\w+) (\d+[mft]+)"
# check_match ", sure" "(c.t|d.g)?, sure" "\${MATCH_FOUND}"

echo "[=== All Tests Passed! ===]"