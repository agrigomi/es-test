#ifndef __LIBIPC_H__
#define __LIBIPC_H__


#include <semaphore.h>

#define MAX_SHM_NAME		32
#define MAX_IO_BUFFER		1024


#define IPC_MODE_SHM		1	/* Shared memory mode */
#define IPC_MODE_INET		2	/* Internet sockets */
#define IPC_MODE_UNIX		3	/* Local sockets */

/* Error defs */
#define E_IPC_OK		0
#define E_IPC_FAIL		-1

/* IPC context */
typedef struct {
	int		mode;		/* IPC mode */
	sem_t		s_data;		/* request semaphore */
	sem_t		s_ready;	/* result semaphore */
	char		shm_name[MAX_SHM_NAME]; /* shared memory name */
	unsigned int	size; 		/* data size */
	unsigned char	io_buffer[MAX_IO_BUFFER];
	/* ... */
} _ipc_t;

#ifdef __cplusplus
extern "C" {
#endif


/* Server side.
 * Create IPC context */
_ipc_t *ipc_server(const char *ifc, int mode, int *pfd);
/* Server side listen for client connections.
 * Returns connection IPC context */
_ipc_t *ipc_listen(_ipc_t *server_cxt, int *pfd);

/* Client side.
 * Returns connection IPC context */
_ipc_t *ipc_client(const char *dst, int mode, int *pfd);

/* Client side.
   Connect to server. */
int ipc_connect(_ipc_t *client_cxt);

int ipc_write(_ipc_t *cxt, void *data, int size);
int ipc_read(_ipc_t *cxt, void *buffer, int size);

void ipc_close(_ipc_t *cxt, int *pfd);
void ipc_unmap_shm(_ipc_t *cxt, int *pfd);

#ifdef __cplusplus
}
#endif

#endif /* __LIBIPC_H__ */
