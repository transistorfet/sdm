/*
 * Header Name:	alias.h
 * Description:	Command Aliases Header
 */

#ifndef _SDM_ACTIONS_ALIAS_H
#define _SDM_ACTIONS_ALIAS_H

#include <string>

#include <sdm/hash.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/actions/action.h>

class MooAlias : public MooAction {
	std::string *m_command;
    public:
	MooAlias(MooThing* thing = NULL, const char *command = NULL);
	virtual ~MooAlias();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

    private:
	virtual int do_evaluate(MooObjectHash *env, MooArgs *args);
};

extern MooObjectType moo_alias_obj_type;

MooObject *moo_alias_create(void);

#endif

