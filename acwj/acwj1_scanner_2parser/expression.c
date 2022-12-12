//
// Created by jewoo on 2022-12-09.
//
#include "definitions.h"
#include "data.h"
#include "declaration.h"

// Parsing of expresssions

static ASTnode *primary(void) {
    ASTnode *node;

    //
    switch (token.token) {
        case T_INTLIT:
            node = makeASTLeaf(A_INTLIT, token.int_value);
            scan(&token);
            return node;

        default:
            fprintf(stderr, "syntax error on line %d\n", Line);
            exit(1);
    }
}

int arithOp(int tok) {
    switch (tok) {
        case T_PLUS:
            return A_ADD;
        case T_MINUS:
            return A_SUBSTRACT;
        case T_STAR:
            return A_MULTIPLY;
        case T_SLASH:
            return A_DIVIDE;
        default:
            fprintf(stderr, "unknown token in arithOp on line %d\n", Line);
            exit(1);
    }
}

// eturn an AST tree whose root is a binary operator
ASTnode *binExpr(void) {
    ASTnode *node;
    ASTnode *left;
    ASTnode *right;


    int nodeType;

    //
    //
    left = primary();

    // If no tokens left, return just the left node.
    if (token.token == T_EOF) {
        return left;
    }

    nodeType = arithOp(token.token);

    // Get the next token it.
    scan(&token);

    // Recursively get the right-hand tree
    right = binExpr();

    // Now build a tree with both sub-trees
    node = makeASTNode(nodeType, left, right, 0);
    return node;
}