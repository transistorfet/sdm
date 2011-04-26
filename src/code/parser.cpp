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
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>

#include <sdm/things/thing.h>
#include <sdm/code/code.h>

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
		return(new MooCodeExpr(m_line, m_col, MCT_OBJECT, new MooString(m_token)));
	    case TT_WORD: {
		if (parser_is_digit(m_token[0]) || (m_token[0] == '-' && parser_is_digit(m_token[1]))) {
			return(new MooCodeExpr(m_line, m_col, MCT_OBJECT, new MooNumber(m_token)));
		}
		else
			return(new MooCodeExpr(m_line, m_col, MCT_IDENTIFIER, new MooString(m_token)));
	    }
	    case TT_OPEN: {
		MooCodeExpr *expr = this->parse_list();
		if (!expr)
			throw MooException("(%d, %d): Empty list?", m_line, m_col);
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
			break;
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

int MooCodeParser::generate(MooCodeExpr *expr, char *buffer, int max, char linebr, int indent)
{
	int i = 0;
	MooObject *obj;
	MooCodeExpr *call;

	for (; expr && i < max; expr = expr->next()) {
		if (indent < 0)
			indent *= -1;
		else {
			for (int j = 0; i < max && j < indent; i++, j++)
				buffer[i] = ' ';
		}


		if (expr->expr_type() == MCT_OBJECT) {
			if (!(obj = expr->value()))
				return(0);
			if (dynamic_cast<MooString *>(obj)) {
				// TODO this really isn't very good.  We don't check and convert control codes to escape characters
				//	and we don't preserve the original quotes.  Do we even need 2 kinds of quoted strings??
				//	Should there be a format/context arg passed to to_string??  We haven't been very consistent in
				//	our use of to_string versus get_string
				// TODO a possible solution is to not use an object here but to use a real string or number since those
				//	should be the only object types we ever find here (unless of course we want to generate some
				//	kind of bizzare intermediate code, but we should never have to do that... I can't think of why
				//	we ever would...)  Oh, or it could be MooCodeExpr the way we have things, because when we
				//	have an MCT_CALL type, it uses the object member to store the expression, but again, if it's a
				//	call type, we should never have anything *except* MooCodeExpr there
				buffer[i++] = '\'';
				i += obj->to_string(&buffer[i], max - i);
				buffer[i++] = '\'';
			}
			else
				i += obj->to_string(&buffer[i], max - i);
			buffer[i++] = linebr;
		}
		else if (expr->expr_type() == MCT_IDENTIFIER) {
			if (!(obj = expr->value()))
				return(0);
			i += obj->to_string(&buffer[i], max - i);
			buffer[i++] = linebr;
		}
		else if (expr->expr_type() == MCT_CALL) {
			if (!(obj = expr->value()))
				return(0);
			if (!(call = dynamic_cast<MooCodeExpr *>(obj)))
				return(0);
			buffer[i++] = '(';
			i += MooCodeParser::generate(call, &buffer[i], max - i, linebr, (indent + 1) * -1);
			if (buffer[i - 1] == linebr)
				i--;
			buffer[i++] = ')';
			buffer[i++] = linebr;
		}
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

