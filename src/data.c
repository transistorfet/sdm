/*
 * Module Name:	data.c
 * Description:	Data Reader/Writer
 */

#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <sdm/data.h>
#include <sdm/string.h>
#include <sdm/memory.h>

#define XMLNODE(ptr)		( (xmlNode *) (ptr) )

struct sdm_data_file {
	string_t filename;
	xmlDoc *doc;
	xmlNode *main;
};

static string_t data_path = NULL;

int init_data(void)
{
	LIBXML_TEST_VERSION
	return(0);
}

int release_data(void)
{
	xmlCleanupParser();
	return(0);
}

int sdm_set_data_path(const char *path)
{
	if (data_path)
		destroy_string(data_path);
	if (!(data_path = create_string("%s/", path)))
		return(-1);
	return(0);
}

int sdm_data_file_exists(const char *file)
{
	FILE *fptr;
	char buffer[STRING_SIZE];

	if (!data_path)
		return(0);
	snprintf(buffer, STRING_SIZE, "%s%s", data_path, file);
	if (!(fptr = fopen(buffer, "r")))
		return(0);
	fclose(fptr);
	return(1);
}


struct sdm_data_file *sdm_data_new(const char *file, const char *rootname)
{
	// TODO create a new file
	return(NULL);
}

struct sdm_data_file *sdm_data_open(const char *file, const char *rootname)
{
	xmlNode *root;
	struct sdm_data_file *data;

	if (!data_path)
		return(NULL);
	if (!(data = (struct sdm_data_file *) memory_alloc(sizeof(struct sdm_data_file))))
		return(NULL);
	memset(data, '\0', sizeof(struct sdm_data_file));
	if (!(data->filename = create_string("%s%s", data_path, file))
	    || !(data->doc = xmlReadFile(data->filename, NULL, 0))
	    || !(root = xmlDocGetRootElement(data->doc)) || xmlStrcmp(root->name, rootname)) {
		sdm_data_close(data);
		return(NULL);
	}
	data->main = root->children;
	return(data);
}

int sdm_data_write(struct sdm_data_file *data)
{
	return(xmlSaveFormatFile(data->filename, data->doc, 1));
}

void sdm_data_close(struct sdm_data_file *data)
{
	if (data->filename)
		destroy_string(data->filename);
	if (data->doc)
		xmlFreeDoc(data->doc);
	memory_free(data);
}

struct sdm_data_entry *sdm_data_first(struct sdm_data_file *data)
{
	return((struct sdm_data_entry *) data->main);

}

struct sdm_data_entry *sdm_data_next(struct sdm_data_file *data, struct sdm_data_entry *entry)
{
	if (XMLNODE(entry))
		return((struct sdm_data_entry *) XMLNODE(entry)->next);
	return(NULL);
}

struct sdm_data_entry *sdm_data_children(struct sdm_data_file *data, struct sdm_data_entry *entry)
{
	if (XMLNODE(entry))
		return((struct sdm_data_entry *) XMLNODE(entry)->children);
	return(NULL);
}


const char *sdm_data_entry_name(struct sdm_data_file *data, struct sdm_data_entry *entry)
{
	if (XMLNODE(entry))
		return(XMLNODE(entry)->name);
	return(NULL);
}

double sdm_data_entry_number(struct sdm_data_file *data, struct sdm_data_entry *entry)
{
	char *str;
	double ret = 0;

	if (!XMLNODE(entry))
		return(0);
	if (!(str = xmlNodeListGetString(data->doc, XMLNODE(entry)->children, 1)))
		return(0);
	ret = atof(str);
	xmlFree(str);
	return(ret);
}

int sdm_data_entry_string(struct sdm_data_file *data, struct sdm_data_entry *entry, char *buffer, int max)
{
	char *str;

	if (!(str = xmlNodeListGetString(data->doc, XMLNODE(entry)->children, 1)))
		return(-1);
	strncpy(buffer, (char *) str, max - 1);
	buffer[max - 1] = '\0';
	xmlFree(str);
	return(0);
}

int sdm_data_find_entry(struct sdm_data_file *data, const char *name, char *buffer, int max)
{
	char *str;
	xmlNode *cur;

	for (cur = data->main; cur; cur = cur->next) {
		if (!xmlStrcmp(cur->name, (const xmlChar *) name)) {
			if (!(str = xmlNodeListGetString(data->doc, XMLNODE(cur)->children, 1)))
				return(0);
			strncpy(buffer, (char *) str, max - 1);
			buffer[max - 1] = '\0';
			xmlFree(str);
			return(1);
		}
	}
	return(0);
}



