/*
 * Object Name:	config.h
 * Description:	Config Object
 */

#ifndef _SDM_OBJS_CONFIG_H
#define _SDM_OBJS_CONFIG_H

#include <string>

#include <sdm/objs/object.h>

class MooConfig : public MooObject {
	std::string *m_filename;

    public:
	MooConfig(const char *filename);
	virtual ~MooConfig();

	virtual int read_entry(const char *type, MooDataFile *);
	virtual int write_data(MooDataFile *);
};

int load_global_config();

#endif


