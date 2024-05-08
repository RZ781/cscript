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

typedef struct lexer_t lexer_t;

struct lexer_t {
	FILE* f;
	lexer_t* next;
	char peek_type;
	char peek_data[256];
};

extern const char *token_names[];

lexer_t lexer_new(FILE* f);
char lex(lexer_t* lexer, char* buffer, int size);
void unlex(lexer_t* lexer, char type, char* buffer);
char peek(lexer_t* lexer);

#endif
