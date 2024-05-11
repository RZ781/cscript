#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "obj.h"
#include "scope.h"

ObjMove builtin_print(int n, ObjRef* args) {
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

ObjMove builtin_sub(int n, ObjRef* args) {
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
		abort();
	}
}

Scope globals_new(void) {
	Scope scope = scope_new(NULL);
	scope_let(&scope, "print", builtin_new(builtin_print, "print"));
	scope_let(&scope, "-", builtin_new(builtin_sub, "-"));
	return scope;
}
