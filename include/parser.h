#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef struct {
	Expr* f;
	int n_args;
	Expr* args;
} FuncCallExpr;

typedef struct {
	char op;
	Expr* l;
	Expr* r;
} OpExpr;

struct Expr {
	enum {
		EXPR_STR,
		EXPR_INT,
		EXPR_VAR,
		EXPR_CALL,
		EXPR_OP,
		EXPR_ASSIGN
	} type;
	union {
		char* s;
		FuncCallExpr call;
		OpExpr op;
	} data;
};

typedef struct {
	int n_stmts;
	Stmt* stmts;
} BlockStmt;

typedef struct {
	Expr cond;
	Stmt* if_path;
	Stmt* else_path;
} IfStmt;

typedef struct {
	char* var;
	Expr value;
} LetStmt;

struct Stmt {
	enum {
		STMT_EXPR,
		STMT_BLOCK,
		STMT_IF,
		STMT_WHILE,
		STMT_LET
	} type;
	union {
		Expr expr;
		BlockStmt block;
		IfStmt if_stmt;
		LetStmt let;
	} data;
};

Expr parse_expr(Lexer* lexer);
Stmt parse_stmt(Lexer* lexer);
void print_expr(Expr* expr);
void print_stmt(Stmt* stmt);
void free_expr(Expr* expr);
void free_stmt(Stmt* stmt);

#endif
