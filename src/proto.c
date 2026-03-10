#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "proto.h"
#include "trace.h"

/* separators */
#define OPERAND_SEPARATOR	","
#define OPCODE_SEPARATOR	" "

/* OP code */
#define OP_ADD	"ADD"
#define OP_SUB	"SUB"
#define OP_MUL	"MUL"
#define OP_DIV	"DIV"
#define OP_LEN	"LEN"
#define OP_CAT	"CAT"
#define OP_FIND	"FIND"
#define OP_HELP	"HELP"

typedef struct {
	const char	*opcode;
	int (*cb)(char *inp, char *out, int sz_out);
} _opcode_t;

static int response(char *resp, int sz_resp, const char *fmt, ...) {
	int r = 0;
	va_list va;

	va_start(va, fmt);
	r = vsnprintf(resp, sz_resp, fmt, va);
	va_end(va);

	return r;
}

#define NOT_NUMBER	0
#define INT_NUMBER	1
#define FLOAT_NUMBER	2

static int is_number(const char *str) {
	int f = 0; /* float flag */
	int n = 0; /* counter */
	int r = INT_NUMBER;
	int l = strlen(str);

	if (l) {
		for (;n < l; n++) {
			if (str[n] == '.') {
				if (!f)
					f = 1;
				else {
					r = NOT_NUMBER;
					f = 0;
					break;
				}
			} else {
				if (!isdigit(str[n])) {
					r = NOT_NUMBER;
					f = 0;
					break;
				}
			}
		}
	} else
		r = NOT_NUMBER;

	if (f)
		r = FLOAT_NUMBER;

	return r;
}

static void str_trim_left(char *str) {
	unsigned int i,l;

	l = strlen(str);

	for (i = 0; i < l; i++) {
		if (str[i] != ' ' && str[i] != '\t')
			break;
	}

	if (i == 0)
		return;

	memmove(str, (str + i), (strlen(str + i) + 1));
}

static void str_trim_right(char *str) {
	unsigned int i,l;

	l = strlen(str);
	if (l) {
		i=l-1;

		while (str[i] == ' ' || str[i] == '\t') {
			str[i]= 0;
			i--;
		}
	}
}

static void str_trim(char *str) {
	str_trim_right(str);
	str_trim_left(str);
}

static int do_add(char *operands, char *resp, int sz_resp) {
	int r = 0;
	char *op1, *op2;

	if ((op1 = strtok_r(operands, OPERAND_SEPARATOR, &op2))) {
		str_trim(op1);
		str_trim(op2);

		/* verify operands */
		int v_op1 = is_number(op1);
		int v_op2 = is_number(op2);

		if (v_op1 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 1 '%s' not a number\n", op1);
		else if (v_op2 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 2 '%s' not a number\n", op2);
		else if (v_op1 == FLOAT_NUMBER || v_op2 == FLOAT_NUMBER)
			r = response(resp, sz_resp, "[OK] %f\n", atof(op1) + atof(op2));
		else
			r = response(resp, sz_resp, "[OK] %d\n", atoi(op1) + atoi(op2));
	}

	return r;
}

static int do_sub(char *operands, char *resp, int sz_resp) {
	int r = 0;
	char *op1, *op2;

	if ((op1 = strtok_r(operands, OPERAND_SEPARATOR, &op2))) {
		str_trim(op1);
		str_trim(op2);

		/* verify operands */
		int v_op1 = is_number(op1);
		int v_op2 = is_number(op2);

		if (v_op1 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 1 '%s' not a number\n", op1);
		else if (v_op2 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 2 '%s' not a number\n", op2);
		else if (v_op1 == FLOAT_NUMBER || v_op2 == FLOAT_NUMBER)
			r = response(resp, sz_resp, "[OK] %f\n", atof(op1) - atof(op2));
		else
			r = response(resp, sz_resp, "[OK] %d\n", atoi(op1) - atoi(op2));
	}

	return r;
}

static int do_mul(char *operands, char *resp, int sz_resp) {
	int r = 0;
	char *op1, *op2;

	if ((op1 = strtok_r(operands, OPERAND_SEPARATOR, &op2))) {
		str_trim(op1);
		str_trim(op2);

		/* verify operands */
		int v_op1 = is_number(op1);
		int v_op2 = is_number(op2);

		if (v_op1 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 1 '%s' not a number\n", op1);
		else if (v_op2 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 2 '%s' not a number\n", op2);
		else if (v_op1 == FLOAT_NUMBER || v_op2 == FLOAT_NUMBER)
			r = response(resp, sz_resp, "[OK] %f\n", atof(op1) * atof(op2));
		else
			r = response(resp, sz_resp, "[OK] %d\n", atoi(op1) * atoi(op2));
	}

	return r;
}

static int do_div(char *operands, char *resp, int sz_resp) {
	int r = 0;
	char *op1, *op2;

	if ((op1 = strtok_r(operands, OPERAND_SEPARATOR, &op2))) {
		str_trim(op1);
		str_trim(op2);

		/* verify operands */
		int v_op1 = is_number(op1);
		int v_op2 = is_number(op2);

		if (v_op1 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 1 '%s' not a number\n", op1);
		else if (v_op2 == NOT_NUMBER)
			r = response(resp, sz_resp, "[ERROR] operand 2 '%s' not a number\n", op2);
		else if (v_op1 == FLOAT_NUMBER || v_op2 == FLOAT_NUMBER) {
			if (atof(op2) == 0.0)
				r = response(resp, sz_resp, "[ERROR] Division by zero !\n");
			else
				r = response(resp, sz_resp, "[OK] %f\n", atof(op1) / atof(op2));
		} else {
			if (atoi(op2) == 0)
				r = response(resp, sz_resp, "[ERROR] Division by zero !\n");
			else
				r = response(resp, sz_resp, "[OK] %d\n", atoi(op1) / atoi(op2));
		}
	}

	return r;
}

static int do_len(char *operand, char *resp, int sz_resp) {
	return response(resp, sz_resp, "[OK] %d\n", strlen(operand));
}

static int do_cat(char *operands, char *resp, int sz_resp) {
	int r = 0;
	char *op1, *op2;

	if ((op1 = strtok_r(operands, OPERAND_SEPARATOR, &op2))) {
		str_trim(op1);
		str_trim(op2);
		r = response(resp, sz_resp, "[OK] %s%s\n", op1, op2);
	}

	return r;
}

static int do_find(char *operands, char *resp, int sz_resp) {
	int r = 0;
	char *op1, *op2;

	if ((op1 = strtok_r(operands, OPERAND_SEPARATOR, &op2))) {
		str_trim(op1);
		str_trim(op2);

		char *substr = strcasestr(op2, op1);

		if(substr)
			r = response(resp, sz_resp, "[OK] %d\n", (unsigned long)substr - (unsigned long)op2);
		else
			r = response(resp, sz_resp, "[ERROR] Not found\n");
	}

	return r;
}

static int do_help(char __attribute__((unused))*operands, char *resp, int sz_resp) {
	static const char *help = "\
 Protocol Syntax:\n\
 <opcode> <operand1>[,<operand2>]\n\
 \n\
 Supported Opcodes:\n\
ADD: Add two numbers.\n\
SUB: Subtract two numbers.\n\
MUL: Multiply two numbers.\n\
DIV: Divide operand1 by operand2.\n\
LEN: Calculate the length of a string (uses operand1).\n\
CAT: Concatenate operand1 and operand2.\n\
FIND: Find the starting index of substring (operand1) within operand2.\n\
?: Print this help\n\
HELP: Print this help\n\
";
	return  response(resp, sz_resp, "%s\n", help);
}

static int do_opcode(const char *opcode, char *operands, char *resp, int sz_resp) {
	int r = 0;
	int n = 0;
	static _opcode_t op[] = {
		{ OP_ADD,	do_add },
		{ OP_SUB,	do_sub },
		{ OP_MUL,	do_mul },
		{ OP_DIV,	do_div },
		{ OP_LEN,	do_len },
		{ OP_CAT,	do_cat },
		{ OP_FIND,	do_find },
		{ OP_HELP,	do_help },
		{ "?",		do_help },
		{ NULL,		NULL }
	};

	while (op[n].opcode) {
		if (strncasecmp(opcode, op[n].opcode, strlen(op[n].opcode)) == 0) {
			r = op[n].cb(operands, resp, sz_resp);
			break;
		}

		n++;
	}

	if (op[n].opcode == NULL)
		r = response(resp, sz_resp, "[ERROR] Unknown operation '%s'\n", opcode);

	return r;
}

int proto_exec(char *req, char *resp, int sz_resp) {
	int r = 0;
	char *opcode;
	char *operands;

	/* extract op code */
	if ((opcode = strtok_r(req, OPCODE_SEPARATOR, &operands)))
		r = do_opcode(opcode, operands, resp, sz_resp);
	else
		r = response(resp, sz_resp,"[ERROR] Syntax\n");

	return r;
}
