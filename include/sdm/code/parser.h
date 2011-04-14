/*
 * Header Name:	parser.h
 * Description:	MooCode Parser
 */

#ifndef _SDM_CODE_PARSER_H
#define _SDM_CODE_PARSER_H

#include <sdm/array.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/code/expr.h>

#define TT_OPEN		1
#define TT_CLOSE	2
#define TT_STRING	3
#define TT_WORD		4

#define MAX_TOKEN_LEN		512

class MooCodeParser {
	short m_line;
	short m_col;
	short m_last_col;
	int m_pos;
	const char *m_input;

	int m_type;
	int m_len;
	char m_token[MAX_TOKEN_LEN];

    public:
	MooCodeParser();

	static MooCodeExpr *parse_code(const char *str);
	MooCodeExpr *parse(const char *str);
	static int generate(MooCodeExpr *expr, char *buffer, int max, char linebr = '\n', int indent = 0);
	static void print(MooCodeExpr *expr);

    private:
	MooCodeExpr *parse_list();
	MooCodeExpr *parse_token();
	int read_token();

	inline char getchar() {
		m_col++;
		if (m_input[m_pos] == '\n') {
			m_line++;
			m_last_col = m_col;
			m_col = 1;
		}
		return(m_input[m_pos++]);
	}

	/// NOTE: This only guarantees only one unget while maintaining correct line and column number (in exchange for slight performance)
	inline void ungetchar() {
		if (--m_pos <= 0)
			m_pos = 0;
		else {
			m_col--;
			if (m_input[m_pos] == '\n') {
				m_line--;
				m_col = m_last_col;
			}
		}
	}
};

#endif

