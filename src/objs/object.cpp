/*
 * Object Name:	object.c
 * Description:	Base Object
 */

#include <stdarg.h>
#include <string.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/things/thing.h>
#include <sdm/tasks/task.h>
#include <sdm/objs/object.h>

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


MooObject *moo_make_object(const MooObjectType *type)
{
	if (!type)
		return(NULL);
	try {
		return(type->m_create());
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
	int tmp;
	moo_id_t current;

	current = MooTask::current_owner();
	if (MooThing::is_wizard(current))
		return(1);
	if (m_owner == MooTask::current_owner())
		perms <<= 3;
	tmp = m_permissions & perms;
	if (tmp == perms)
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
		if (!(type = data->read_name()))
			break;
		this->read_entry(type, data);
		if (res < 0)
			error = 1;
		/// We handled the whole rest of the data so just exit
		else if (res == MOO_HANDLED_ALL)
			return(error);
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
	char *action_name, *remain;
	char buffer[STRING_SIZE];

	strncpy(buffer, name, STRING_SIZE);
	buffer[STRING_SIZE - 1] = '\0';

	if ((action_name = strchr(buffer, ':'))) {
		*action_name = '\0';
		action_name++;
	}

	if ((remain = strchr(buffer, '.'))) {
		*remain = '\0';
		remain++;
	}

	if (!(obj = MooThing::reference(buffer))
	    && !(obj = env->get(buffer, NULL))
	    && !(obj = global_env->get(buffer, NULL)))
		return(NULL);
	if (remain && !(obj = obj->resolve_property(remain, value)))
		return(NULL);

	if (action_name) {
		// TODO why do we assume the object is a thingref, basically... we kinda need to just use thing instead of thingref =/
		MooThing *thing;
		if (!(thing = obj->get_thing()) && !(obj = ((MooObject *) thing)->access_method(action_name, value)))
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

int MooObject::evaluate(MooObjectHash *env, MooArgs *args)
{
	int res;

	this->check_throw(MOO_PERM_X);

	// TODO where would this go??
	//args->m_this = this;

	if (this->permissions() & MOO_PERM_SUID)
		res = MooTask::suid_evaluate(this, env, args);
	else
		res = this->do_evaluate(env, args);
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


