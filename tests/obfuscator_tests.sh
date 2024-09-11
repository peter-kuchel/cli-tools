#!/bin/sh 


OB_BIN="./bin/obfuscator"
TEST_FILE="tests/ob_js_test.js"
TEST_FILE_NOT_SUPPORTED="tests/ob_hs_test.hs"

rm $TEST_FILE
make mk-bin-dir obfuscator

printf \
'\n\narr = [1,2,3]\n'\
'let x = 1\n'\
'arr.push(x)\n'\
'if (x < 2){\n'\
'   x++\n'\
'}\n'\
'console.log(arr)' > $TEST_FILE

# $OB_BIN $TEST_FILE_NOT_SUPPORTED

$OB_BIN $TEST_FILE
# res=$($OB_BIN $TEST_FILE)

# echo $res 