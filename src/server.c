#include <stdio.h>
#include "libipc.h"
#include "argv.h"
#include "trace.h"

#define OPT_HELP	"help"
#define OPT_SHELP	"h"
#define OPT_SVERSION	"v"
#define OPT_STHREADS	"t"
#define OPT_IFC		"ifc"
#define OPT_SIFC	"i"

#define VERSION		"1.0.0"

static _argv_t args[] = {
	{ OPT_SHELP,	0,				NULL,		"Print this help" },
	{ OPT_HELP,	OF_LONG,			NULL,		"Print this help" },
	{ OPT_SVERSION,	0,				NULL,		"Print version" },
	{ OPT_STHREADS,	0,				NULL,		"Use threads (process based by default)" },
	{ OPT_SIFC,	OF_VALUE,			NULL,		"Interface name (shared memory name or lan interface)" },
	{ OPT_IFC,	OF_LONG|OF_VALUE,		NULL,		"Interface name (shared memory name or lan interface)" },
	//...
	{ NULL,		0,				NULL,		NULL }
};

static void usage(void) {
	int n = 0;

	printf("options:\n");
	while (args[n].opt_name) {
		if (args[n].opt_flags & OF_LONG)
			printf("--%s:      \t%s\n", args[n].opt_name, args[n].opt_help);
		else
			printf("-%s:      \t%s\n", args[n].opt_name, args[n].opt_help);

		n++;
	}
	printf("\nUsage: server [options]\n");
}

static const char *opt_ifc(void) {
	const char *r = NULL;

	if (argv_check(OPT_IFC))
		r = argv_value(OPT_IFC);
	else if (argv_check(OPT_SIFC))
		r = argv_value(OPT_SIFC);

	return r;
}

int main(int argc, char *argv[]) {
	int r = 0;
	const char *ifc = NULL;

	if (argv_parse(argc, (_cstr_t *)argv, args)) {
		if (argv_check(OPT_SVERSION))
			printf("%s\n", VERSION);
		else if (argv_check(OPT_SHELP) || argv_check(OPT_HELP))
			usage();
		else {
			if ((ifc = opt_ifc())) {
				_ipc_t *s_ipc = ipc_server(ifc, IPC_MODE_SHM);

				if (s_ipc) {
					_ipc_t *c_ipc = ipc_listen(s_ipc);
					TRACE("server: incomming connection from '%s'\n", c_ipc->shm_name);

					char buf[256];

					int n = ipc_read(c_ipc, buf, sizeof(buf));
					printf(buf);
				}

				ipc_close(s_ipc);
			} else
				usage();
		}
	} else
		usage();


	return r;
}
