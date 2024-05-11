#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope.h"
#include "obj.h"

Scope scope_new(Scope* parent) {
	Scope scope;
	scope.parent = parent;
	if (parent)
		scope.globals = parent->globals;
	else
		scope.globals = NULL;
	scope.size = 0;
	scope.cap = 4;
	scope.entries = malloc(4 * sizeof(Entry));
	return scope;
}

void scope_free(Scope* scope) {
	for (int i=0; i<scope->size; i++)
		obj_free(MOVE(scope->entries[i].value));
	free(scope->entries);
}

ObjRef scope_get(Scope* scope, const char* var) {
	Scope* curr = scope;
	while (curr) {
		bool found = false;
		ObjRef obj;
		for (int i=curr->size-1; i>=0; i--) {
			if (strcmp(curr->entries[i].key, var) == 0) {
				found = true;
				obj = REF(curr->entries[i].value);
				break;
			}
		}
		if (found)
			return obj;
		curr = curr->parent;
	}
	scope_let(scope, var, error_new(OBJ_ERROR_VAR, "variable %s not declared", var));
	return scope_get(scope, var);
}

ObjRef scope_set(Scope* scope, const char* var, ObjMove value) {
	Obj obj = GET(value);
	while (scope) {
		for (int i=scope->size-1; i>=0; i--) {
			if (strcmp(scope->entries[i].key, var) == 0) {
				scope->entries[i].value = obj;
				return REF(obj);
			}
		}
		scope = scope->parent;
	}
	scope_let(scope, var, error_new(OBJ_ERROR_VAR, "variable %s not declared", var));
	return scope_get(scope, var);
}

void scope_let(Scope* scope, const char* var, ObjMove value) {
	Obj obj = GET(value);
	if (scope->cap == scope->size) {
		scope->cap *= 2;
		scope->entries = realloc(scope->entries, scope->cap * sizeof(Entry));
	}
	Entry entry;
	strncpy(entry.key, var, 16);
	entry.value = obj;
	scope->entries[scope->size++] = entry;
}
