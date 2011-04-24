/*
 * Module Name:	data.c
 * Description:	Data Reader/Writer
 */

#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include <sdm/data.h>

#define IS_WHITESPACE(ch)	\
	( ((ch) == ' ') || ((ch) == '\t') || ((ch) == '\n') || ((ch) == '\r') )

static char *data_path = NULL;

static int moo_data_strip_copy(char *, const char *, int);

int init_data(void)
{
	LIBXML_TEST_VERSION
	return(0);
}

void release_data(void)
{
	xmlCleanupParser();
}

static char *make_string(char *fmt, ...)
{
	char *str;
	va_list va;
	char buffer[STRING_SIZE];

	va_start(va, fmt);
	vsnprintf(buffer, STRING_SIZE, fmt, va);
	if (!(str = (char *) malloc(strlen(buffer) + 1)))
		return(NULL);
	strcpy(str, buffer);
	return(str);
}

int moo_set_data_path(const char *path)
{
	if (data_path)
		free(data_path);
	if (!(data_path = make_string("%s/", path)))
		return(-1);
	return(0);
}

int moo_data_file_exists(const char *file)
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

int moo_data_read_file(const char *file, char *buffer, int max)
{
	FILE *fptr;
	int count;
	char name[STRING_SIZE];

	if (!data_path)
		return(0);
	snprintf(name, STRING_SIZE, "%s%s", data_path, file);
	if (!(fptr = fopen(name, "r")))
		return(0);
	count = fread(buffer, 1, max, fptr);
	buffer[count] = '\0';
	fclose(fptr);
	return(count);
}


MooDataFile::MooDataFile(const char *file, int mode, const char *rootname)
{
	this->filename = NULL;
	this->doc = NULL;
	this->root = NULL;
	this->current = NULL;
	this->writer = NULL;
	this->buffer = NULL;

	if (!data_path)
		throw MooException("No data path");
	if (!(this->filename = make_string("%s%s", data_path, file)))
		throw moo_mem_error;

	if (mode & MOO_DATA_READ) {
		if (!(this->doc = xmlReadFile(this->filename, NULL, 0))
		    || !(this->root = xmlDocGetRootElement(this->doc)) || xmlStrcmp(this->root->name, (xmlChar *) rootname)) {
			throw MooException("Error opening XML file for reading");
		}
		this->current = this->root->children;
	}

	if (mode & MOO_DATA_WRITE) {
		if (!(this->buffer = xmlBufferCreate())
		    || !(this->writer = xmlNewTextWriterMemory(this->buffer, 0))
		    || (xmlTextWriterSetIndent(this->writer, 1) < 0)
		    || (xmlTextWriterStartDocument(this->writer, NULL, "UTF-8", NULL) < 0)
		    || (xmlTextWriterStartElement(this->writer, (xmlChar *) rootname) < 0)) {
			throw MooException("Error opening XML file for writing");
		}
	}
}

MooDataFile::~MooDataFile()
{
	FILE *fptr;

	if (this->writer) {
		xmlTextWriterEndElement(this->writer);
		xmlTextWriterEndDocument(this->writer);
		xmlFreeTextWriter(this->writer);
	}
	if (this->buffer) {
		if ((fptr = fopen(this->filename, "w"))) {
			fputs((char *) this->buffer->content, fptr);
			fclose(fptr);
		}
		xmlBufferFree(this->buffer);
	}
	if (this->filename)
		free(this->filename);
	if (this->doc)
		xmlFreeDoc(this->doc);
}

int MooDataFile::read_rewind()
{
	this->current = this->root->children;
	return(0);
}

int MooDataFile::read_next()
{
	if (this->current && this->current->next) {
		this->current = this->current->next;
		return(1);
	}
	return(0);
}

int MooDataFile::read_children()
{
	if (this->current && this->current->children) {
		this->current = this->current->children;
		return(1);
	}
	return(0);
}

int MooDataFile::read_parent()
{
	if (this->current && this->current->parent && (this->current->parent != this->root)) {
		this->current = this->current->parent;
		return(1);
	}
	return(0);
}


const char *MooDataFile::read_name()
{
	if (this->current) {
		if (this->current->name)
			return((const char *) this->current->name);
		else if (this->current->type == XML_CDATA_SECTION_NODE)
			return("cdata");
	}
	return(NULL);
}

long int MooDataFile::read_attrib_integer(const char *name)
{
	xmlChar *value;
	long int num;

	if (this->current && (value = xmlGetProp(this->current, (xmlChar *) name))) {
		num = strtol((char *) value, NULL, 0);
		xmlFree(value);
		return(num);
	}
	return(0);
}

double MooDataFile::read_attrib_float(const char *name)
{
	xmlChar *value;
	double num;

	if (this->current && (value = xmlGetProp(this->current, (xmlChar *) name))) {
		num = strtod((char *) value, NULL);
		xmlFree(value);
		return(num);
	}
	return(0);
}

int MooDataFile::read_attrib_string(const char *name, char *buffer, int max)
{
	xmlChar *value;

	if (this->current && (value = xmlGetProp(this->current, (xmlChar *) name))) {
		strncpy(buffer, (char *) value, max);
		buffer[max - 1] = '\0';
		xmlFree(value);
		return(strlen(buffer));
	}
	buffer[0] = '\0';
	return(0);
}

long int MooDataFile::read_integer()
{
	char *str;
	long int num;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current, 1))) {
		num = strtol(str, NULL, 0);
		xmlFree(str);
		return(num);
	}
	return(0);
}

double MooDataFile::read_float()
{
	char *str;
	double num;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current, 1))) {
		num = strtod(str, NULL);
		xmlFree(str);
		return(num);
	}
	return(0);
}

int MooDataFile::read_string(char *buffer, int max)
{
	int size;
	char *str;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current, 1))) {
		size = moo_data_strip_copy(buffer, str, max);
		xmlFree(str);
		return(size);
	}
	buffer[0] = '\0';
	return(0);
}

int MooDataFile::read_raw_string(char *buffer, int max)
{
	char *str;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current, 1))) {
		strncpy(buffer, str, max);
		xmlFree(str);
		buffer[max - 1] = '\0';
		return(strlen(buffer));
	}
	buffer[0] = '\0';
	return(0);
}


long int MooDataFile::read_integer_entry()
{
	char *str;
	long int num;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current->children, 1))) {
		num = strtol(str, NULL, 0);
		xmlFree(str);
		return(num);
	}
	return(0);
}

double MooDataFile::read_float_entry()
{
	char *str;
	double num;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current->children, 1))) {
		num = strtod(str, NULL);
		xmlFree(str);
		return(num);
	}
	return(0);
}

int MooDataFile::read_string_entry(char *buffer, int max)
{
	int size;
	char *str;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current->children, 1))) {
		size = moo_data_strip_copy(buffer, str, max);
		xmlFree(str);
		return(size);
	}
	buffer[0] = '\0';
	return(0);
}

int MooDataFile::read_raw_string_entry(char *buffer, int max)
{
	char *str;

	if (this->current && (str = (char *) xmlNodeListGetString(this->doc, this->current->children, 1))) {
		strncpy(buffer, str, max);
		xmlFree(str);
		buffer[max - 1] = '\0';
		return(strlen(buffer));
	}
	buffer[0] = '\0';
	return(0);
}


int MooDataFile::write_begin_entry(const char *name)
{
	if (xmlTextWriterStartElement(this->writer, (xmlChar *) name) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_attrib_integer(const char *name, long int value)
{
	if (xmlTextWriterWriteFormatAttribute(this->writer, (xmlChar *) name, "%ld", value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_attrib_float(const char *name, double value)
{
	if (xmlTextWriterWriteFormatAttribute(this->writer, (xmlChar *) name, "%f", value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_attrib_string(const char *name, const char *value)
{
	if (xmlTextWriterWriteAttribute(this->writer, (xmlChar *) name, (xmlChar *) value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_integer(long int value)
{
	if (xmlTextWriterWriteFormatString(this->writer, "%ld", value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_float(double value)
{
	if (xmlTextWriterWriteFormatString(this->writer, "%f", value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_string(const char *value)
{
	if (xmlTextWriterWriteFormatString(this->writer, "%s", value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_raw_string(const char *value)
{
	if (xmlTextWriterWriteCDATA(this->writer, (xmlChar *) value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_end_entry()
{
	if (xmlTextWriterEndElement(this->writer) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_current()
{
	int res = -1;
	xmlBufferPtr buffer;

	if (!this->current || !(buffer = xmlBufferCreate()))
		return(-1);
	if (xmlNodeDump(buffer, this->doc, this->current, 2, 1) >= 0) {
		// TODO this doesn't really format the data correctly but it at least works
		xmlBufferAdd((xmlBuffer *) buffer, (xmlChar *) "\n", -1);
		res = xmlTextWriterWriteRaw(this->writer, buffer->content);
	}
	xmlBufferFree(buffer);
	return(res);
}


int MooDataFile::write_integer_entry(const char *name, long int value)
{
	if (xmlTextWriterWriteFormatElement(this->writer, (xmlChar *) name, "%ld", (xmlChar *) value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_octal_entry(const char *name, long int value)
{
	if (xmlTextWriterWriteFormatElement(this->writer, (xmlChar *) name, "%#lo", (xmlChar *) value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_hex_entry(const char *name, long int value)
{
	if (xmlTextWriterWriteFormatElement(this->writer, (xmlChar *) name, "%#lx", (xmlChar *) value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_float_entry(const char *name, double value)
{
	if (xmlTextWriterWriteFormatElement(this->writer, (xmlChar *) name, "%f", value) < 0)
		return(-1);
	return(0);
}

int MooDataFile::write_string_entry(const char *name, const char *value)
{
	if (xmlTextWriterWriteFormatElement(this->writer, (xmlChar *) name, "%s", (xmlChar *) value) < 0)
		return(-1);
	return(0);
}


/*** Local Functions ***/

static int moo_data_strip_copy(char *dest, const char *src, int max)
{
	int i, j;

	max--;		/** Reserve space for the \0 character */
	for (i = 0, j = 0; IS_WHITESPACE(src[i]) && (src[i] != '\0'); i++)
		;
	for (; (j < max) && (src[i] != '\0'); i++, j++) {
		if (IS_WHITESPACE(src[i])) {
			for (; (j < max) && IS_WHITESPACE(src[i]) && (src[i] != '\0'); i++)
				;
			dest[j] = ' ';
			if (src[i] == '\0')
				break;
			else if (j < max - 1)
				dest[++j] = src[i];
		}
		else
			dest[j] = src[i];
	}
	dest[j] = '\0';
	return(j);
}


