#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
#include "parser.h"

const char unops[] = "+-~!";

static char temp[256];

void eat(lexer_t* lexer, char c, char* buffer, int size) {
	if (c != lex(lexer, buffer, size)) {
		const char* token_name;
		if (c < ' ') {
			token_name = token_names[(int) c];
		} else {
			char* s = malloc(2);
			s[0] = c;
			s[1] = 0;
			token_name = s;
		}
		fprintf(stderr, "expected %s, got %s\n", token_name, buffer);
		exit(-1);
	}
}

expr_t parse_expr_inner(lexer_t* lexer) {
	expr_t e;
	char* s = malloc(256);
	char type = lex(lexer, s, 256);
	if (type == TOKEN_INT) {
		e.type = EXPR_INT;
		e.data.s = s;
	} else if (type == TOKEN_STR) {
		e.type = EXPR_STR;
		e.data.s = s;
	} else if (type == TOKEN_ID) {
		e.type = EXPR_VAR;
		e.data.s = s;
		while (peek(lexer) == '=') {
			eat(lexer, '=', temp, 256);
			op_t op;
			op.l = malloc(sizeof(expr_t));
			op.r = malloc(sizeof(expr_t));
			*op.l = e;
			*op.r = parse_expr(lexer);
			e.type = EXPR_ASSIGN;
			e.data.op = op;
		}
	} else {
		//eat(lexer, -1, temp, 256);
		fprintf(stderr, "unexpected token %s\n", s);
		exit(-1);
	}
	return e;
}

expr_t parse_expr_priority(lexer_t* lexer, int priority) {
	if (priority == 0) {
		return parse_expr_inner(lexer);
	} else if (priority == 1) {
		expr_t e = parse_expr_priority(lexer, priority-1);
		while (peek(lexer) == '(') {
			eat(lexer, '(', temp, 256);
			int cap = 4;
			int size = 0;
			expr_t* args = malloc(cap * sizeof(expr_t));
			while (peek(lexer) != ')') {
				if (cap == size) {
					cap *= 2;
					args = realloc(args, cap * sizeof(expr_t));
				}
				args[size++] = parse_expr(lexer);
				if (peek(lexer) != ')')
					eat(lexer, ',', temp, 256);
			}
			eat(lexer, ')', temp, 256);
			func_call_t call;
			call.f = malloc(sizeof(expr_t));
			*call.f = e;
			call.n_args = size;
			call.args = args;
			e.type = EXPR_CALL;
			e.data.call = call;
		}
		return e;
	} else if (priority == 2) {
		char type = lex(lexer, temp, 256);
		char c = temp[0];
		bool op_found = false;
		for (int i=0; unops[i]; i++) {
			if (unops[i] == c) {
				op_found = true;
				break;
			}
		}
		if (!op_found) {
			unlex(lexer, type, temp);
			return parse_expr_priority(lexer, priority-1);
		}
		op_t op;
		op.op = c;
		op.l = malloc(sizeof(expr_t));
		op.r = NULL;
		*op.l = parse_expr_priority(lexer, priority);
		expr_t e;
		e.type = EXPR_OP;
		e.data.op = op;
		return e;
	}
	return (expr_t) {.type=0};
}

expr_t parse_expr(lexer_t* lexer) {
	return parse_expr_priority(lexer, 2);
}

stmt_t parse_stmt(lexer_t* lexer) {
	stmt_t stmt;
	if (peek(lexer) == '{') {
		eat(lexer, '{', temp, 256);
		int cap = 4;
		int size = 0;
		stmt_t* stmts = malloc(cap * sizeof(stmt_t));
		while (peek(lexer) != '}') {
			if (cap == size) {
				cap *= 2;
				stmts = realloc(stmts, cap * sizeof(stmt_t));
			}
			stmts[size++] = parse_stmt(lexer);
		}
		eat(lexer, '}', temp, 256);
		block_t block;
		block.n_stmts = size;
		block.stmts = stmts;
		stmt.type = STMT_BLOCK;
		stmt.data.block = block;
	} else if (peek(lexer) == TOKEN_IF) {
		if_t if_stmt;
		eat(lexer, TOKEN_IF, temp, 256);
		eat(lexer, '(', temp, 256);
		if_stmt.cond = parse_expr(lexer);
		eat(lexer, ')', temp, 256);
		if_stmt.if_path = malloc(sizeof(stmt_t));
		*if_stmt.if_path = parse_stmt(lexer);
		if (peek(lexer) == TOKEN_ELSE) {
			eat(lexer, TOKEN_ELSE, temp, 256);
			if_stmt.else_path = malloc(sizeof(stmt_t));
			*if_stmt.else_path = parse_stmt(lexer);
		} else {
			if_stmt.else_path = NULL;
		}
		stmt.type = STMT_IF;
		stmt.data.if_stmt = if_stmt;
	} else if (peek(lexer) == TOKEN_WHILE) {
		if_t if_stmt;
		eat(lexer, TOKEN_WHILE, temp, 256);
		eat(lexer, '(', temp, 256);
		if_stmt.cond = parse_expr(lexer);
		eat(lexer, ')', temp, 256);
		if_stmt.if_path = malloc(sizeof(stmt_t));
		*if_stmt.if_path = parse_stmt(lexer);
		if_stmt.else_path = NULL;
		stmt.type = STMT_WHILE;
		stmt.data.if_stmt = if_stmt;
	} else if (peek(lexer) == TOKEN_LET) {
		let_t let;
		eat(lexer, TOKEN_LET, temp, 256);
		let.var = malloc(256);
		eat(lexer, TOKEN_ID, let.var, 256);
		eat(lexer, '=', temp, 256);
		let.value = parse_expr(lexer);
		eat(lexer, ';', temp, 256);
		stmt.type = STMT_LET;
		stmt.data.let = let;
	} else {
		stmt.type = STMT_EXPR;
		stmt.data.expr = parse_expr(lexer);
		eat(lexer, ';', temp, 256);
	}
	return stmt;
}

void print_expr(expr_t* e) {
	switch (e->type) {
		case EXPR_INT:
			printf("int(%s)", e->data.s);
			break;
		case EXPR_STR:
			printf("str(%s)", e->data.s);
			break;
		case EXPR_VAR:
			printf("var(%s)", e->data.s);
			break;
		case EXPR_CALL:
			printf("call(");
			print_expr(e->data.call.f);
			printf(": ");
			for (int i=0; i<e->data.call.n_args; i++) {
				print_expr(e->data.call.args + i);
				printf("; ");
			}
			printf(")");
			break;
		default:
			fprintf(stderr, "unknown expression in print_expr\n");
			exit(-1);
	}
}

void print_stmt(stmt_t* stmt) {
	switch (stmt->type) {
		case STMT_EXPR:
			print_expr(&stmt->data.expr);
			printf("; \n");
			break;
		case STMT_BLOCK:
			printf("{\n");
			for (int i=0; i<stmt->data.block.n_stmts; i++)
				print_stmt(stmt->data.block.stmts + i);
			printf("}\n");
			break;
		case STMT_LET:
			printf("let %s = ", stmt->data.let.var);
			print_expr(&stmt->data.let.value);
			putchar('\n');
			break;
		case STMT_WHILE:
			printf("while ");
			print_expr(&stmt->data.if_stmt.cond);
			printf("{\n");
			print_stmt(stmt->data.if_stmt.if_path);
			printf("}\n");
			break;
		default:
			fprintf(stderr, "unknown statement in print_stmt\n");
			exit(-1);
	}
}

void free_expr(expr_t* e) {
	switch (e->type) {
		case EXPR_STR: case EXPR_INT: case EXPR_VAR:
			free(e->data.s);
			break;
		case EXPR_CALL:
			free_expr(e->data.call.f);
			free(e->data.call.f);
			for (int i=0; i<e->data.call.n_args; i++)
				free_expr(e->data.call.args + i);
			free(e->data.call.args);
			break;
		case EXPR_OP: case EXPR_ASSIGN:
			free_expr(e->data.op.l);
			free(e->data.op.l);
			if (e->data.op.r) {
				free_expr(e->data.op.r);
				free(e->data.op.r);
			}
			break;
		default:
			fprintf(stderr, "unknown expression in free_expr\n");
			exit(-1);
	}
}

void free_stmt(stmt_t* stmt) {
	switch (stmt->type) {
		case STMT_EXPR:
			free_expr(&stmt->data.expr);
			break;
		case STMT_BLOCK:
			for (int i=0; i<stmt->data.block.n_stmts; i++)
				free_stmt(stmt->data.block.stmts + i);
			free(stmt->data.block.stmts);
			break;
		case STMT_IF: case STMT_WHILE:
		{
			if_t* if_stmt = &stmt->data.if_stmt;
			free_expr(&if_stmt->cond);
			free_stmt(if_stmt->if_path);
			free(if_stmt->if_path);
			if (if_stmt->else_path) {
				free_stmt(if_stmt->else_path);
				free(if_stmt->else_path);
			}
			break;
		}
		case STMT_LET:
		{
			let_t* let = &stmt->data.let;
			free(let->var);
			free_expr(&let->value);
			break;
		}
		default:
			fprintf(stderr, "unknown statement in free_stmt\n");
			exit(-1);
	}
}
