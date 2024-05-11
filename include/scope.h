#ifndef ENV_H
#define ENV_H
#include "obj.h"

typedef struct Scope Scope;

typedef struct {
	char key[16];
	Obj value;
} Entry;

struct Scope {
	Scope* parent;
	Scope* globals;
	int size;
	int cap;
	Entry* entries;
};

Scope scope_new(Scope* parent);
void scope_free(Scope* scope);
ObjRef scope_get(Scope* scope, const char* var);
ObjRef scope_set(Scope* scope, const char* var, ObjMove value);
void scope_let(Scope* scope, const char* var, ObjMove value);

#endif
