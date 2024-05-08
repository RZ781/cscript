#include <stdlib.h>
#include "run.h"
#include "parser.h"
#include "env.h"
#include "obj.h"

obj_move_t assign(expr_t* var, obj_move_t obj_move, env_t* env) {
	switch (var->type) {
		case EXPR_VAR:
			return obj_copy(env_set(env, var->data.s, obj_move));
		default:
			fprintf(stderr, "unknown expression in assign\n");
			exit(-1);
	}
}

obj_move_t run_expr(expr_t* e, env_t* env) {
	switch (e->type) {
		case EXPR_INT:
		{
			char* end;
			long l = strtol(e->data.s, &end, 10);
			return int_new(l);
		}
		case EXPR_STR:
			return str_new(e->data.s);
		case EXPR_VAR:
			return obj_copy(env_get(env, e->data.s));
		case EXPR_CALL:
		{
			func_call_t* call = &e->data.call;
			obj_t f = GET(run_expr(call->f, env));
			obj_t* args = malloc(call->n_args * sizeof(obj_t));
			obj_ref_t* args_ref = malloc(call->n_args * sizeof(obj_ref_t));
			for (int i=0; i<call->n_args; i++) {
				args[i] = GET(run_expr(&call->args[i], env));
				args_ref[i] = REF(args[i]);
			}
			obj_t result = GET(obj_call(REF(f), call->n_args, args_ref));
			obj_free(MOVE(f));
			for (int i=0; i<call->n_args; i++)
				obj_free(MOVE(args[i]));
			free(args);
			free(args_ref);
			return MOVE(result);
		}
		case EXPR_OP:
		{
			op_t op = e->data.op;
			char s[2] = {op.op, 0};
			obj_ref_t f = env_get(env->globals, s);
			obj_t ret;
			obj_t* args = malloc(2 * sizeof(obj_t));
			obj_ref_t* args_ref = malloc(2 * sizeof(obj_ref_t));
			args[0] = GET(run_expr(op.l, env));
			args_ref[0] = REF(args[0]);
			if (op.r) {
				args[1] = GET(run_expr(op.r, env));
				args_ref[1] = REF(args[1]);
				ret = GET(obj_call(f, 2, args_ref));
				obj_free(MOVE(args[1]));
			} else {
				ret = GET(obj_call(f, 1, args_ref));
			}
			obj_free(MOVE(args[0]));
			free(args);
			free(args_ref);
			return MOVE(ret);
		}
		case EXPR_ASSIGN:
		{
			op_t op = e->data.op;
			obj_t value = GET(run_expr(op.r, env));
			if (IS_ERROR(value.type))
				return MOVE(value);
			return assign(op.l, MOVE(value), env);
		}
		default:
			fprintf(stderr, "unknown expression in run_expr\n");
			exit(-1);
	}
}

obj_move_t run_stmt(stmt_t* stmt, env_t* env) {
	obj_t error = GET(void_new());
	switch (stmt->type) {
		case STMT_EXPR:
		{
			obj_t value = GET(run_expr(&stmt->data.expr, env));
			if (IS_ERROR(value.type))
				error = value;
			else
				obj_free(MOVE(value));
			break;
		}
		case STMT_BLOCK:
		{
			block_t* block = &stmt->data.block;
			env_t block_env = env_new(env);
			for (int i=0; i<block->n_stmts; i++) {
				obj_t value = GET(run_stmt(&block->stmts[i], &block_env));
				if (IS_ERROR(value.type)) {
					error = value;
					break;
				}
				obj_free(MOVE(value));
			}
			env_free(&block_env);
			break;
		}
		case STMT_IF:
		{
			if_t* if_stmt = &stmt->data.if_stmt;
			obj_t cond = GET(run_expr(&if_stmt->cond, env));
			if (IS_ERROR(cond.type)) {
				error = cond;
			} else if (obj_to_bool(REF(cond))) {
				env_t if_env = env_new(env);
				obj_t value = GET(run_stmt(if_stmt->if_path, &if_env));
				env_free(&if_env);
				if (IS_ERROR(value.type))
					error = value;
				obj_free(MOVE(cond));
			} else if (if_stmt->else_path) {
				env_t if_env = env_new(env);
				obj_t value = GET(run_stmt(if_stmt->else_path, &if_env));
				env_free(&if_env);
				if (IS_ERROR(value.type))
					error = value;
				obj_free(MOVE(cond));
			}
			break;
		}
		case STMT_WHILE:
		{
			if_t* if_stmt = &stmt->data.if_stmt;
			while (1) {
				obj_t cond = GET(run_expr(&if_stmt->cond, env));
				if (IS_ERROR(cond.type)) {
					error = cond;
					break;
				}
				bool c = obj_to_bool(REF(cond));
				obj_free(MOVE(cond));
				if (!c)
					break;
				env_t while_env = env_new(env);
				obj_t value = GET(run_stmt(if_stmt->if_path, &while_env));
				env_free(&while_env);
				if (IS_ERROR(value.type)) {
					error = value;
					break;
				}
			}
			break;
		}
		case STMT_LET:
		{
			let_t* let = &stmt->data.let;
			obj_t value = GET(run_expr(&let->value, env));
			if (IS_ERROR(value.type)) {
				error = value;
				break;
			}
			env_let(env, let->var, MOVE(value));
			break;
		}
		default:
			fprintf(stderr, "unknown statement in run_stmt\n");
			exit(-1);
	}
	return MOVE(error);
}
