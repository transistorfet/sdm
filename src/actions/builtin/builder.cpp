/*
 * Name:	builder.c
 * Description:	Builder Actions
 */

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/args.h>
#include <sdm/objs/float.h>
#include <sdm/objs/integer.h>
#include <sdm/objs/string.h>
#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/world.h>
#include <sdm/code/code.h>


/**

teleport <destination>
@dig <room-name>
@addexit [<room>] <direction> <target>

*/

static int builder_teleport(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *to;

	// TODO what permissions would control teleporting other than wizard
	if (args->m_user->is_wizard()) {
		if ((to = args->m_args->get_thing(0)))
			return(args->m_user->moveto(args->m_user, args->m_channel, to));
	}
	args->m_user->notify(TNT_STATUS, args->m_user, args->m_channel, "Unable to teleport to that location");
	return(0);
}

static int builder_create(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
/*
	struct sdm_thing *obj;

	if (sdm_interpreter_parse_args(args, 1) < 0) {
		sdm_notify(args->caller, args, "<red>Unable to find the given parent.\n");
		return(0);
	}

	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(args->obj)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, args->obj->id))))) {
		sdm_notify(args->caller, args, "<red>Error creating object.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	sdm_moveto(args->caller, obj, args->caller, NULL);
	sdm_notify(args->caller, args, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
*/
	return(0);
}

static int builder_dig(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{

	// @dig "Name"

/*
	struct sdm_thing *obj;
	struct sdm_thing *room;

	if (!(room = sdm_interpreter_find_thing(NULL, "/core/room")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(room)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, room->id))))) {
		sdm_notify(args->caller, args, "<red>Error creating room.\n");
		return(-1);
	}

	sdm_set_string_property(obj, "name", args->text);
	// TODO this is a dangerous dereference
	sdm_moveto(args->caller, obj, args->caller->location->location, NULL);
	sdm_notify(args->caller, args, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
*/
	return(0);
}

static int builder_add_exit(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
/*
	int i = 0;
	char buffer[STRING_SIZE];
	struct sdm_thing *obj, *exit, *target;

	if (!(exit = sdm_interpreter_find_thing(NULL, "/core/exit")))
		return(-1);
	if (!(obj = SDM_THING(create_sdm_object(SDM_OBJECT(exit)->type, 2, SDM_THING_ARGS(SDM_NEW_ID, exit->id))))) {
		sdm_notify(args->caller, args, "<red>Error creating exit.\n");
		return(-1);
	}

	sdm_interpreter_get_string(args->text, buffer, STRING_SIZE, &i);
	sdm_set_string_property(obj, "name", buffer);
	if ((target = sdm_interpreter_get_thing(args->caller, &args->text[i], &i)))
		sdm_set_number_property(obj, "target", target->id);

	// TODO this is a somewhat dangerous dereference
	sdm_moveto(args->caller, obj, args->caller->location, NULL);
	sdm_notify(args->caller, args, "<green>Object #%d created successfully.\n", obj->id);
	//args->result = SDM_OBJECT(obj);
*/
	return(0);
}

static int builder_info(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooThing *object, *tmp;

	if (!(object = args->m_args->get_thing(0)))
		;//object = args->m_this;
	// TODO display all detailed information
	args->m_user->notifyf(TNT_STATUS, NULL, args->m_channel, "<green>ID: %s (#%d)", object->name(), object->id());
	args->m_user->notifyf(TNT_STATUS, NULL, args->m_channel, "<green>PARENT: %s (#%d)", object->parent()->name(), object->parent_id());
	if ((tmp = object->owner_thing()))
		args->m_user->notifyf(TNT_STATUS, NULL, args->m_channel, "<green>OWNER: %s (#%d)", tmp->name(), tmp->id());
	if ((tmp = object->location()))
		args->m_user->notifyf(TNT_STATUS, NULL, args->m_channel, "<green>LOCATION: %s (#%d)", tmp->name(), tmp->id());
	return(0);
}

static int builder_set(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
/*
	int i = 0;
	char prop[STRING_SIZE];
	struct sdm_string *string;

	if (sdm_interpreter_get_string(args->text, prop, STRING_SIZE, &i) <= 0) {
		sdm_notify(args->caller, args, "<red>Invalid property.\n");
		return(-1);
	}
	if (!args->obj && !(args->obj = sdm_interpreter_get_thing(args->caller, &args->text[i], &i))) {
		sdm_notify(args->caller, args, "<red>Object not found.\n");
		return(-1);
	}

	if (!(string = create_sdm_string(&args->text[i])) || (sdm_thing_set_property(args->obj, prop, SDM_OBJECT(string)) >= 0))
		sdm_notify(args->caller, args, "<green>Property set successfully.\n");
	else
		sdm_notify(args->caller, args, "<red>Error setting property.\n");
*/
	return(0);
}

static int builder_save(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args)
{
	MooWorld *root;

	if (!(root = MooWorld::root()))
		return(-1);
	root->write();
	return(0);
}


int moo_load_builder_actions(MooObjectHash *env)
{
	env->set("builder_teleport", new MooCodeFunc(builder_teleport));
	env->set("builder_create", new MooCodeFunc(builder_create));
	env->set("builder_dig", new MooCodeFunc(builder_dig));
	env->set("builder_add_exit", new MooCodeFunc(builder_add_exit));
	env->set("builder_info", new MooCodeFunc(builder_info));
	env->set("builder_set", new MooCodeFunc(builder_set));
	env->set("builder_save", new MooCodeFunc(builder_save));
	return(0);
}



