#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "obj.h"
#include "env.h"

obj_move_t builtin_print(int n, obj_ref_t* args) {
	for (int i=0; i<n; i++) {
		char* s = obj_to_str(args[i]);
		printf("%s", s);
		free(s);
		if (i == n-1)
			putchar('\n');
		else
			putchar(' ');
	}
	return void_new();
}

obj_move_t builtin_sub(int n, obj_ref_t* args) {
	if (n == 1) {
		if (TYPE(args[0]) != OBJ_INT)
			return error_new(OBJ_ERROR_TYPE, "can't negate non-numeric types");
		return int_new(-DATA(args[0]).i);
	} else if (n == 2) {
		if (TYPE(args[0]) != OBJ_INT || TYPE(args[1]) != OBJ_INT)
			return error_new(OBJ_ERROR_TYPE, "can't subtract non-numeric types");
		return int_new(DATA(args[0]).i - DATA(args[1]).i);
	} else {
		fprintf(stderr, "op must have 1 or 2 args, got %i\n", n);
		exit(-1);
	}
}

env_t globals_new(void) {
	env_t env = env_new(NULL);
	env_let(&env, "print", builtin_new(builtin_print, "print"));
	env_let(&env, "-", builtin_new(builtin_sub, "-"));
	return env;
}
