/*
 * Object Name:	mutable.cpp
 * Description:	Mutable Object
 */

#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/objs/mutable.h>


#define ID_TABLE_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE
#define ID_INIT_SIZE		100
/// This is to prevent us from making giant table accidentally
#define ID_MAX_SIZE		65536

MooArray<MooMutable *> *moo_id_table = NULL;

int init_mutable(void)
{
	if (moo_id_table)
		return(1);
	moo_id_table = new MooArray<MooMutable *>(ID_INIT_SIZE, ID_MAX_SIZE, ID_TABLE_BITS);
	return(0);
}

void release_mutable(void)
{
	if (moo_id_table)
		delete moo_id_table;
}

/*************************
 * MooMutable Definition *
 *************************/

MooMutable::MooMutable()
{
	m_id = -1;
}

MooMutable::MooMutable(moo_id_t id)
{
	// TODO temporary since we haven't properly dealt with refcounting/thingrefs
	this->set_nofree();

	m_id = -1;
	this->assign_id(id);
}

MooMutable::~MooMutable()
{
	if (m_id >= 0)
		moo_id_table->set(m_id, NULL);
}

int MooMutable::is_wizard(moo_id_t id)
{
	MooMutable *obj;

	if (!(obj = moo_id_table->get(id)))
		return(0);
	return(obj->is_wizard());
}


MooObject *MooMutable::lookup(moo_id_t id)
{
	MooObject *obj;
	char buffer[STRING_SIZE];

	if (id < 0)
		return(NULL);
	if (!(obj = moo_id_table->get(id))) {
		/// We check for the file so that we don't print a 'failed to open file' error
		snprintf(buffer, STRING_SIZE, "objs/%04d.xml", id);
		if (!moo_data_file_exists(buffer))
			return(NULL);
		obj = MooMutable::load_object(id);
	}
	return(obj);
}

MooObject *MooMutable::reference(const char *name)
{
	moo_id_t id;

	if (name[0] == '#') {
		id = ::atoi(&name[1]);
		return(MooMutable::lookup(id));
	}
	// TODO should we just get rid of this entirely?
	else if (name[0] == '$') {
		// TODO should we break up the reference??
		//MooObject *ref = global_env->get(&name[1]);
		// TODO I don't think this is right; we should be using #0 anyways
		//if (!ref)
		//	return(NULL);
		//return(ref->get_thing());
	}
	return(NULL);
}

MooObject *MooMutable::load_object(moo_id_t id)
{
	MooObject *obj;
	MooDataFile *data;
	char type[STRING_SIZE];
	char filename[STRING_SIZE];

	if (id < 0) {
		moo_status("MUTABLE: attempted to load invalid ID, %d", id);
		return(NULL);
	}
	snprintf(filename, STRING_SIZE, "objs/%04d.xml", id);

	try {
		data = new MooDataFile(filename, MOO_DATA_READ);
		data->root_name(type, STRING_SIZE);
		moo_status("MUTABLE: Loading object %d of %s type from file \"%s\".", id, type, filename);
		obj = MooObject::read_object(data, type);
		delete data;
		return(obj);
	}
	catch (MooException e) {
		moo_status("DATA: %s", e.get());
		return(NULL);
	}
}

int MooMutable::save_object()
{
	MooDataFile *data;
	char file[STRING_SIZE];
	const MooObjectType *objtype;

	/// If we are currently writing the file to disc then don't write it again
	if (m_bitflags & MOO_BF_WRITING)
		return(0);

	if (m_id < 0) {
		moo_status("MUTABLE: attempted to save object with invalid ID, %d", m_id);
		return(-1);
	}

	if (!(objtype = this->objtype())) {
		moo_status("MUTABLE: Object %d has no known type.", m_id);
		return(-1);
	}
	//snprintf(file, STRING_SIZE, "objs/%04d/%04d.xml", m_id / 10000, m_id % 10000);
	snprintf(file, STRING_SIZE, "objs/%04d.xml", m_id);
	moo_status("MUTABLE: Saving object %d to file \"%s\".", m_id, file);
	data = new MooDataFile(file, MOO_DATA_WRITE, objtype->m_name);

	m_bitflags |= MOO_BF_WRITING;
	try {
		this->write_object(data);
	}
	catch (MooException e) {
		m_bitflags &= ~MOO_BF_WRITING;
		throw e;
	}
	m_bitflags &= ~MOO_BF_WRITING;

	delete data;
	return(0);
}

int MooMutable::save_all()
{
	MooMutable *obj;

	for (int i = 0; i <= moo_id_table->last(); i++) {
		if ((obj = moo_id_table->get(i)))
			obj->save_object();
	}
	return(0);
}


int MooMutable::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "bits")) {
		int bits = data->read_integer_entry();
		m_bitflags |= (bits & MOO_BF_WIZARD) ? MOO_BF_WIZARD : 0;
	}
	else if (!strcmp(type, "id")) {
		moo_id_t id = data->read_integer_entry();
		this->assign_id(id);
	}
	else
		return(MooObject::read_entry(type, data));
	return(MOO_HANDLED);
}

int MooMutable::write_data(MooDataFile *data)
{
	if (m_id >= 0)
		data->write_integer(m_id);
	if (m_bitflags & MOO_BF_WIZARD)
		data->write_hex_entry("bits", MOO_BF_WIZARD);
	return(0);
}

int MooMutable::write_object(MooDataFile *data)
{
	data->write_integer_entry("id", m_id);
	return(0);
}

int MooMutable::is_assigned(moo_id_t id)
{
	if (moo_id_table->get(id))
		return(1);
	return(0);
}

int MooMutable::assign_id(moo_id_t id)
{
	/// If the thing already has an ID, then remove it from the table
	if (this->m_id >= 0)
		moo_id_table->set(m_id, NULL);

	/// Assign the thing to the appropriate index in the table and set the ID if it succeeded
	m_id = -1;
	if (id == MOO_NEW_ID) {
		// TODO this will load every object in to the system, we need no change this if we want to only load the needed objs
		id = moo_id_table->next_space();
		for (; id < ID_MAX_SIZE; id++) {
			if (!MooMutable::lookup(id))
				break;
		}
	}

	if (id >= ID_MAX_SIZE)
		throw MooException("MUTABLE: Maximum of %d objects reached; assignment failed.", ID_MAX_SIZE);
	if (moo_id_table->set(id, this))
		m_id = id;
	if (m_id < 0 && id != MOO_NO_ID)
		moo_status("MUTABLE: Attempted to reassign ID, %d", id);
	return(m_id);
}


