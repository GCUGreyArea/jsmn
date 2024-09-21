#include "jsmn.hpp"
#include <stdio.h>
#include <string.h>

int main(int argc, char ** argv) {
    if(argc == 1) return -1;

    jsmn_parser p(argv[1]);

    int count = p.parse();
    if(count < 0) {
        return -1;
    }
    
    for(unsigned int i = 0;i < count; i++) {
        p.print_token(i);
    }

    p.serialise("test.bin");
    p.init();
    p.deserialise("test.bin");

    count = p.parsed_tokens();
    for(unsigned int i = 0;i < count; i++) {
        p.print_token(i);
    }


    return 0;
}
