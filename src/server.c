#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>
#include "libipc.h"
#include "argv.h"
#include "trace.h"

#define OPT_HELP	"help"
#define OPT_SHELP	"h"
#define OPT_SVERSION	"v"
#define OPT_STHREADS	"t"
#define OPT_IFC		"ifc"

#define VERSION		"1.0.0"

static _ipc_t		*_g_server_shm_ = NULL;
static int		_g_fd_shm_ = -1;
static const char	*_g_ifc_ = NULL;
static int		_g_running_ = 0;
static int		_g_fork_ = 0;

static _argv_t args[] = {
	{ OPT_SHELP,	0,				NULL,		"Print this help" },
	{ OPT_HELP,	OF_LONG,			NULL,		"Print this help" },
	{ OPT_SVERSION,	0,				NULL,		"Print version" },
	{ OPT_STHREADS,	0,				NULL,		"Use threads (process based by default)" },
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

	return r;
}

static void close_ipc(void) {
	if (_g_server_shm_) {
		ipc_close(_g_server_shm_, &_g_fd_shm_);
		_g_server_shm_ = NULL;
	}
}

typedef struct {
	pthread_t thread;
	int running;
	_ipc_t	*ipc_cxt;
	int fd;
} _udata_t;

static void *io_thread(void *arg) {
	_udata_t *ud = (_udata_t *)arg;
	char inp[MAX_IO_BUFFER];
	char out[MAX_IO_BUFFER];
	int n = 0;

	ud->running = 1;

	while (ud->running) {
		if ((n = ipc_read(ud->ipc_cxt, inp, sizeof(inp))) > 0) {
			int rsz = 0;

			memset(out, 0, sizeof(out));

			// call protocol here
			// ...
			rsz = snprintf(out, sizeof(out), ">> %s\n", inp);
			memset(inp, 0, sizeof(inp));
			usleep(10000);
			///////////////////

			ipc_write(ud->ipc_cxt, out, rsz);
		} else
			break;
	}

	ipc_unmap_shm(ud->ipc_cxt, &(ud->fd));
	free(ud);
	return NULL;
}

static int do_fork(_ipc_t *ipc_cxt, int fd) {
	int r = 0;
	int pid = fork();

	if (pid == 0) {
		/* child process */
		char inp[MAX_IO_BUFFER];
		char out[MAX_IO_BUFFER];
		int n = 0;

		/* we need to know about parent/child process */
		_g_fork_ = 1;

		/* _g_server_shm_ needed only in parent process (for listening) */
		ipc_unmap_shm(_g_server_shm_, &_g_fd_shm_); /* Unmap server area */
		_g_server_shm_ = NULL;

		while ((n = ipc_read(ipc_cxt, inp, sizeof(inp))) > 0) {
			int rsz = 0;

			memset(out, 0, sizeof(out));

			// call protocol here
			// ...
			rsz = snprintf(out, sizeof(out), ">> %s\n", inp);
			memset(inp, 0, sizeof(inp));
			usleep(10000);
			///////////////////

			ipc_write(ipc_cxt, out, rsz);
		}

		TRACE("fork %d exit\n", getpid());
		ipc_unmap_shm(ipc_cxt, &fd);
		exit(0);
	} else {
		/* parent process */
		ipc_unmap_shm(ipc_cxt, &fd); /* Unmap client */
	}

	return r;
}

void sig_handler(int sig) {
	switch (sig) {
		case SIGINT:
		case SIGTERM:
		case SIGKILL:
			_g_running_ = 0;
			close_ipc();
			break;
		case SIGSEGV:
			exit(-1);
		case SIGCHLD: {
				int stat;
				pid_t	pid;

				while (1) {
					if ((pid = wait3 (&stat, WNOHANG, (struct rusage *)NULL )) <= 0)
						break;
					TRACE("SIGCHLD: PID=%u, STATUS=%d\n", getpid(), pid, stat);
				}
			} break;

	}
}

int main(int argc, char *argv[]) {
	int r = 0;
	int signals [] = { SIGINT, SIGTERM, SIGKILL, SIGCHLD, SIGSEGV, 0 };
	int n = 0;

	/* signal handling */
	while (signals[n]) {
		signal(signals[n], sig_handler);
		n++;
	}

	if (argv_parse(argc, (_cstr_t *)argv, args)) {
		if (argv_check(OPT_SVERSION))
			printf("%s\n", VERSION);
		else if (argv_check(OPT_SHELP) || argv_check(OPT_HELP))
			usage();
		else {
			if ((_g_ifc_ = opt_ifc())) {
				_g_server_shm_ = ipc_server(_g_ifc_, IPC_MODE_SHM, &_g_fd_shm_);

				if (_g_server_shm_) {
					int use_threads = argv_check(OPT_STHREADS);

					_g_running_ = 1;

					while (_g_running_) {
						int cfd = -1;
						_ipc_t *c_ipc = ipc_listen(_g_server_shm_, &cfd);

						if (use_threads) {
							/* Switch to threads model */
							_udata_t *arg = (_udata_t *)malloc(sizeof(_udata_t));

							if (arg) {
								arg->running = 0;
								arg->ipc_cxt = c_ipc;
								arg->fd = cfd;

								pthread_create(&(arg->thread), NULL, io_thread, arg);
								pthread_detach(arg->thread);
								pthread_setname_np(arg->thread, (const char *)c_ipc->shm_name);
							}
						} else
							r = do_fork(c_ipc, cfd);
					}
				} else
					r = -1;

				if (!_g_fork_) {
					TRACE("server: shutdown\n");
					close_ipc();
				}
			} else
				usage();
		}
	} else
		usage();


	return r;
}
