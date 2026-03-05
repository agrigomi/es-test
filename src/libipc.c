#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "libipc.h"

#if USE_SOCKETS
int ipc_get_fd(_ipc_cxt_t *cxt) {
	int r = -1;



	return r;
}
#endif

#if USE_SHARED_MEMORY
_ipc_cxt_t *open_shared_memory(const char *ifc) {
	_ipc_cxt_t *r = NULL;


	return r;
}
#endif

_ipc_cxt_t *ipc_server(const char *ifc, int mode) {
	_ipc_cxt_t *r = NULL;

	switch (mode) {
		case IPC_MODE_SHM:
			/* Open server side shared area for connect only */
			r = open_shared_memory(ifc);
			break;
		case IPC_MODE_INET:
			break;
		case IPC_MODE_UNIX:
			break;
	}

	return r;
}

_ipc_cxt_t *ipc_client(const char *dst, int mode) {
	_ipc_cxt_t *r = NULL;
	char ifc[MAX_SHM_NAME];

	memset(ifc, 0, sizeof(ifc));

	switch (mode) {
		case IPC_MODE_SHM:
			/* Create uniqie name for client shared memory */
			snprintf(ifc, sizeof(ifc), "SMC%d", getpid());
			/* Open client side shared area for data transfer */
			r = open_shared_memory(ifc);
			break;
		case IPC_MODE_INET:
			break;
		case IPC_MODE_UNIX:
			break;
	}

	return r;
}

void ipc_close(_ipc_cxt_t *cxt) {

}

_ipc_cxt_t *ipc_listen(_ipc_cxt_t *server_cxt) {
	 _ipc_cxt_t *r = NULL;



	return r;
}

int ipc_connect(_ipc_cxt_t *cxt) {
	int r = E_IPC_FAIL;



	return r;
}

