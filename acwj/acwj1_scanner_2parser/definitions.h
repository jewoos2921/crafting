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
    T_EOF,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_INTLIT
};

typedef struct token {
    int token; // Token type, from the enum list above;
    int int_value; // For T_INTLIT, the integer value
} Token;

// AST node types
enum {
    A_ADD,
    A_SUBSTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_INTLIT
};

typedef struct ASTnode {
    int op; // "Operation" to be performed on this tree.
    struct ASTnode *left;
    struct ASTnode *right;
    int intValue;
} ASTnode;

typedef char *string;

#endif //CRAFTING_DEFINITIONS_H
