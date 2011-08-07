/*
 * Header Name:	parser.h
 * Description:	MooCode Parser
 */

#ifndef _SDM_CODE_PARSER_H
#define _SDM_CODE_PARSER_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#include <sdm/code/expr.h>

#define TT_OPEN		1
#define TT_CLOSE	2
#define TT_STRING	3
#define TT_WORD		4

#define MAX_TOKEN_LEN		512

class MooCodeStyle {
    public:
	const char *m_indent;
	char m_indent_len;
	char m_linebr;
	char m_wrap;

	MooCodeStyle(const char *indent, char linebr, int wrap);
	int linebr(char *buffer, int max);
	int indent(char *buffer, int max, int level);
};

extern MooCodeStyle moo_style_normal;
extern MooCodeStyle moo_style_one_line;

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
	static int generate(MooCodeExpr *expr, char *buffer, int max, MooCodeStyle *style = NULL, int level = 0);
	static int generate_expr(MooCodeExpr *expr, char *buffer, int max, MooCodeStyle *style = NULL, int level = 0);
	static void print(MooCodeExpr *expr);

	static int encode_string(const char *str, char *buffer, int max);

    private:
	MooCodeExpr *parse_list();
	MooCodeExpr *parse_token();
	int read_token();

	int token_type() { return(m_type); }
	int token_len() { return(m_len); }
	const char *token() { return(m_token); }

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

static inline int parser_is_digit(char ch)
{
	if ((ch >= 0x30) && (ch <= 0x39))
		return(1);
	return(0);
}

static inline int parser_is_whitespace(char ch)
{
	if ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'))
		return(1);
	return(0);
}

static inline int parser_is_word(char ch)
{
	if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || (ch == '_'))
		return(1);
	return(0);
}

static inline int parser_is_identifier(char ch)
{
	if ((ch != '(') && (ch != ')') && !parser_is_whitespace(ch))
		return(1);
	return(0);
}

static inline char parser_escape_char(char ch)
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

static inline const char *parser_next_word(const char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; i++) {
		if (parser_is_whitespace(str[i])) {
			while (parser_is_whitespace(str[i]))
				i++;
			break;
		}
	}
	return(&str[i]);
}

#endif

