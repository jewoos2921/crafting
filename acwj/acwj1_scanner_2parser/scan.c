//
// Created by jewoo on 2022-12-09.
//

#include "definitions.h"
#include "data.h"
#include "declaration.h"

// Lexical Scanning

// Return the position of character c
// in string s, or -1 if c not found
static int chrpos(string s, int c) {
    char *p = strchr(s, c);

    return (int) (p ? p - s : -1);
}

// Get the next character from the input file.
static int next(void) {
    int c;

    if (PutBack) {
        c = PutBack;
        PutBack = 0;
        return c;
    }

    c = fgetc(Infile);
    if ('\n' == c) {
        Line++;
    }

    return c;
}

// Put back an unwanted character
static void putBack(int c) {
    PutBack = c;
}

// skip past input that we don't need to deal with, i.e.
// whitespae, newlines.
// Return the first character we do need to deal with.
static int skip(void) {
    int c = next();

    while (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c) {
        c = next();
    }

    return c;
}

// Scan and return an integer literal value from the input file.
// Store the value as a string in Text.
static int scanInt(int c) {
    int k;
    int val = 0;

    // Convert each character into an int value
    while ((k = chrpos("0123456789", c)) >= 0) {
        val = val * 10 + k;
        c = next();
    }

    // We hit a non-integer character, put it back
    putBack(c);
    return val;
}

// Scan and return the next token found in the input.
// Return 1 if token valid, 0 if no tokens left.
int scan(Token *t) {
    int c;

    //
    c = skip();

    //
    switch (c) {
        case EOF:
            return 0;
        case '+':
            t->token = T_PLUS;
            break;
        case '-':
            t->token = T_MINUS;
            break;
        case '*':
            t->token = T_STAR;
            break;
        case '/':
            t->token = T_SLASH;
            break;
        default:

            // if it's a digit scan the literal integer value in
            if (isdigit(c)) {
                t->int_value = scanInt(c);
                t->token = T_INTLIT;
                break;
            }

            printf("Unrecognised character %c on line %d\n", c, Line);
            exit(1);
    }

    // We found a token
    return 1;
}