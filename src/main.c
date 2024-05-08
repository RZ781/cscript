#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
#include "parser.h"
#include "run.h"
#include "globals.h"
#include "obj.h"

int main(int argc, char** argv) {
	const char* name = "src.csc";
	if (argc == 2)
		name = argv[1];
	FILE* f = fopen(name, "r");
	lexer_t lexer = lexer_new(f);
	env_t globals = globals_new();
	bool error = false;
	globals.globals = &globals;
	while (peek(&lexer) != TOKEN_EOF && !error) {
		stmt_t stmt = parse_stmt(&lexer);
		obj_t value = GET(run_stmt(&stmt, &globals));
		if (IS_ERROR(value.type)) {
			char* s = obj_to_str(REF(value));
			fprintf(stderr, "uncaught error: %s\n", s);
			free(s);
			error = true;
		}
		free_stmt(&stmt);
		obj_free(MOVE(value));
	}
	env_free(&globals);
	fclose(f);
}
