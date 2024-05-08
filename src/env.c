#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "obj.h"

env_t env_new(env_t* parent) {
	env_t env;
	env.parent = parent;
	if (parent)
		env.globals = parent->globals;
	else
		env.globals = NULL;
	env.size = 0;
	env.cap = 4;
	env.entries = malloc(4 * sizeof(entry_t));
	return env;
}

void env_free(env_t* env) {
	for (int i=0; i<env->size; i++)
		obj_free(MOVE(env->entries[i].value));
	free(env->entries);
}

obj_ref_t env_get(env_t* env, const char* var) {
	env_t* curr = env;
	while (curr) {
		bool found = false;
		obj_ref_t obj;
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
	env_let(env, var, error_new(OBJ_ERROR_VAR, "variable %s not declared", var));
	return env_get(env, var);
}

obj_ref_t env_set(env_t* env, const char* var, obj_move_t value) {
	obj_t obj = GET(value);
	while (env) {
		for (int i=env->size-1; i>=0; i--) {
			if (strcmp(env->entries[i].key, var) == 0) {
				env->entries[i].value = obj;
				return REF(obj);
			}
		}
		env = env->parent;
	}
	env_let(env, var, error_new(OBJ_ERROR_VAR, "variable %s not declared", var));
	return env_get(env, var);
}

void env_let(env_t* env, const char* var, obj_move_t value) {
	obj_t obj = GET(value);
	if (env->cap == env->size) {
		env->cap *= 2;
		env->entries = realloc(env->entries, env->cap * sizeof(entry_t));
	}
	entry_t entry;
	strncpy(entry.key, var, 16);
	entry.value = obj;
	env->entries[env->size++] = entry;
}
