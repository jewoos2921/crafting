//
// Created by jewoo on 2022-12-09.
//

#ifndef CRAFTING_DECLARATION_H
#define CRAFTING_DECLARATION_H


int scan(Token *t);

ASTnode *makeASTNode(int op, ASTnode *left,
                     ASTnode *right, int intValue);

ASTnode *makeASTLeaf(int op, int intValue);

ASTnode *makeASTUnary(int op, ASTnode *left, int intValue);

ASTnode *binExpr(void);

int interpretAST(ASTnode *node);

#endif //CRAFTING_DECLARATION_H
