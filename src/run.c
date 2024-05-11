#include <stdlib.h>
#include "run.h"
#include "parser.h"
#include "scope.h"
#include "obj.h"

ObjMove assign(Expr* var, ObjMove obj_move, Scope* scope) {
	switch (var->type) {
		case EXPR_VAR:
			return obj_copy(scope_set(scope, var->data.s, obj_move));
		default:
			fprintf(stderr, "unknown expression in assign\n");
			abort();
	}
}

ObjMove run_expr(Expr* e, Scope* scope) {
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
			return obj_copy(scope_get(scope, e->data.s));
		case EXPR_CALL:
		{
			FuncCallExpr* call = &e->data.call;
			Obj f = GET(run_expr(call->f, scope));
			Obj* args = malloc(call->n_args * sizeof(Obj));
			ObjRef* args_ref = malloc(call->n_args * sizeof(ObjRef));
			for (int i=0; i<call->n_args; i++) {
				args[i] = GET(run_expr(&call->args[i], scope));
				args_ref[i] = REF(args[i]);
			}
			Obj result = GET(obj_call(REF(f), call->n_args, args_ref));
			obj_free(MOVE(f));
			for (int i=0; i<call->n_args; i++)
				obj_free(MOVE(args[i]));
			free(args);
			free(args_ref);
			return MOVE(result);
		}
		case EXPR_OP:
		{
			OpExpr op = e->data.op;
			char s[2] = {op.op, 0};
			ObjRef f = scope_get(scope->globals, s);
			Obj ret;
			Obj* args = malloc(2 * sizeof(Obj));
			ObjRef* args_ref = malloc(2 * sizeof(ObjRef));
			args[0] = GET(run_expr(op.l, scope));
			args_ref[0] = REF(args[0]);
			if (op.r) {
				args[1] = GET(run_expr(op.r, scope));
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
			OpExpr op = e->data.op;
			Obj value = GET(run_expr(op.r, scope));
			if (IS_ERROR(value.type))
				return MOVE(value);
			return assign(op.l, MOVE(value), scope);
		}
		default:
			fprintf(stderr, "unknown expression in run_expr\n");
			abort();
	}
}

ObjMove run_stmt(Stmt* stmt, Scope* scope) {
	Obj error = GET(void_new());
	switch (stmt->type) {
		case STMT_EXPR:
		{
			Obj value = GET(run_expr(&stmt->data.expr, scope));
			if (IS_ERROR(value.type))
				error = value;
			else
				obj_free(MOVE(value));
			break;
		}
		case STMT_BLOCK:
		{
			BlockStmt* block = &stmt->data.block;
			Scope block_scope = scope_new(scope);
			for (int i=0; i<block->n_stmts; i++) {
				Obj value = GET(run_stmt(&block->stmts[i], &block_scope));
				if (IS_ERROR(value.type)) {
					error = value;
					break;
				}
				obj_free(MOVE(value));
			}
			scope_free(&block_scope);
			break;
		}
		case STMT_IF:
		{
			IfStmt* if_stmt = &stmt->data.if_stmt;
			Obj cond = GET(run_expr(&if_stmt->cond, scope));
			if (IS_ERROR(cond.type)) {
				error = cond;
			} else if (obj_to_bool(REF(cond))) {
				Scope if_scope = scope_new(scope);
				Obj value = GET(run_stmt(if_stmt->if_path, &if_scope));
				scope_free(&if_scope);
				if (IS_ERROR(value.type))
					error = value;
				obj_free(MOVE(cond));
			} else if (if_stmt->else_path) {
				Scope if_scope = scope_new(scope);
				Obj value = GET(run_stmt(if_stmt->else_path, &if_scope));
				scope_free(&if_scope);
				if (IS_ERROR(value.type))
					error = value;
				obj_free(MOVE(cond));
			}
			break;
		}
		case STMT_WHILE:
		{
			IfStmt* if_stmt = &stmt->data.if_stmt;
			while (1) {
				Obj cond = GET(run_expr(&if_stmt->cond, scope));
				if (IS_ERROR(cond.type)) {
					error = cond;
					break;
				}
				bool c = obj_to_bool(REF(cond));
				obj_free(MOVE(cond));
				if (!c)
					break;
				Scope while_scope = scope_new(scope);
				Obj value = GET(run_stmt(if_stmt->if_path, &while_scope));
				scope_free(&while_scope);
				if (IS_ERROR(value.type)) {
					error = value;
					break;
				}
			}
			break;
		}
		case STMT_LET:
		{
			LetStmt* let = &stmt->data.let;
			Obj value = GET(run_expr(&let->value, scope));
			if (IS_ERROR(value.type)) {
				error = value;
				break;
			}
			scope_let(scope, let->var, MOVE(value));
			break;
		}
		default:
			fprintf(stderr, "unknown statement in run_stmt\n");
			abort();
	}
	return MOVE(error);
}
