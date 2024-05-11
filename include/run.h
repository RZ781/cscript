#ifndef RUN_H
#define RUN_H
#include "parser.h"
#include "obj.h"
#include "scope.h"

ObjMove run_expr(Expr* e, Scope* env);
ObjMove run_stmt(Stmt* stmt, Scope* env);

#endif
