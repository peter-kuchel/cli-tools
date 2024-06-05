#!/bin/sh

DIR="/home/ubuntu/peter-kuchel.github.io"

PORT=8000
HOST="2605:fd00:4:1000:f816:3eff:fe88:8d3"

TEST_URL=[$HOST]:$PORT
CURL_OPTS=-sIXGET  # -s -I -XGET

function client_requests(){

    sleep 1
    printf "\nClient side testing -- beginning momentarily"
    sleep 3
    
    printf "\ntesting: get / " $HOST:$PORT
    sleep 1 
    curl $CURL_OPTS "http://$TEST_URL" 

    sleep 1 
    printf "\ntesting getting js and css\n\n"
    sleep 1 
    printf "requesting js\n"
    curl $CURL_OPTS "http://$TEST_URL/script.js"
    sleep 1 
    printf "requesting css\n" 
    curl $CURL_OPTS "http://$TEST_URL/style.css"  


    printf "finished tests"
}

# create bin and compile 
make mk-bin-dir minserver 

# run the server 
./bin/minserver -p $PORT -d $DIR -h $HOST -ipv6 & client_requests 