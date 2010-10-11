/*
 * Name:	parser.cpp
 * Description:	MooCode Parser
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/data.h>
#include <sdm/memory.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/things/thing.h>
#include <sdm/actions/action.h>
#include <sdm/actions/code/code.h>

#include "expr.h"
#include "event.h"
#include "thread.h"

/*
MooCodeEvent::MooCodeEvent(moo_event_func_t func, MooCodeExpr *expr, MooArgs *args)
{
	m_func = func;
	m_expr = expr;
	m_args = args;
}

MooCodeEvent::~MooCodeEvent()
{
	// TODO destroy references??
}

void MooCodeEvent::set_args(MooArgs *args)
{
	// TODO should this instead make the args?
	if (m_args)
		delete m_args;
	m_args = args;
}

int MooCodeEvent::do_event(MooCodeThread *thread)
{
	if (!m_func)
		return(-1);
	return(this->m_func(thread));
}

int MooCodeEvent::evaluate_expr(MooCodeThread *thread)
{
	if (!m_expr)
		return(-1);
	return(m_expr->eval(thread));
}

int MooCodeEvent::evaluate_expr_list(MooCodeThread *thread)
{
	MooCodeExpr *remain;
	MooCodeEvent *event;

	if (!m_expr)
		return(-1);
	remain = m_expr->next();

	if (remain) {
		event = new MooCodeEvent(MooCodeEvent::evaluate_expr_list, remain);
		thread->push_event(event);
	}

	event = new MooCodeEvent(MooCodeEvent::append_return, NULL, m_args);
	thread->push_event(event);

	event = new MooCodeEvent(MooCodeEvent::evaluate_expr, m_expr);
	thread->push_event(event);
	return(0);
}

int MooCodeEvent::append_return(MooCodeThread *thread)
{
	if (!m_args)
		return(-1);
	return(m_args->m_args->push(thread->get_return()));
}
*/


#define LISPY_MAX_NUMBER		128
#define LISPY_MAX_STRING		1024

static MooCodeExpr *lispy_parse_input(sdMachine *, sdType *, int, sdInput *);
static MooCodeExpr *lispy_parse_expr(sdMachine *, sdType *, int, sdInput *);
static MooCodeExpr *lispy_parse_number(sdMachine *, sdType *, sdInput *, char, linenumber_t);
static MooCodeExpr *lispy_parse_string(sdMachine *, sdType *, sdInput *, char, linenumber_t);
static MooCodeExpr *lispy_parse_identifier(sdMachine *, sdType *, sdInput *, char, linenumber_t);
static int lispy_get_next_char(sdInput *);
static inline int lispy_is_identifier(char);
static inline int lispy_is_digit(char);
static inline int lispy_is_whitespace(char);
static inline int lispy_is_word(char);
static inline char lispy_escape_char(char);

MooCodeExpr *moo_lispy_parse(const char *code)
{
	return(lispy_parse_input(mach, &sdExprTypeDef, 0, input));
}

///// Local Functions /////

sdExpr *lispy_parse_input(sdMachine *mach, sdType *type, int openfunc, sdInput *input)
{
	sdExpr *head, *cur;

	head = cur = lispy_parse_expr(mach, type, openfunc, input);
	while (cur) {
		cur->next = lispy_parse_expr(mach, type, 0, input);
		cur = cur->next;
	}
	return(head);
}

static sdExpr *lispy_parse_expr(sdMachine *mach, sdType *type, int openfunc, sdInput *input)
{
	char ch;
	linenumber_t line;
	sdExpr *expr;

	line = sdrl_get_linenumber(input);
	ch = lispy_get_next_char(input);

	if (!ch)
		return(NULL);
	else if (ch == ')')
		return(NULL);
	else if (lispy_is_digit(ch) || ((ch == '-') && lispy_is_digit(sdrl_peek_char(input)))) {
		if (!(expr = lispy_parse_number(mach, type, input, ch, line)))
			return(NULL);
	}
	else if ((ch == '\"') || (ch == '\'')) {
		if (!(expr = lispy_parse_string(mach, type, input, ch, line)))
			return(NULL);
	}
	else if (ch == '(') {
		if(!(expr = lispy_parse_input(mach, type, 1, input)))
			return(NULL);
		expr = sdrl_make_expr_expr(mach, type, SDRL_ET_CALL, line, expr, NULL);
	}
	else {
		if (!(expr = lispy_parse_identifier(mach, type, input, ch, line)))
			return(NULL);
		if (!openfunc) {
			if (!(expr = sdrl_make_string_expr(mach, type, SDRL_ET_IDENTIFIER, line, "$", expr))
			    || !(expr = sdrl_make_expr_expr(mach, type, SDRL_ET_CALL, line, expr, NULL)))
				return(NULL);
		}
	}
	return(expr);
}

static sdExpr *lispy_parse_number(sdMachine *mach, sdType *type, sdInput *input, char first, linenumber_t line)
{
	int i = 0;
	char buffer[LISPY_MAX_NUMBER];

	// TODO make this parse octal and hex numbers
	buffer[0] = first;
	while ((i < LISPY_MAX_NUMBER - 1) && (buffer[++i] = sdrl_get_char(input))) {
		if (!lispy_is_identifier(buffer[i]))
			break;
	}
	if (buffer[i] == ')')
		sdrl_unget_char(input, ')');
	buffer[i] = '\0';
	return(sdrl_make_number_expr(mach, type, SDRL_ET_NUMBER, line, strtod(buffer, NULL), NULL));
}

static sdExpr *lispy_parse_string(sdMachine *mach, sdType *type, sdInput *input, char first, linenumber_t line)
{
	int i;
	char ch;
	char buffer[LISPY_MAX_STRING];

	for (i = 0; (i < LISPY_MAX_STRING) && (buffer[i] = sdrl_get_char(input)); i++) {
		if ((buffer[i] == '\\') && (first == '\"')) {
			if (!(ch = sdrl_get_char(input)))
				break;
			buffer[i] = lispy_escape_char(ch);
		}
		else if (buffer[i] == first)
			break;
	}
	buffer[i] = '\0';
	return(sdrl_make_string_expr(mach, type, SDRL_ET_STRING, line, buffer, NULL));
}

static sdExpr *lispy_parse_identifier(sdMachine *mach, sdType *type, sdInput *input, char first, linenumber_t line)
{
	int i;
	char buffer[LISPY_MAX_STRING];

	buffer[0] = first;
	for (i = 1; (i < LISPY_MAX_STRING) && (buffer[i] = sdrl_get_char(input)); i++) {
		if (!lispy_is_identifier(buffer[i]))
			break;
	}
	if (buffer[i] == ')')
		sdrl_unget_char(input, ')');
	buffer[i] = '\0';
	return(sdrl_make_string_expr(mach, type, SDRL_ET_IDENTIFIER, line, buffer, NULL));
}

static int lispy_get_next_char(sdInput *input)
{
	char ch;

	while ((ch = sdrl_get_char(input))) {
		if (ch == '#') {
			while ((ch = sdrl_get_char(input)) && (ch != '\n')) ;
		}
		else if (!lispy_is_whitespace(ch))
			break;
	}
	return(ch);
}

static inline int lispy_is_identifier(char ch)
{
	if ((ch != '(') && (ch != ')') && !lispy_is_whitespace(ch))
		return(1);
	return(0);
}

static inline int lispy_is_digit(char ch)
{
	if ((ch >= 0x30) && (ch <= 0x39))
		return(1);
	return(0);
}

static inline int lispy_is_whitespace(char ch)
{
	if ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'))
		return(1);
	return(0);
}

static inline int lispy_is_word(char ch)
{
	if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || (ch == '_'))
		return(1);
	return(0);
}

static inline char lispy_escape_char(char ch)
{
	switch (ch) {
		case 't':
			return('\x09');
		case 'r':
			return('\x0d');
		case 'n':
			return('\x0a');
		default:
			return(ch);
	}
}



