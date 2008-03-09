/*
 * Module Name:	data.c
 * Description:	Data Reader/Writer
 */

#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include <sdm/data.h>
#include <sdm/string.h>
#include <sdm/memory.h>

struct sdm_data_file {
	string_t filename;
	xmlDoc *doc;
	xmlNode *root;
	xmlNode *current;
	xmlTextWriterPtr writer;
	xmlBufferPtr buffer;
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


struct sdm_data_file *sdm_data_open(const char *file, int mode, const char *rootname)
{
	struct sdm_data_file *data;

	if (!data_path)
		return(NULL);
	if (!(data = (struct sdm_data_file *) memory_alloc(sizeof(struct sdm_data_file))))
		return(NULL);
	memset(data, '\0', sizeof(struct sdm_data_file));
	if (!(data->filename = create_string("%s%s", data_path, file))) {
		sdm_data_close(data);
		return(NULL);
	}

	if (mode & SDM_DATA_READ) {
		if (!(data->doc = xmlReadFile(data->filename, NULL, 0))
		    || !(data->root = xmlDocGetRootElement(data->doc)) || xmlStrcmp(data->root->name, rootname)) {
			sdm_data_close(data);
			return(NULL);
		}
		data->current = data->root->children;
	}

	if (mode & SDM_DATA_WRITE) {
		if (!(data->buffer = xmlBufferCreate())
		    || !(data->writer = xmlNewTextWriterMemory(data->buffer, 0))
		    || (xmlTextWriterSetIndent(data->writer, 1) < 0)
		    || (xmlTextWriterStartDocument(data->writer, NULL, "UTF-8", NULL) < 0)
		    || (xmlTextWriterStartElement(data->writer, rootname) < 0)) {
			sdm_data_close(data);
			return(NULL);
		}
	}
	return(data);
}

void sdm_data_close(struct sdm_data_file *data)
{
	FILE *fptr;

	if (data->writer) {
		xmlTextWriterEndElement(data->writer);
		xmlTextWriterEndDocument(data->writer);
		xmlFreeTextWriter(data->writer);
	}
	if (data->buffer) {
		if ((fptr = fopen(data->filename, "w"))) {
			fputs(data->buffer->content, fptr);
			fclose(fptr);
		}
		xmlBufferFree(data->buffer);
	}
	if (data->filename)
		destroy_string(data->filename);
	if (data->doc)
		xmlFreeDoc(data->doc);
	memory_free(data);
}

int sdm_data_read_rewind(struct sdm_data_file *data)
{
	data->current = data->root->children;
	return(0);
}

int sdm_data_read_next(struct sdm_data_file *data)
{
	if (data->current && data->current->next) {
		data->current = data->current->next;
		return(1);
	}
	return(0);
}

int sdm_data_read_children(struct sdm_data_file *data)
{
	if (data->current && data->current->children) {
		data->current = data->current->children;
		return(1);
	}
	return(0);
}

int sdm_data_read_parent(struct sdm_data_file *data)
{
	if (data->current && data->current->parent && (data->current->parent != data->root)) {
		data->current = data->current->parent;
		return(1);
	}
	return(0);
}


const char *sdm_data_read_name(struct sdm_data_file *data)
{
	if (data->current)
		return(data->current->name);
	return(NULL);
}

int sdm_data_read_attrib(struct sdm_data_file *data, const char *name, char *buffer, int max)
{
	xmlChar *value;

	if (data->current && (value = xmlGetProp(data->current, name))) {
		strncpy(buffer, value, max);
		buffer[max - 1] = '\0';
		xmlFree(value);
		return(strlen(buffer));
	}
	buffer[0] = '\0';
	return(0);
}

double sdm_data_read_number(struct sdm_data_file *data)
{
	char *str;
	double num;

	if (data->current && (str = xmlNodeListGetString(data->doc, data->current->children, 1))) {
		num = atof(str);
		xmlFree(str);
		return(num);
	}
	return(0);
}

int sdm_data_read_string(struct sdm_data_file *data, char *buffer, int max)
{
	char *str;

	if (data->current && (str = xmlNodeListGetString(data->doc, data->current->children, 1))) {
		strncpy(buffer, (char *) str, max - 1);
		buffer[max - 1] = '\0';
		xmlFree(str);
		return(strlen(buffer));
	}
	return(0);
}



int sdm_data_write_begin_entry(struct sdm_data_file *data, const char *name)
{
	if (xmlTextWriterStartElement(data->writer, name) < 0)
		return(-1);
	return(0);
}

int sdm_data_write_attrib(struct sdm_data_file *data, const char *name, const char *value)
{
	if (xmlTextWriterWriteAttribute(data->writer, name, value) < 0)
		return(-1);
	return(0);
}

int sdm_data_write_number(struct sdm_data_file *data, double value)
{
	if (xmlTextWriterWriteFormatString(data->writer, "%f", value) < 0)
		return(-1);
	return(0);
}

int sdm_data_write_string(struct sdm_data_file *data, const char *value)
{
	if (xmlTextWriterWriteFormatString(data->writer, "%s", value) < 0)
		return(-1);
	return(0);
}

int sdm_data_write_end_entry(struct sdm_data_file *data)
{
	if (xmlTextWriterEndElement(data->writer) < 0)
		return(-1);
	return(0);
}

int sdm_data_write_current(struct sdm_data_file *data)
{
	int res;
	char *str;

	if (data->current && (str = xmlNodeListGetString(data->doc, data->current->children, 1))) {
		res = xmlTextWriterWriteString(data->writer, str);
		xmlFree(str);
		return(res);
	}
	return(-1);
}


int sdm_data_write_number_entry(struct sdm_data_file *data, const char *name, const char *value)
{
	if (xmlTextWriterWriteFormatElement(data->writer, name, "%f", value) < 0)
		return(-1);
	return(0);
}

int sdm_data_write_string_entry(struct sdm_data_file *data, const char *name, const char *value)
{
	if (xmlTextWriterWriteFormatElement(data->writer, name, "%s", value) < 0)
		return(-1);
	return(0);
}



