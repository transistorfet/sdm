/*
 * Header Name:	parser.h
 * Description:	MooCode Parser
 */

#ifndef _SDM_ACTIONS_CODE_PARSER_H
#define _SDM_ACTIONS_CODE_PARSER_H

#include <sdm/array.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

#include "expr.h"

#define TT_OPEN		1
#define TT_CLOSE	2
#define TT_STRING	3
#define TT_WORD		4

#define MAX_TOKEN_LEN		512

class MooCodeParser {
	short m_line;
	short m_col;
	int m_pos;
	const char *m_input;

	int m_type;
	int m_len;
	char m_token[MAX_TOKEN_LEN];

    public:
	MooCodeParser();

	MooCodeExpr *parse(const char *str);

	MooCodeExpr *parse_list();
	MooCodeExpr *parse_token();
	int read_token();
	
};

#endif

