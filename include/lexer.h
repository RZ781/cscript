#ifndef LEXER_H
#define LEXER_H

#define TOKEN_INT	0
#define TOKEN_STR	1
#define TOKEN_ID	2
#define TOKEN_EOF	3
#define TOKEN_IF	4
#define TOKEN_ELSE	5
#define TOKEN_WHILE	6
#define TOKEN_LET	7

typedef struct Lexer Lexer;

struct Lexer {
	FILE* f;
	Lexer* next;
	char peek_type;
	char peek_data[256];
};

extern const char *token_names[];

Lexer lexer_new(FILE* f);
char lex(Lexer* lexer, char* buffer, int size);
void unlex(Lexer* lexer, char type, char* buffer);
char peek(Lexer* lexer);

#endif
