#ifndef __LIBIPC_H__
#define __LIBIPC_H__

#include "ipc_defs.h"

#if USE_SHARED_MEMORY
#include <semaphore.h>

#define MAX_SHM_NAME	16
#endif


#define IPC_MODE_SHM		1	/* Shared memory mode */
#define IPC_MODE_INET		2	/* Internet sockets */
#define IPC_MODE_UNIX		3	/* Local sockets */

/* Error defs */
#define E_IPC_OK		0
#define E_IPC_FAIL		-1

/* IPC context */
typedef struct {
	int		mode;		/* IPC mode */
#if USE_SHARED_MEMORY
	sem_t		s_data;		/* request semaphore */
	sem_t		s_result;	/* result semaphore */
	char		shm_name[MAX_SHM_NAME]; /* shared memory name */
	unsigned int	size; 		/* data size */
	unsigned char	io_buffer[MAX_IO_BUFFER];
#endif
} _ipc_t;

#ifdef __cplusplus
extern "C" {
#endif

#if USE_SOCKETS
int ipc_get_fd(_ipc_t *cxt);
#endif

/* Server side.
 * Create IPC context */
_ipc_t *ipc_server(const char *ifc, int mode);
/* Server side listen for client connections.
 * Returns connection IPC context */
_ipc_t *ipc_listen(_ipc_t *server_cxt);

/* Client side.
 * Returns connection IPC context */
_ipc_t *ipc_client(const char *dst, int mode);

/* Client side.
   Connect to server. */
int ipc_connect(_ipc_t *client_cxt);

int ipc_write(_ipc_t *cxt, void *data, int size);
int ipc_read(_ipc_t *cxt, void *buffer, int size);

void ipc_close(_ipc_t *cxt);
void ipc_unmap(_ipc_t *cxt);

#ifdef __cplusplus
}
#endif

#endif /* __LIBIPC_H__ */
