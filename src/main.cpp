/*
 * Module Name:	main.c
 * Description:	Mud
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sdm/data.h>
#include <sdm/exception.h>
#include <sdm/code/code.h>
#include <sdm/drivers/driver.h>
#include <sdm/drivers/tcp.h>
#include <sdm/drivers/server.h>

#include <sdm/objs/boolean.h>
#include <sdm/objs/number.h>
#include <sdm/objs/string.h>
#include <sdm/objs/array.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/funcptr.h>

#include <sdm/lib/irc/pseudoserv.h>

static int exit_flag = 1;
static char loadfile[STRING_SIZE] = "";

int parse_args(int argc, char **argv);
int serverloop(void);
static void handle_sigint(int);

extern int moo_load_basic_funcs(MooObjectHash *env);
extern int moo_load_string_funcs(MooObjectHash *env);
extern int moo_load_thing_methods(MooObjectHash *env);
extern int moo_load_user_methods(MooObjectHash *env);
extern int moo_load_telnet_methods(MooObjectHash *env);

int init_moo(void)
{
	try {
		init_data();
		moo_set_data_path("data");

		init_object();
		init_mutable();

		/// Register Immutable Object Types
		moo_object_register_type(&moo_nil_obj_type);
		moo_object_register_type(&moo_boolean_obj_type);
		moo_object_register_type(&moo_number_obj_type);
		moo_object_register_type(&moo_string_obj_type);
		moo_object_register_type(&moo_func_ptr_obj_type);

		/// Initialize Mutable Object Types
		init_array();
		init_hash();

		init_moo_code();

		init_driver();

		init_tcp();
		init_server();
		init_thing();

		moo_load_basic_funcs(global_env);
		moo_load_string_funcs(global_env);
		moo_load_thing_methods(global_env);
		moo_load_user_methods(global_env);
		moo_load_telnet_methods(global_env);

		init_irc_pseudoserv();
	}
	catch (MooException e) {
		moo_status("%s", e.get());
		return(-1);
	}

	signal(SIGINT, handle_sigint);
	return(0);
}

void release_moo(void)
{
	release_irc_pseudoserv();

	release_thing();
	release_server();
	release_tcp();

	release_driver();

	release_moo_code();

	release_array();
	release_hash();

	release_mutable();
	release_object();

	release_data();
}

/**
 * Main Entry Point
 */
int main(int argc, char **argv)
{
	if (parse_args(argc, argv))
		return(-1);

	if (init_moo()) {
		printf("Failed to initialize moo\n");
		release_moo();
		return(0);
	}

	// TODO create a new task and execute (#0:boot)
	// TODO have #0:boot then call all this other shit.  It could be a compiled function and still call some other function
	// TODO #0:boot must create the listener task)
	//load_global_config();

	MooCodeFrame *frame;
	frame = new MooCodeFrame();
	frame->owner(0);
	if (loadfile[0] != '\0')
		frame->push_code(loadfile);
	//frame->push_code("(loop (debug \"Looping...\"))");
	frame->schedule(0);

	serverloop();

	release_moo();
	return(0);
}

int serverloop(void)
{
	while (exit_flag) {
		MooDriver::wait(1);
		MooCodeFrame::run_all();
	}
	return(0);
}

static void handle_sigint(int signum)
{
	printf("\nShutting down...\n");
	exit_flag = 0;
}


/*
 * Parse Command Line
 *
 * -r
 * --readonly
 *   The database is read only.  No modified data will be written to disc.
 *
 * -d <database>
 * --db <database>
 *   Specify the directory containing the object database.  Default: data/objs/ (???)
 *
 * -i
 * --interactive
 *   Interactive mode (commands can be entered like an interpreter)
 *
 * --bare
 *   Start with no objects.
 *
 * --bootstrap
 *   Bootstrap a basic database.
 *
 * -e <code>
 * --run <code>
 *   Run the moocode provided.  (Calling the save function from the code provided will write changes to disc).
 *
 * -l <file>
 * --load <file>
 *   Load and run the provided file.
 */
int parse_args(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--load")) {
			if (++i >= argc) {
				printf("No file name given to --load\n");
				return(-1);
			}
			snprintf(loadfile, STRING_SIZE, "(load \"%s\")", argv[i]);
		}
		else {
			printf("Unrecognized argument: %s\n", argv[i]);
			return(-1);
		}
	}
	return(0);
}


