#include <stdio.h>
#include <unistd.h>
#include "libipc.h"
#include "argv.h"
#include "trace.h"

#define OPT_HELP	"help"
#define OPT_SHELP	"h"
#define OPT_SVERSION	"v"
#define OPT_DST		"dst"

#define VERSION	"1.0.0"

static _argv_t args[] = {
	{ OPT_SHELP,	0,				NULL,		"Print this help" },
	{ OPT_HELP,	OF_LONG,			NULL,		"Print this help" },
	{ OPT_SVERSION,	0,				NULL,		"Print version" },
	{ OPT_DST,	OF_LONG|OF_VALUE,		NULL,		"Destination address (shared memory name or IP)" },
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
	printf("\nUsage: hl [options]\n");
}

static const char *opt_dst(void) {
	const char *r = NULL;

	if (argv_check(OPT_DST))
		r = argv_value(OPT_DST);

	return r;
}

int main(int argc, char *argv[]) {
	int r = 0;

	if (argv_parse(argc, (_cstr_t *)argv, args)) {
		if (argv_check(OPT_SVERSION))
			printf("%s\n", VERSION);
		if (argv_check(OPT_SHELP) || argv_check(OPT_HELP))
			usage();
		else {
			const char *dst = opt_dst();

			if (dst) {
				int fd = -1;
				_ipc_t *c_ipc = ipc_client(dst, IPC_MODE_SHM, &fd);

				if (c_ipc)
					ipc_connect(c_ipc);

				ipc_write(c_ipc, "alabala", 7);
				ipc_close(c_ipc, &fd);
			}
		}
	} else
		usage();


	return r;
}
