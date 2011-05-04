/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/globals.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/args.h>
#include <sdm/objs/object.h>
#include <sdm/objs/string.h>
#include <sdm/funcs/func.h>
#include <sdm/funcs/method.h>
#include <sdm/tasks/task.h>
#include <sdm/things/thing.h>
#include <sdm/things/user.h>

#include <sdm/code/code.h>

#define TYPE_INIT_SIZE		32

const MooObjectType moo_object_obj_type = {
	NULL,
	"object",
	typeid(MooObject).name(),
	NULL
};

extern MooObjectHash *global_env;

static MooHash<const MooObjectType *> *name_type_list = NULL;
static MooHash<const MooObjectType *> *realname_type_list = NULL;

int init_object(void)
{
	if (name_type_list)
		return(1);
	name_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	realname_type_list = new MooHash<const MooObjectType *>(TYPE_INIT_SIZE, MOO_HBF_REPLACE | MOO_HBF_REMOVE);
	moo_object_register_type(&moo_object_obj_type);
	return(0);
}

void release_object(void)
{
	if (!name_type_list)
		return;
	delete realname_type_list;
	delete name_type_list;
	name_type_list = NULL;
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

const MooObjectType *moo_object_find_type(const char *name, const MooObjectType *base)
{
	const MooObjectType *type, *cur;

	if (!(type = name_type_list->get(name)))
		return(NULL);
	/// If base is given, then only return this type if it is a subclass of base
	if (!base)
		return(type);
	for (cur = type; cur; cur = cur->m_parent) {
		if (cur == base)
			return(type);
	}
	return(NULL);
}


MooObject *moo_make_object(const MooObjectType *type, MooDataFile *data)
{
	if (!type)
		return(NULL);
	try {
		return(type->m_make(data));
	} catch(...) {
		return(NULL);
	}
}

MooObject::MooObject()
{
	m_delete = 0;
	m_owner = MooTask::current_owner();
	m_permissions = MOO_DEFAULT_PERMS;
}

const MooObjectType *MooObject::type()
{
	const MooObjectType *type;
	type = realname_type_list->get(typeid(*this).name());
	if (!type)
		type = name_type_list->get("object");
	return(type);
}


moo_id_t MooObject::owner(moo_id_t id)
{
	if (this == MooTask::current_task())
		MooTask::current_owner(id);
	return(m_owner = id);
}

moo_perm_t MooObject::permissions(moo_perm_t perms)
{
	// TODO this really sucks.  perms of 0 are invalid.  This is here because when you load perms from XML, if the
	//	tag is not there, it will read as 0, but if perms are not specified, we should assume default_perms
	if (!perms)
		perms = MOO_DEFAULT_PERMS;
	return(m_permissions = perms);
}

int MooObject::is_true()
{
	if (this->get_float() == 0)
		return(0);
	return(1);
}

void MooObject::check_throw(moo_perm_t perms)
{
	if (!this->check(perms))
		throw moo_permissions;
}

int MooObject::check(moo_perm_t perms)
{
	moo_id_t current;

	current = MooTask::current_owner();
	if (MooThing::is_wizard(current))
		return(1);
	if (m_owner == MooTask::current_owner())
		perms <<= 3;
	if ((m_permissions & perms) == perms)
		return(1);
	return(0);
}


int MooObject::read_file(const char *file, const char *type)
{
	int res = -1;
	MooDataFile *data;

	try {
		data = new MooDataFile(file, MOO_DATA_READ, type);
		moo_status("Reading %s data from file \"%s\".", type, file);
		res = this->read_data(data);
		delete data;
		return(res);
	}
	catch (MooException e) {
		moo_status("DATA: %s", e.get());
		return(-1);
	}
	catch (...) {
		moo_status("Error opening file \"%s\".", file);
		return(-1);
	}
}

int MooObject::write_file(const char *file, const char *type)
{
	MooDataFile *data;

	moo_status("Writing %s data to file \"%s\".", type, file);
	data = new MooDataFile(file, MOO_DATA_WRITE, type);
	this->write_data(data);
	delete data;
	return(0);
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

int MooObject::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "owner")) {
		moo_id_t id = data->read_integer_entry();
		this->owner(id);
	}
	else if (!strcmp(type, "permissions")) {
		moo_perm_t perms = data->read_integer_entry();
		this->permissions(perms);
	}
	else
		return(MOO_NOT_HANDLED);
	return(MOO_HANDLED);
}

int MooObject::write_data(MooDataFile *data)
{
	data->write_integer_entry("owner", this->owner());
	data->write_octal_entry("permissions", this->permissions());
	return(0);
}

MooObject *MooObject::resolve(const char *name, MooObjectHash *env, MooObject *value)
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
		if (env->set(buffer, MOO_INCREF(value)))
			return(NULL);
		return(value);
	}
	else {
		// TODO should we modify this so that we never do a global_env lookup and instead rely on the env being linked to global_env
		if (!(obj = MooThing::reference(buffer))
		    && !(obj = env->get(buffer))
		    && !(obj = global_env->get(buffer)))
			return(NULL);
	}
	if (remain && !(obj = obj->resolve_property(remain, value)))
		return(NULL);

	if (method) {
		if (!(obj = obj->resolve_method(method, value)))
			return(NULL);
	}
	obj->check_throw(MOO_PERM_R);
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
		// TODO does permissions checks actually have to be in the access function, to properly check before setting the value?
		//obj->check_throw(MOO_PERM_R);
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
	if (dynamic_cast<MooMethod *>(func))
		return(func);
	// TODO this is a memory leak i think, because the return'd pointer is assumed to be borrowed
	return(new MooMethod(this, func));
}

// TODO should this be somewhere else where we can more generically put the parameter parsing? (ie. get it out of MooArgs)
int MooObject::call_method(MooObject *channel, const char *name, const char *text, MooObject **result)
{
	int res;
	MooArgs *args;
	MooObject *func;

	if (!(func = this->resolve_method(name)))
		return(-1);
	args = new MooArgs();
	if (text)
		args->m_args->set(0, new MooString(text));
	res = this->call_method(channel, func, args);
	if (result) {
		*result = args->m_result;
		args->m_result = NULL;
	}
	MOO_DECREF(args);
	return(res);
}

int MooObject::call_method(MooObject *channel, MooObject *func, MooArgs *args)
{
	int res;
	MooObjectHash *env;

	env = new MooObjectHash();
	env->set("user", MooThing::lookup(MooTask::current_user()));
	env->set("channel", channel);
	res = this->call_method(func, env, args);
	MOO_DECREF(env);
	return(res);
}

int MooObject::call_method(MooObject *func, MooObjectHash *env, MooArgs *args)
{
	int res;
	MooCodeFrame *frame;

	frame = new MooCodeFrame(env);
	// TODO is this no longer needed here (should be done in MooMethod do_evaluate)
	args->m_this = this;
	try {
		frame->push_call(frame->env(), func, args);
		res = frame->run();
	}
	catch (MooException e) {
		// TODO temporary for debugging purposes??
		moo_status("CODE: %s", e.get());
		frame->print_stack();

		// TODO this is totally horrible and ugly! what is a better way to print an error
		MooUser *user;
		MooThing *thing = MooThing::lookup(MooTask::current_user());
		if ((user = dynamic_cast<MooUser *>(thing)))
			// TODO send this to the current channel if possible
			user->notify(TNT_STATUS, NULL, NULL, e.get());
		res = -1;
	}
	args->m_result = frame->get_return();
	MOO_DECREF(frame);
	return(res);
}

// TODO should this function be private or something? I think it's only ever called by MooCodeEventCallExpr
int MooObject::evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	int res;

	this->check_throw(MOO_PERM_X);
	if (this->permissions() & MOO_PERM_SUID)
		MooTask::suid(this, frame);
	res = this->do_evaluate(frame, env, args);
	return(res);
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


