/*
 * Header Name:	data.h
 * Description:	Data Reader/Writer Header
 */

#ifndef _SDM_DATA_H
#define _SDM_DATA_H

#include <sdm/globals.h>

struct sdm_data_file;
struct sdm_data_entry;

int init_data(void);
int release_data(void);

int sdm_set_data_path(const char *);
int sdm_data_file_exists(const char *);

struct sdm_data_file *sdm_data_new(const char *, const char *);
struct sdm_data_file *sdm_data_open(const char *, const char *);
int sdm_data_write(struct sdm_data_file *);
void sdm_data_close(struct sdm_data_file *);

struct sdm_data_entry *sdm_data_first(struct sdm_data_file *);
struct sdm_data_entry *sdm_data_next(struct sdm_data_file *, struct sdm_data_entry *);
struct sdm_data_entry *sdm_data_children(struct sdm_data_file *, struct sdm_data_entry *);

const char *sdm_data_entry_name(struct sdm_data_file *, struct sdm_data_entry *);
double sdm_data_entry_number(struct sdm_data_file *, struct sdm_data_entry *);
int sdm_data_entry_string(struct sdm_data_file *, struct sdm_data_entry *, char *, int);

int sdm_data_find_entry(struct sdm_data_file *, const char *, char *, int);

#endif


