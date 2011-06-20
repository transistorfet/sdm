/*
 * Name:	parser.cpp
 * Description:	MooCode Parser
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/objs/boolean.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>

#include <sdm/code/code.h>

MooCodeStyle moo_style_normal = MooCodeStyle("  ", '\n', 60);
MooCodeStyle moo_style_one_line = MooCodeStyle(NULL, ' ', 0);

MooCodeParser::MooCodeParser()
{

}

MooCodeExpr *MooCodeParser::parse_code(const char *code)
{
	MooCodeExpr *expr;
	MooCodeParser parser;

	expr = parser.parse(code);
	// TODO temporary
	MooCodeParser::print(expr);
	return(expr);
}

MooCodeExpr *MooCodeParser::parse(const char *str)
{
	m_input = str;
	m_pos = 0;
	m_line = 1;
	m_col = 1;
	m_last_col = 1;
	return(this->parse_list());
}

MooCodeExpr *MooCodeParser::parse_list()
{
	MooCodeExpr *first = NULL, *cur = NULL, *prev = NULL;

	while (1) {
		this->read_token();
		if (!(cur = this->parse_token()))
			break;
		if (prev)
			prev->next(cur);
		prev = cur;
		if (!first)
			first = cur;
	}
	return(first);
}

MooCodeExpr *MooCodeParser::parse_token()
{
	switch (m_type) {
	    case 0:
	    case TT_CLOSE:
		return(NULL);
	    case TT_STRING:
		return(new MooCodeExpr(m_line, m_col, MCT_OBJECT, new MooString("%s", m_token)));
	    case TT_WORD: {
		if (!strcasecmp(m_token, "#t"))
			return(new MooCodeExpr(m_line, m_col, MCT_OBJECT, new MooBoolean(B_TRUE)));
		else if (!strcasecmp(m_token, "#f"))
			return(new MooCodeExpr(m_line, m_col, MCT_OBJECT, new MooBoolean(B_FALSE)));
		else if (parser_is_digit(m_token[0]) || (m_token[0] == '-' && parser_is_digit(m_token[1])))
			return(new MooCodeExpr(m_line, m_col, MCT_OBJECT, new MooNumber(m_token)));
		else
			return(new MooCodeExpr(m_line, m_col, MCT_IDENTIFIER, new MooString("%s", m_token)));
	    }
	    case TT_OPEN: {
		MooCodeExpr *expr = this->parse_list();
		return(new MooCodeExpr(m_line, m_col, MCT_CALL, expr));
	    }
	    default:
		throw MooException("(%d, %d): Invalid token type, %d", m_line, m_col, m_type);
	}
}

int MooCodeParser::read_token()
{
	char ch;

	m_type = 0;
	m_len = 0;
	m_token[0] = '\0';
	while (m_input[m_pos] != '\0') {
		ch = this->getchar();

		if (parser_is_whitespace(ch))
			continue;
		else if (ch == ';') {
			while (1) {
				ch = this->getchar();
				if (ch == '\0' || ch == '\n')
					break;
			}
		}
		else if (ch == '(') {
			m_type = TT_OPEN;
			break;
		}
		else if (ch == ')') {
			m_type = TT_CLOSE;
			break;
		}
		else if (ch == '\"') {
			m_type = TT_STRING;
			while (m_input[m_pos] != '\0') {
				ch = this->getchar();
 				if (ch == '\"')
					break;
				else if (ch == '\\') {
					ch = this->getchar();
					if (ch == '\0')
						break;
					m_token[m_len] = parser_escape_char(ch);
				}
				else
					m_token[m_len] = ch;
				m_len++;
			}
			m_token[m_len] = '\0';
			break;
		}
		else if (ch == '\'') {
			m_type = TT_STRING;
			while (m_input[m_pos] != '\0') {
				ch = this->getchar();
 				if (ch == '\'')
					break;
				m_token[m_len++] = ch;
			}
			m_token[m_len] = '\0';
			break;
		}
		else {
			m_type = TT_WORD;
			m_token[m_len++] = ch;
			while (m_input[m_pos] != '\0') {
				ch = this->getchar();
				if (parser_is_whitespace(ch))
					break;
 				if (ch == '(')
					throw MooException("(%d, %d): Unexpected open bracket.", m_line, m_col);
 				else if (ch == ')') {
					this->ungetchar();
					break;
				}
				m_token[m_len++] = ch;
			}
			m_token[m_len] = '\0';
			break;
		}
	}
	return(0);
}

int MooCodeParser::generate(MooCodeExpr *expr, char *buffer, int max, MooCodeStyle *style, int level)
{
	int i = 0;

	if (!style)
		style = &moo_style_normal;
	for (; expr && i < max; expr = expr->next()) {
		i += style->indent(&buffer[i], max - i, level);
		if (i >= max)
			break;
		i += MooCodeParser::generate_expr(expr, &buffer[i], max - i, style, level);
		if (i >= max)
			break;
		buffer[i++] = style->m_linebr;
	}
	if (i > 0)
		i--;
	buffer[i] = '\0';
	return(i);
}

int MooCodeParser::generate_expr(MooCodeExpr *expr, char *buffer, int max, MooCodeStyle *style, int level)
{
	int i = 0;
	MooObject *obj;
	MooString *str;
	MooCodeExpr *call;

	if (!style)
		style = &moo_style_normal;
	if (expr->expr_type() == MCT_OBJECT) {
		if (!(obj = expr->value()))
			return(0);
		if ((str = dynamic_cast<MooString *>(obj))) {
			buffer[i++] = '\"';
			i += MooCodeParser::encode_string(str->m_str, &buffer[i], max - i);
			buffer[i++] = '\"';
		}
		else
			i += obj->to_string(&buffer[i], max - i);
	}
	else if (expr->expr_type() == MCT_IDENTIFIER) {
		if (!(obj = expr->value()))
			return(0);
		i += obj->to_string(&buffer[i], max - i);
	}
	else if (expr->expr_type() == MCT_CALL) {
		call = dynamic_cast<MooCodeExpr *>(expr->value());
		buffer[i++] = '(';
		if (call) {
			i += MooCodeParser::generate_expr(call, &buffer[i], max - i, style, level + 1);
			if ((call = call->next())) {
				buffer[i++] = style->m_linebr;
				i += MooCodeParser::generate(call, &buffer[i], max - i, style, level + 1);
			}
		}
		buffer[i++] = ')';
	}
	buffer[i] = '\0';
	return(i);
}

#define MAX_CODE	2048

void MooCodeParser::print(MooCodeExpr *expr)
{
	char buffer[MAX_CODE];

	MooCodeParser::generate(expr, buffer, MAX_CODE);
	moo_status("CODE: \n%s", buffer);
}

int MooCodeParser::encode_string(const char *str, char *buffer, int max)
{
	int i = 0, j = 0;

	for (; str[i] != '\0' && j < max; i++, j++) {
		switch (str[i]) {
		    case '\x09':
			buffer[j++] = '\\';
			buffer[j] = 't';
			break;
		    case '\x0d':
			buffer[j++] = '\\';
			buffer[j] = 'r';
			break;
		    case '\x0a':
			buffer[j++] = '\\';
			buffer[j] = 'n';
			break;
		    default:
			buffer[j] = str[i];
			break;
		}
	}
	return(j);
}

MooCodeStyle::MooCodeStyle(const char *indent, char linebr, int wrap)
{
	m_indent = indent;
	m_indent_len = indent ? strlen(indent) : 0;
	m_linebr = linebr;
	m_wrap = wrap;
}

int MooCodeStyle::linebr(char *buffer, int max)
{
	if (max <= 0)
		return(0);
	buffer[0] = m_linebr;
	return(1);
}

int MooCodeStyle::indent(char *buffer, int max, int level)
{
	int i = 0;

	if (m_indent) {
		for (int j = 0; i < max && j < level; i += m_indent_len, j++)
			strncpy(&buffer[i], m_indent, m_indent_len);
	}
	return(i);
}

