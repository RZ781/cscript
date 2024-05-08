#ifndef ENV_H
#define ENV_H
#include "obj.h"

typedef struct env_t env_t;

typedef struct {
	char key[16];
	obj_t value;
} entry_t;

struct env_t {
	env_t* parent;
	env_t* globals;
	int size;
	int cap;
	entry_t* entries;
};

env_t env_new(env_t* parent);
void env_free(env_t* env);
obj_ref_t env_get(env_t* env, const char* var);
obj_ref_t env_set(env_t* env, const char* var, obj_move_t value);
void env_let(env_t* env, const char* var, obj_move_t value);

#endif
