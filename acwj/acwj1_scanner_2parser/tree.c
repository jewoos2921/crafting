//
// Created by jewoo on 2022-12-09.
//

#include "definitions.h"
#include "data.h"
#include "declaration.h"

// AST tree functions

// Build and return a generic AST node.
ASTnode *makeASTNode(int op, ASTnode *left,
                     ASTnode *right, int intValue) {
    ASTnode *node;

    // Malloc a new ASTnode
    node = (ASTnode *) malloc(sizeof(ASTnode));
    if (node == NULL) {
        fprintf(stderr, "unable to malloc in makeASTNode\n");
        exit(1);
    }

    // Copy in the field values and return it
    node->op = op;
    node->left = left;
    node->right = right;
    node->intValue = intValue;
    return node;
}

// Make an AST leaf node
ASTnode *makeASTLeaf(int op, int intValue) {
    return makeASTNode(op, NULL, NULL, intValue);
}

ASTnode *makeASTUnary(int op, ASTnode *left, int intValue) {
    return makeASTNode(op, left, NULL, intValue);
}
