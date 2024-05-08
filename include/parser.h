#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

typedef struct expr_t expr_t;
typedef struct stmt_t stmt_t;

typedef struct {
	expr_t* f;
	int n_args;
	expr_t* args;
} func_call_t;

typedef struct {
	char op;
	expr_t* l;
	expr_t* r;
} op_t;

struct expr_t {
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
		func_call_t call;
		op_t op;
	} data;
};

typedef struct {
	int n_stmts;
	stmt_t* stmts;
} block_t;

typedef struct {
	expr_t cond;
	stmt_t* if_path;
	stmt_t* else_path;
} if_t;

typedef struct {
	char* var;
	expr_t value;
} let_t;

struct stmt_t {
	enum {
		STMT_EXPR,
		STMT_BLOCK,
		STMT_IF,
		STMT_WHILE,
		STMT_LET
	} type;
	union {
		expr_t expr;
		block_t block;
		if_t if_stmt;
		let_t let;
	} data;
};

expr_t parse_expr(lexer_t* lexer);
stmt_t parse_stmt(lexer_t* lexer);
void print_expr(expr_t* expr);
void print_stmt(stmt_t* stmt);
void free_expr(expr_t* expr);
void free_stmt(stmt_t* stmt);

#endif
