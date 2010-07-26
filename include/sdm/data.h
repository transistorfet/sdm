/*
 * Header Name:	data.h
 * Description:	Data Reader/Writer Header
 */

#ifndef _MOO_DATA_H
#define _MOO_DATA_H

#include <sdm/globals.h>
#include <sdm/misc.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#define MOO_DATA_READ		0x01
#define MOO_DATA_WRITE		0x02
#define MOO_DATA_READ_WRITE	0x03

class MooDataFile {
    private:
	string_t filename;
	xmlDoc *doc;
	xmlNode *root;
	xmlNode *current;
	xmlTextWriterPtr writer;
	xmlBufferPtr buffer;

    public:
	MooDataFile(const char *file, int mode, const char *rootname);
	~MooDataFile();

	int read_rewind();
	int read_next();
	int read_children();
	int read_parent();

	const char *read_name();
	int read_attrib(const char *name, char *buffer, int max);
	long int read_integer();
	double read_float();
	int read_string(char *buffer, int max);
	int read_raw_string(char *buffer, int max);

	long int read_integer_entry();
	double read_float_entry();
	int read_string_entry(char *buffer, int max);
	int read_raw_string_entry(char *buffer, int max);

	int write_begin_entry(const char *name);
	int write_attrib(const char *name, const char *value);
	int write_integer(long int value);
	int write_float(double value);
	int write_string(const char *value);
	int write_raw_string(const char *value);
	int write_end_entry();
	int write_current();

	int write_integer_entry(const char *name, long int value);
	int write_float_entry(const char *name, double value);
	int write_string_entry(const char *name, const char *value);
};

int init_data(void);
void release_data(void);

int moo_set_data_path(const char *);
int moo_data_file_exists(const char *);

#endif


