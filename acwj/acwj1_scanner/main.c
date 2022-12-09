//
// Created by jewoo on 2022-12-09.
//

#include "definitions.h"
#include "data.h"
#include "declaration.h"
#include <errno.h>

// Compiler setup and top-level execution

// Initialise global variables
static void init(void) {
    Line = 1;
    PutBack = '\n';
}

// Print out a usage if started incorrectly
static void usage(string prog) {
    fprintf(stderr, "Usage: %s infile\n", prog);
    exit(1);
}

// List of printable tokens
string tok_str[] = {"+", "-", "*", "/", "intlit"};

// Loop scanning in all the tokens in the input file.
// Print out details of each token found.
static void scanFile() {
    Token T;

    while (scan(&T)) {
        printf("Token %s", tok_str[T.token]);
        if (T.token == T_INTLIT) {
            printf(", value %d", T.int_value);
        }
        printf("\n");
    }
}

// check arguments and print a usage
// if we don't have argument.
// Open up the input file and call sacnFile() to scan the tokens in it.
int main(int argc, string argv[]) {
    if (argc != 2) {
        usage(argv[0]);
    }

    init();

    if ((Infile = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open %s\n", argv[1], strerror(errno));
        exit(1);
    }

    scanFile();
    exit(0);
}