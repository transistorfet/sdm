/*
 * Name:	builtin.c
 * Description:	Builtin Stuff
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

#include <sdm/code/code.h>

#include <sdm/things/thing.h>
#include <sdm/actions/builtin/builtin.h>

#define BUILTIN_LIST_SIZE	32
#define BUILTIN_LIST_BITS	MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL | MOO_HBF_DELETE

MooObjectType moo_builtin_obj_type = {
	NULL,
	"builtin",
	typeid(MooBuiltin).name(),
	(moo_type_create_t) moo_builtin_create
};

extern int moo_load_moocode_actions(MooObjectHash *env);
extern int moo_load_basic_actions(MooObjectHash *env);
extern int moo_load_builder_actions(MooObjectHash *env);
extern int moo_load_channel_actions(MooObjectHash *env);
extern int moo_load_item_actions(MooObjectHash *env);
extern int moo_load_mobile_actions(MooObjectHash *env);
extern int moo_load_room_actions(MooObjectHash *env);
extern int moo_load_user_actions(MooObjectHash *env);

int init_builtin(void)
{
	moo_object_register_type(&moo_builtin_obj_type);

	moo_load_moocode_actions(global_env);
	moo_load_basic_actions(global_env);
	moo_load_builder_actions(global_env);
	moo_load_channel_actions(global_env);
	moo_load_item_actions(global_env);
	moo_load_mobile_actions(global_env);
	moo_load_room_actions(global_env);
	moo_load_user_actions(global_env);
	return(0);
}

void release_builtin(void)
{
	moo_object_deregister_type(&moo_builtin_obj_type);
}

MooObject *moo_builtin_create(void)
{
	return(new MooBuiltin());
}

MooBuiltin::MooBuiltin(moo_action_t func, const char *params)
{
	m_func = func;
	m_master = NULL;
	if (params)
		this->params(params);
}

int MooBuiltin::set(const char *name)
{
	MooBuiltin *master;

	if (!(master = dynamic_cast<MooBuiltin *>(global_env->get(name))))
		return(-1);
	m_master = master;
	m_func = master->m_func;
	return(0);
}

const char *MooBuiltin::params(const char *params)
{
	if (params) {
		strncpy(m_params, params, MOO_PARAM_STRING_SIZE);
		m_params[MOO_PARAM_STRING_SIZE - 1] = '\0';
	}
	return(m_params);
}

int MooBuiltin::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "func")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		this->set(buffer);
	}
	else if (!strcmp(type, "params")) {
		char buffer[STRING_SIZE];

		if (data->read_string_entry(buffer, STRING_SIZE) < 0)
			return(-1);
		this->params(buffer);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooBuiltin::write_data(MooDataFile *data)
{
	const char *name;

	MooObject::write_data(data);
	// TODO we should do this in a more stable way, since there is a chance it wont be found in the list or master will be NULL
	if (!(name = global_env->key(m_master)))
		return(-1);
	data->write_string_entry("func", name);
	data->write_string_entry("params", this->params());
	return(0);
}

int MooBuiltin::do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	if (!m_func)
		return(-1);
	if (args->m_args->last() >= 0)
		args->parse_args(this->params(), args->m_args->get_string(0));
	return(m_func(frame, env, args));
}




