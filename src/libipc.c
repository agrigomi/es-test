#include "libipc.h"

#if USE_SOCKETS
int ipc_get_fd(_ipc_cxt_t *cxt) {
	int r = -1;



	return r;
}
#endif

int ipc_open(const char *ifc, int mode, _ipc_cxt_t *cxt) {
	int r = E_FAIL;



	return r;
}

void ipc_close(_ipc_cxt_t *cxt) {

}

int ipc_listen(_ipc_cxt_t *cxt) {
	int r = E_FAIL;



	return r;
}

int ipc_connect(_ipc_cxt_t *cxt, const char *dst) {
	int r = E_FAIL;



	return r;
}

