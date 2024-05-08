#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>
#define MOVE(x) obj_move(&x)
#define REF(x) ((obj_ref_t) {.obj_ref=(x)})
#define GET(x) ((x).obj_move)
#define TYPE(x) ((x).obj_ref.type)
#define DATA(x) ((x).obj_ref.data)
#define IS_ERROR(type) ((type) >= OBJ_ERROR && (type) <= OBJ_ERROR_USER)

typedef struct obj_t obj_t;
typedef struct obj_move_t obj_move_t;
typedef struct obj_ref_t obj_ref_t;

typedef struct {
	obj_move_t (*f)(int, obj_ref_t*);
	const char* name;
} builtin_t;

struct obj_t {
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
		builtin_t b;
	} data;
};

struct obj_move_t {
	obj_t obj_move;
};

struct obj_ref_t {
	obj_t obj_ref;
};

obj_move_t int_new(long i);
obj_move_t str_new(const char* s);
obj_move_t builtin_new(obj_move_t (*f)(int, obj_ref_t*), const char* name);
obj_move_t void_new(void);
obj_move_t error_new(int type, const char* format, ...);

char* obj_to_str(obj_ref_t obj);
bool obj_to_bool(obj_ref_t obj);
obj_move_t obj_call(obj_ref_t f, int n, obj_ref_t* args);
void obj_free(obj_move_t obj);
obj_move_t obj_copy(obj_ref_t obj);
obj_move_t obj_move(obj_t* obj);

#endif
