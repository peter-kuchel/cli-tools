#include <stdio.h> 


int main(int argc, char** argv){

    long a = 10; 
    long b = 20000; 

    int c = 0;

    c |= (a << 16);
    c |= b; 
    

    printf("%d\n", c);

    int d = c >> 16; // a 
    int e = c & 0xFFFF;

    printf("%d, %d\n", d,e);
    return 0;
}