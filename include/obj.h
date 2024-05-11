#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>
#define MOVE(x) obj_move(&x)
#define REF(x) ((ObjRef) {.obj_ref=(x)})
#define GET(x) ((x).obj_move)
#define TYPE(x) ((x).obj_ref.type)
#define DATA(x) ((x).obj_ref.data)
#define IS_ERROR(type) ((type) >= OBJ_ERROR && (type) <= OBJ_ERROR_USER)

typedef struct Obj Obj;
typedef struct ObjMove ObjMove;
typedef struct ObjRef ObjRef;

typedef struct {
	ObjMove (*f)(int, ObjRef*);
	const char* name;
} Builtin;

struct Obj {
	enum {
		OBJ_INT,
		OBJ_STR,
		OBJ_BUILTIN,
		OBJ_VOID,
		OBJ_MOVED,
		OBJ_ERROR,
		OBJ_ERROR_TYPE,
		OBJ_ERROR_VAR,
		OBJ_ERROR_USER
	} type;
	union {
		long i;
		char* s;
		Builtin b;
	} data;
};

struct ObjMove {
	Obj obj_move;
};

struct ObjRef {
	Obj obj_ref;
};

ObjMove int_new(long i);
ObjMove str_new(const char* s);
ObjMove builtin_new(ObjMove (*f)(int, ObjRef*), const char* name);
ObjMove void_new(void);
ObjMove error_new(int type, const char* format, ...);

char* obj_to_str(ObjRef obj);
bool obj_to_bool(ObjRef obj);
ObjMove obj_call(ObjRef f, int n, ObjRef* args);
void obj_free(ObjMove obj);
ObjMove obj_copy(ObjRef obj);
ObjMove obj_move(Obj* obj);

#endif
