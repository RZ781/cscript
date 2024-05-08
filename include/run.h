#ifndef RUN_H
#define RUN_H
#include "parser.h"
#include "obj.h"
#include "env.h"

obj_move_t run_expr(expr_t* e, env_t* env);
obj_move_t run_stmt(stmt_t* stmt, env_t* env);

#endif
