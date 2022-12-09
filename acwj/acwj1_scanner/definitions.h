//
// Created by jewoo on 2022-12-09.
//

#ifndef CRAFTING_DEFINITIONS_H
#define CRAFTING_DEFINITIONS_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


// Structue and enum definitions

enum {
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_INTLIT
};

typedef struct token {
    int token;
    int int_value;
} Token;

typedef char *string;

#endif //CRAFTING_DEFINITIONS_H
