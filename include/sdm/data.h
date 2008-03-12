/*
 * Header Name:	data.h
 * Description:	Data Reader/Writer Header
 */

#ifndef _SDM_DATA_H
#define _SDM_DATA_H

#include <sdm/globals.h>

#define SDM_DATA_READ		0x01
#define SDM_DATA_WRITE		0x02
#define SDM_DATA_READ_WRITE	0x03

struct sdm_data_file;

int init_data(void);
int release_data(void);

int sdm_set_data_path(const char *);
int sdm_data_file_exists(const char *);

struct sdm_data_file *sdm_data_open(const char *, int, const char *);
void sdm_data_close(struct sdm_data_file *);

int sdm_data_read_rewind(struct sdm_data_file *);
int sdm_data_read_next(struct sdm_data_file *);
int sdm_data_read_children(struct sdm_data_file *);
int sdm_data_read_parent(struct sdm_data_file *);

const char *sdm_data_read_name(struct sdm_data_file *);
int sdm_data_read_attrib(struct sdm_data_file *, const char *, char *, int);
long int sdm_data_read_integer(struct sdm_data_file *);
double sdm_data_read_float(struct sdm_data_file *);
int sdm_data_read_string(struct sdm_data_file *, char *, int);

int sdm_data_write_begin_entry(struct sdm_data_file *, const char *);
int sdm_data_write_attrib(struct sdm_data_file *, const char *, const char *);
int sdm_data_write_integer(struct sdm_data_file *, long int);
int sdm_data_write_float(struct sdm_data_file *, double);
int sdm_data_write_string(struct sdm_data_file *, const char *);
int sdm_data_write_end_entry(struct sdm_data_file *);
int sdm_data_write_current(struct sdm_data_file *);

int sdm_data_write_integer_entry(struct sdm_data_file *, const char *, long int);
int sdm_data_write_float_entry(struct sdm_data_file *, const char *, double);
int sdm_data_write_string_entry(struct sdm_data_file *, const char *, const char *);

#endif


