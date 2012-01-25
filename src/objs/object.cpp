/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/globals.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/objs/boolean.h>
#include <sdm/objs/string.h>
#include <sdm/objs/funcptr.h>
#include <sdm/objs/thing.h>

#include <sdm/code/code.h>

#define TYPE_INIT_SIZE		32

extern MooObjectHash *global_env;

static MooHash<const MooObjectType *> *name_type_list = NULL;
static MooHash<const MooObjectType *> *realname_type_list = NULL;

int init_object(void)
{
	if (name_type_list)
		return(1);
	name_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	realname_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	return(0);
}

void release_object(void)
{
	if (!name_type_list)
		return;
	delete realname_type_list;
	delete name_type_list;
	name_type_list = NULL;
	realname_type_list = NULL;
}

int moo_object_register_type(const MooObjectType *type)
{
	name_type_list->set(type->m_name, type);
	realname_type_list->set(type->m_realname, type);
	return(0);
}

int moo_object_deregister_type(const MooObjectType *type)
{
	name_type_list->remove(type->m_name);
	realname_type_list->remove(type->m_realname);
	return(0);
}

const MooObjectType *moo_object_find_type(const char *name)
{
	return(name_type_list->get(name));
}

MooObject::MooObject()
{
	m_bitflags = 0;
}

#define DEBUG_LIMIT	77

void MooObject::print_debug()
{
	char buffer[STRING_SIZE];

	//MooCodeParser::generate(m_expr, buffer, LARGE_STRING_SIZE, &moo_style_one_line);
	if (this->to_string(buffer, DEBUG_LIMIT) >= DEBUG_LIMIT)
		strcpy(&buffer[DEBUG_LIMIT], "...");
	moo_status("DEBUG: (%s) %s", typeid(*this).name(), buffer);
}

const MooObjectType *MooObject::objtype()
{
	const MooObjectType *type;
	type = realname_type_list->get(typeid(*this).name());
	if (!type)
		type = name_type_list->get("object");
	return(type);
}

int MooObject::is_true()
{
	MooBoolean *b;

	// TODO change this to a static call??
	if (!(b = dynamic_cast<MooBoolean *>(this)))
		return(1);
	return(b->value() != B_FALSE);
}

int MooObject::read_data(MooDataFile *data)
{
	int res;
	int error = 0;
	const char *type;

	do {
		type = NULL;
		try {
			if (!(type = data->read_name()))
				break;
			res = this->read_entry(type, data);
			if (res < 0)
				error = 1;
			/// We handled the whole rest of the data so just exit
			else if (res == MOO_HANDLED_ALL)
				return(error);
		}
		catch (MooException e) {
			if (type)
				moo_status("DATA: (at <%s>) %s", type, e.get());
			else
				moo_status("DATA: %s", e.get());
			return(-1);
		}
	} while (data->read_next());
	/// We return if the file loaded incorrectly but we don't stop trying to load the file
	return(error);
}

MooObject *MooObject::read_object(MooDataFile *data, const char *type)
{
	int res;
	MooObject *obj = NULL;
	const MooObjectType *objtype;

	if (!(objtype = moo_object_find_type(type)) || !objtype->m_load) {
		moo_status("OBJ: No such type, %s", type);
		return(NULL);
	}
	res = data->read_children();
	try {
		obj = objtype->m_load(data);
	} catch(MooException e) {
		moo_status("OBJ: Error loading object of type %s", type);
	}
	if (res)
		data->read_parent();
	return(obj);
}

MooObject *MooObject::resolve(const char *name, MooObjectHash *env, MooObject *value, MooObject **parent)
{
	MooObject *obj;
	char *method, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	if ((method = strchr(buffer, ':'))) {
		*method = '\0';
		method++;
	}

	if ((remain = strchr(buffer, '.'))) {
		*remain = '\0';
		remain++;
	}

	if (!method && !remain && value) {
		if (!env)
			env = global_env;
		// TODO check perms
		if (env->set(buffer, MOO_INCREF(value)))
			return(NULL);
		return(value);
	}
	else {
		// TODO should we modify this so that we never do a global_env lookup and instead rely on the env being linked to global_env
		if (!(obj = MooMutable::reference(buffer))
		    && !(obj = env->get(buffer))
		    && !(obj = global_env->get(buffer)))
			return(NULL);
	}
	if (remain && !(obj = obj->resolve_property(remain, method ? NULL : value)))
		return(NULL);

	if (method) {
		if (parent)
			*parent = obj;
		if (!(obj = obj->resolve_method(method, value)))
			return(NULL);
	}
	return(obj);
}

MooObject *MooObject::resolve_property(const char *name, MooObject *value)
{
	MooObject *obj;
	char *str, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	// TODO should you throw an error if we can't set the value, instead of just returing NULL?
	for (str = buffer, obj = this; str && *str != '\0' && obj; str = remain) {
		if ((remain = strchr(buffer, '.'))) {
			*remain = '\0';
			remain++;
			if (!(obj = obj->access_property(str, NULL)))
				return(NULL);
		}
		else
			return(obj->access_property(str, value));
	}
	return(NULL);
}

MooObject *MooObject::resolve_method(const char *name, MooObject *value)
{
	MooObject *func;

	if (!(func = this->access_method(name, value)))
		return(NULL);
	return(func);
}

int MooObject::call_method(MooObject *channel, const char *name, MooObject **result, MooObject *obj1, MooObject *obj2, MooObject *obj3, MooObject *obj4, MooObject *obj5)
{
	int res;
	clock_t start;
	MooObject *func;
	MooObjectHash *env;
	MooObjectArray *args;
	MooCodeFrame *frame;

	if (!(func = this->resolve_method(name)))
		return(-1);
	args = new MooObjectArray();
	args->set(0, this);
	if (obj1) {
		args->set(1, obj1);
		if (obj2) {
			args->set(2, obj2);
			if (obj3) {
				args->set(3, obj3);
				if (obj4) {
					args->set(4, obj4);
					if (obj5)
						args->set(5, obj5);
				}
			}
		}
	}

	frame = new MooCodeFrame();
	frame->set_user_channel(MooThing::lookup(MooTask::current_user()), channel);

	start = clock();
	try {
		frame->push_call(frame->env(), func, args);
		frame->run();
	}
	catch (MooException e) {
		// TODO temporary for debugging purposes??
		moo_status("CODE: %s", e.get());
		frame->print_stacktrace();

		// TODO you need some way of reporting the error back to the user
		/*
		channel = dynamic_cast<MooThing *>(env->get("channel"));
		if ((thing = MooThing::lookup(MooTask::current_user())))
			thing->notify(TNT_STATUS, NULL, channel, e.get());
		*/
		res = -1;
	}
	if (result)
		 *result = frame->get_return();
	//moo_status("Executed (%s ...) in %f seconds", name, ((float) clock() - start) / CLOCKS_PER_SEC);

	// TODO this seems kinda... bad
	frame = NULL;	/// Release frame
	env = NULL;	/// Release environment
	args = NULL;	/// Release args
	return(res);
}

int MooObject::evaluate(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args)
{
	// TODO PERMS this no longer makse sense because we can't get owner from 'this'
	frame->elevate(this);
	return(this->do_evaluate(frame, env, args));
}


///// String Parsers /////

/**
 * Format a string using the given fmt string and place the resulting
 * string into the given buffer.  The number of characters written to
 * the buffer is returned.  If a $ is encountered, the substring up
 * to the next space is taken to be a reference name.  If the reference
 * name is enclosed in { }, then the reference name is take to be
 * up to the closing }.  The reference is evaluated into a string using
 * the MooObject::expand_reference function.  If a & follows the $ then the
 * resolved string is recursively expanded.
 */
int MooObject::format(char *buffer, int max, MooObjectHash *env, const char *fmt)
{
	int i;
	int j = 0;

	max--;
	for (i = 0;(fmt[i] != '\0') && (j < max);i++) {
		if (fmt[i] == '\\') {
			if (fmt[++i] == '\0')
				break;
			i += MooObject::escape_char(&fmt[i], &buffer[j++]) - 1;
		}
		else if (fmt[i] == '$')
			j += MooObject::expand_reference(&buffer[j], max - j + 1, env, &fmt[i], &i);
		else
			buffer[j++] = fmt[i];
	}
	buffer[j] = '\0';
	return(j);
}

#define IS_NUMBER_CHAR(ch)	\
	( ((ch) >= 0x30) && ((ch) <= 0x39) )

#define IS_VALID_CHAR(ch)	\
	( (((ch) >= '0') && ((ch) <= '9'))	\
	|| (((ch) >= 'A') && ((ch) <= 'Z')) || (((ch) >= 'a') && ((ch) <= 'z'))	\
	|| ((ch) == '.') || ((ch) == '_') )

/**
 * Expand the reference using the given args and copy the resulting string
 * to the given buffer up to the max number of characters.  The number
 * of characters written to the buffer is returned.  If the given str_count
 * pointer is not NULL then the number of characters used as the variable
 * name in str is added to the value it points to.  The given str may
 * start with the '$' char or may start just after it.
 */
int MooObject::expand_reference(char *buffer, int max, MooObjectHash *env, const char *str, int *used)
{
	int k;
	char delim;
	int recurse;
	int i = 0, j = 0;
	MooObject *obj;

	if (str[i] == '$')
		i++;

	if (str[i] == '&') {
		recurse = 1;
		i++;
	}
	else
		recurse = 0;

	if (str[i] == '{') {
		delim = '}';
		i++;
	}
	else
		delim = ' ';

	for (k = 0; (str[i] != '\0') && (str[i] != delim) && IS_VALID_CHAR(str[i]) && (k < max - 1); k++, i++)
		buffer[k] = str[i];
	buffer[k] = '\0';
	if ((obj = MooObject::resolve(buffer, env, NULL))) {
		j = obj->to_string(buffer, max);
		if (recurse) {
			char value[STRING_SIZE];
			strncpy(value, buffer, STRING_SIZE);
			value[STRING_SIZE - 1] = '\0';
			j = MooObject::format(buffer, max, env, value);
		}
	}
	if (delim != '}')
		i--;

	if (used)
		*used += i;
	buffer[j] = '\0';
	return(j);
}


/**
 * Convert the charcter escape sequence (assuming str starts after the escape
 * character) and stores the character in buffer[0].  The number of characters
 * read as a sequence from str is returned
 */
int MooObject::escape_char(const char *str, char *buffer)
{
	char number[3];

	if (*str == '\0')
		return(0);
	switch (str[0]) {
		case 'n':
			buffer[0] = '\n';
			break;
		case 'r':
			buffer[0] = '\r';
			break;
		case 't':
			buffer[0] = '\t';
			break;
		case 'e':
			buffer[0] = '\x1b';
			break;
		case 'x':
			if ((str[1] != '\0') && (str[2] != '\0')) {
				number[0] = str[1];
				number[1] = str[2];
				number[2] = '\0';
				buffer[0] = strtol(number, NULL, 16);
			}
			return(3);
		default:
			if (IS_NUMBER_CHAR(str[0])) {
				buffer[0] = str[0] - 0x30;
				if (IS_NUMBER_CHAR(str[1])) {
					buffer[0] = (buffer[0] * 8) + str[1] - 0x30;
					if (IS_NUMBER_CHAR(str[2])) {
						buffer[0] = (buffer[0] * 8) + str[2] - 0x30;
						return(3);
					}
					return(2);
				}
				return(1);
			}
			else
				buffer[0] = str[0];
			break;
	}
	return(1);
}


