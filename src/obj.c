#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "obj.h"

ObjMove int_new(long i) {
	Obj obj;
	obj.type = OBJ_INT;
	obj.data.i = i;
	return MOVE(obj);
}

ObjMove str_new(const char* s) {
	Obj obj;
	obj.type = OBJ_STR;
	obj.data.s = malloc(strlen(s) + 1);
	strcpy(obj.data.s, s);
	return MOVE(obj);
}

ObjMove builtin_new(ObjMove (*f)(int, ObjRef*), const char* name) {
	Obj obj;
	obj.type = OBJ_BUILTIN;
	obj.data.b.f = f;
	obj.data.b.name = name;
	return MOVE(obj);
}

ObjMove void_new(void) {
	Obj obj;
	obj.type = OBJ_VOID;
	return MOVE(obj);
}

ObjMove error_new(int type, const char* format, ...) {
	if (!IS_ERROR(type)) {
		fprintf(stderr, "error must have an error code, got code %i\n", type);
		abort();
	}
	Obj obj;
	obj.type = type;
	obj.data.s = malloc(256);
	va_list args;
	va_start(args, format);
	vsnprintf(obj.data.s, 256, format, args);
	va_end(args);
	return MOVE(obj);
}

char* obj_to_str(ObjRef obj) {
	switch (TYPE(obj)) {
		case OBJ_INT:
		{
			char* s = malloc(32);
			snprintf(s, 32, "%li", DATA(obj).i);
			return s;
		}
		case OBJ_BUILTIN:
		{
			char* s = malloc(32);
			snprintf(s, 32, "<builtin %s>", DATA(obj).b.name);
			return s;
		}
		case OBJ_VOID:
		{
			char* s = malloc(5);
			strcpy(s, "void");
			return s;
		}
		default:
			if (TYPE(obj) || IS_ERROR(TYPE(obj))) {
				char* s = malloc(strlen(DATA(obj).s)+1);
				strcpy(s, DATA(obj).s);
				return s;
			}
			fprintf(stderr, "unknown type in obj_to_str\n");
			abort();
	}
}

bool obj_to_bool(ObjRef obj) {
	switch (TYPE(obj)) {
		case OBJ_INT:
			return DATA(obj).i != 0;
		case OBJ_STR:
			return *DATA(obj).s != 0;
		case OBJ_BUILTIN:
			return true;
		default:
			fprintf(stderr, "unknown type in obj_to_bool\n");
			abort();
	}
}

ObjMove obj_call(ObjRef f, int n, ObjRef* args) {
	switch (TYPE(f)) {
		case OBJ_BUILTIN:
			return DATA(f).b.f(n, args);
		default: {
			char* name = obj_to_str(f);
			Obj error = GET(error_new(OBJ_ERROR_TYPE, "can't call object %s", name));
			free(name);
			return MOVE(error);
		}
	}
}

void obj_free(ObjMove obj_move) {
	Obj obj = GET(obj_move);
	switch (obj.type) {
		case OBJ_INT: case OBJ_BUILTIN: case OBJ_MOVED: case OBJ_VOID:
			break;
		default:
			if (obj.type == OBJ_STR || IS_ERROR(obj.type)) {
				free(obj.data.s);
				break;
			}
			fprintf(stderr, "unknown type in obj_free\n");
			abort();
	}
}

ObjMove obj_copy(ObjRef obj) {
	ObjMove ret;
	switch (TYPE(obj)) {
		case OBJ_INT:
			ret = int_new(DATA(obj).i);
			break;
		case OBJ_BUILTIN:
			ret = builtin_new(DATA(obj).b.f, DATA(obj).b.name);
			break;
		case OBJ_STR:
			ret = str_new(DATA(obj).s);
			break;
		default:
			if (IS_ERROR(TYPE(obj))) {
				ret = error_new(TYPE(obj), "%s", DATA(obj).s);
				break;
			}
			fprintf(stderr, "unknown object in obj_copy\n");
			abort();
	}
	return ret;
}

ObjMove obj_move(Obj* obj) {
	if (obj->type == OBJ_MOVED) {
		fprintf(stderr, "object moved twice\n");
		abort();
	}
	ObjMove obj_move = {*obj};
	obj->type = OBJ_MOVED;
	return obj_move;
}
