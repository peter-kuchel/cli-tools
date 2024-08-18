#!/bin/sh

make mk-bin-dir grepl

GREPL_BIN='./bin/grepl'

# Tests taken from code crafters tests for each stage

# Alternation tests 
echo -n "a cat" | $GREPL_BIN -E "a (cat|dog)"
echo -n "a dog" | $GREPL_BIN -E "a (cat|dog)"
echo -n "a cow" | $GREPL_BIN -E "a (cat|dog)"

# Wildcard tests 
echo -n "cat" | $GREPL_BIN -E "c.t"
echo -n "cot" | $GREPL_BIN -E "c.t"
echo -n "car" | $GREPL_BIN -E "c.t"

# Match 0 or 1 times 
echo -n "cat" | $GREPL_BIN -E "ca?t"
echo -n "act" | $GREPL_BIN -E "ca?t"
echo -n "dog" | $GREPL_BIN -E "ca?t"
echo -n "cag" | $GREPL_BIN -E "ca?t"

# Match 1 or more times 
echo -n "caaats" | $GREPL_BIN -E "ca+t"
echo -n "cat" | $GREPL_BIN -E "ca+t"
echo -n "act" | $GREPL_BIN -E "ca+t"

# End of string anchor
echo -n "cat" | $GREPL_BIN -E "cat$"
echo -n "cats" | $GREPL_BIN -E "cat$"

# Beginning of string anchor
echo -n "log" | $GREPL_BIN -E "^log"
echo -n "slog" | $GREPL_BIN -E "^log"

# Combining character classes
echo -n "sally has 3 apples" | $GREPL_BIN -E "\d apple"
echo -n "sally has 1 orange" | $GREPL_BIN -E "\d apple"
echo -n "sally has 124 apples" | $GREPL_BIN -E "\d\d\d apples"
echo -n "sally has 12 apples" | $GREPL_BIN -E "\d\\d\\d apples"			# might have passed for the wrong reason
echo -n "sally has 3 dogs" | $GREPL_BIN -E "\d \w\w\ws"
echo -n "sally has 4 dogs" | $GREPL_BIN -E "\d \w\w\ws"
echo -n "sally has 1 dog" | $GREPL_BIN -E "\d \w\w\ws"

# Negative character groups
echo -n "apple" | $GREPL_BIN -E "[^xyz]"
echo -n "banana" | $GREPL_BIN -E "[^anb]"

# Positive character groups
echo -n "a" | $GREPL_BIN -E "[abcd]"
echo -n "efgh" | $GREPL_BIN -E "[abcd]"

# Match alphanumeric 
echo -n "word" | $GREPL_BIN -E "\w"
echo -n "$!?" | $GREPL_BIN -E "\w"

# Match digits
echo -n "123" | $GREPL_BIN -E "\d"
echo -n "apple" | $GREPL_BIN -E "\d"

# Match character literal 
echo -n "dog" | $GREPL_BIN -E "d"
echo -n "dog" | $GREPL_BIN -E "f"



