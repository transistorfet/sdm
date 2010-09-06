/*
 * Object Name:	action.c
 * Description:	Action Base Object
 */

#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/things/user.h>
#include <sdm/actions/action.h>

#define MOO_IS_WHITESPACE(ch)	( (ch) == ' ' || (ch) == '\n' || (ch) == '\r' )

static char *moo_prepositions[] = { "from", "on", "with", "to", "at", NULL };


MooObjectType moo_action_obj_type = {
	NULL,
	"action",
	typeid(MooAction).name(),
	(moo_type_create_t) NULL
};

MooAction::MooAction(const char *name, MooThing *thing)
{
	this->init(name, thing);
}

MooAction::~MooAction()
{
	// TODO maybe you could check the thing and make sure the action is removed???
	if (m_name)
		delete m_name;
}

void MooAction::init(const char *name, MooThing *thing)
{
	m_name = name ? new std::string(name) : NULL;
	m_thing = thing;
}


MooArgs::MooArgs()
{
	m_action = NULL;
	m_action_text = NULL;
	m_result = NULL;
	m_user = NULL;
	m_channel = NULL;
	m_caller = NULL;
	m_this = NULL;
	m_object = NULL;
	m_target = NULL;
	m_text = NULL;
}

MooArgs::~MooArgs()
{
	if (m_result)
		delete m_result;
}

int MooArgs::parse_word(char *buffer)
{
	int i;

	for (i = 0; buffer[i] != '\0' && !MOO_IS_WHITESPACE(buffer[i]); i++)
		;
	buffer[i++] = '\0';
	return(i);
}

int MooArgs::parse_whitespace(char *buffer)
{
	int i;

	for (i = 0; buffer[i] != '\0' && MOO_IS_WHITESPACE(buffer[i]); i++)
		;
	return(i);
}

int MooArgs::parse_words(char *buffer)
{
	// TODO should there be a char array in the args? or passed to this function?
	// TODO should this get the buffer? or should the buffer be automatically added and handled by MooArgs? (stored in m_buffer)
}

int MooArgs::parse_args(MooThing *user, char *buffer, int max, const char *action, const char *text)
{
	int i;
	int res;

	if (text) {
		strncpy(buffer, text, max - 1);
		buffer[max - 1] = '\0';
		res = this->parse_args(user, action, buffer);
		m_text = text;
	}
	else {
		strncpy(buffer, action, max - 1);
		buffer[max - 1] = '\0';
		res = this->parse_args(user, buffer, &i);
		m_text = &action[i];
	}
	return(res);
}

int MooArgs::parse_args(MooThing *user, char *buffer, int *argpos)
{
	int i;
	char *action;

	/// Parse out the action string
	i = this->parse_whitespace(buffer);
	action = &buffer[i];
	i += this->parse_word(&buffer[i]);
	if (argpos)
		*argpos = i;
	return(this->parse_args(user, action, &buffer[i]));
}

int MooArgs::parse_args(MooThing *user, const char *action, char *buffer)
{
	int i = 0, j, k, len;
	const char *objname;
	MooThing *object = NULL, *target = NULL;

	// TODO we need to get m_text somehow (string without the action) but since we cut up this buffer, we need another buffer
	//	The main trouble is that we'd have to parse out the action twice
	objname = &buffer[i];
	while (buffer[i] != '\0') {
		for (; buffer[i] != '\0' && MOO_IS_WHITESPACE(buffer[i]); i++)
			;
		k = i - 1;
		for (j = 0; moo_prepositions[j] != NULL; j++) {
			len = strlen(moo_prepositions[j]);
			if (!strncmp(&buffer[i], moo_prepositions[j], len) && MOO_IS_WHITESPACE(buffer[i + len])) {
				i += len;
				for (; buffer[i] != '\0' && MOO_IS_WHITESPACE(buffer[i]); i++)
					;
				/// Isolate the object name
				if (k >= 0)
					buffer[k] = '\0';
				else
					objname = "";
				target = user->find(&buffer[i]);
				break;
			}
		}
		for (; buffer[i] != '\0' && !MOO_IS_WHITESPACE(buffer[i]); i++)
			;
	}
	// TODO if a name is specified (not ""), and NULL is returned, we should error instead of continuing as normal
	object = user->find(objname);
	
	m_user = user;
	m_caller = (MooThing *) user;
	m_action_text = action;
	m_object = object;
	m_target = target;
	return(0);
}

int MooArgs::parse_args(MooThing *user, MooThing *object, MooThing *target)
{
	// TODO this assumes the current task is owned by the appropriate user
	//	this would be useful for when you don't want to take a user param...
	//m_user = MooThing::lookup(MooTask::current_user());

	m_user = user;
	// TODO this isn't really correct
	m_caller = user;
	m_object = object;
	m_target = target;
	m_text = NULL;
	return(0);
}

