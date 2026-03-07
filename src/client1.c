#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "libipc.h"
#include "argv.h"
#include "trace.h"

#define OPT_HELP	"help"
#define OPT_SHELP	"h"
#define OPT_SVERSION	"v"
#define OPT_DST		"dst"

#define VERSION	"1.0.0"

static _ipc_t	*_g_ipc_ = NULL;
static int	_g_shm_fd_ = -1;

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

static void close_ipc(void) {
	if (_g_ipc_) {
		ipc_close(_g_ipc_, &_g_shm_fd_);
		_g_ipc_ = NULL;
	}
}

void sig_handler(int sig) {
	switch (sig) {
		case SIGINT:
		case SIGTERM:
		case SIGKILL:
			close_ipc();
			fflush(stderr);
			exit(0);
	}
}

static void remove_eol(char *str) {
	size_t l = strlen(str);

	while(str[l] < ' ' && l > 0) {
		str[l] = 0;
		l--;
	}
}

int main(int argc, char *argv[]) {
	int r = 0;

	int signals [] = { SIGINT, SIGTERM, SIGKILL, 0 };
	int n = 0;

	/* signal handling */
	while (signals[n]) {
		signal(signals[n], sig_handler);
		n++;
	}

	if (argv_parse(argc, (_cstr_t *)argv, args)) {
		if (argv_check(OPT_SVERSION))
			printf("%s\n", VERSION);
		if (argv_check(OPT_SHELP) || argv_check(OPT_HELP))
			usage();
		else {
			const char *dst = opt_dst();

			if (dst) {
				_g_ipc_ = ipc_client(dst, IPC_MODE_SHM, &_g_shm_fd_);

				if (_g_ipc_) {
					if (ipc_connect(_g_ipc_) == E_IPC_OK) {
						char buf[MAX_IO_BUFFER] = "";

						while (fgets(buf, sizeof(buf), stdin)) {
							remove_eol(buf);
							if (strncasecmp(buf, "exit", 4) == 0 ||
									strncasecmp(buf, "quit", 4) == 0)
								break;
							/* Send request */
							ipc_write(_g_ipc_, buf, strlen(buf));
							memset(buf, 0, sizeof(buf));

							/* Read response */
							int nc = ipc_read(_g_ipc_, buf, sizeof(buf));
							buf[nc] = 0;
							printf("%s", buf);
							memset(buf, 0, sizeof(buf));
						}
					} else {
						LOG("ERROR: No connection to '%s'\n", dst);
					}

					close_ipc();
				}
			}
		}
	} else
		usage();

	fflush(stderr);

	return r;
}
