#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

const char *token_names[] = {
	"int",
	"string",
	"id",
	"eof",
	"if",
	"else",
	"while",
	"let",
	"operator",
	"unary operator",
	"binary operator"
};

void lex_inner(lexer_t* lexer) {
	FILE* f = lexer->f;
	char* buffer = lexer->peek_data;
	fscanf(f, " ");
	buffer[0] = 0;
	char c = fgetc(f);
	if (c == EOF) {
		c = TOKEN_EOF;
	} else if (isdigit(c)) {
		int i;
		buffer[0] = c;
		for (i=1; i<255; i++) {
			c = fgetc(f);
			if (!isdigit(c)) {
				ungetc(c, f);
				break;
			}
			buffer[i] = c;
		}
		buffer[i] = 0;
		c = TOKEN_INT;
	} else if (isalpha(c) || c == '_') {
		int i;
		buffer[0] = c;
		for (i=1; i<255; i++) {
			c = fgetc(f);
			if (!(isalnum(c) || c == '_')) {
				ungetc(c, f);
				break;
			}
			buffer[i] = c;
		}
		buffer[i] = 0;
		if (strcmp(buffer, "if") == 0)
			c = TOKEN_IF;
		else if (strcmp(buffer, "else") == 0)
			c = TOKEN_ELSE;
		else if (strcmp(buffer, "while") == 0)
			c = TOKEN_WHILE;
		else if (strcmp(buffer, "let") == 0)
			c = TOKEN_LET;
		else
			c = TOKEN_ID;
	} else if (c == '"') {
		int i;
		for (i=0; i<255; i++) {
			c = fgetc(f);
			if (c == EOF) {
				fprintf(stderr, "unexpected eof\n");
				exit(-1);
			} else if (c == '"') {
				break;
			} else if (c == '\\') {
				c = fgetc(f);
				if (c == 'n')
					c = '\n';
			}
			buffer[i] = c;
		}
		buffer[i] = 0;
		c = TOKEN_STR;
	} else {
		buffer[0] = c;
		buffer[1] = 0;
	}
	lexer->peek_type = c;
}

lexer_t lexer_new(FILE* f) {
	lexer_t lexer;
	lexer.f = f;
	lexer.next = NULL;
	lex_inner(&lexer);
	return lexer;
}

char lex(lexer_t* lexer, char* buffer, int size) {
	char p = lexer->peek_type;
	lexer_t* next = lexer->next;
	strncpy(buffer, lexer->peek_data, size);
	if (next) {
		*lexer = *next;
		free(next);
	} else {
		lex_inner(lexer);
	}
	return p;
}

void unlex(lexer_t* lexer, char type, char* buffer) {
	lexer_t* next = malloc(sizeof(lexer_t));
	*next = *lexer;
	lexer->next = next;
	lexer->peek_type = type;
	strncpy(lexer->peek_data, buffer, 256);
}

char peek(lexer_t* lexer) {
	return lexer->peek_type;
}
