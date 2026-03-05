#ifndef __LIBIPC_H__
#define __LIBIPC_H__

#include "ipc_defs.h"

#if USE_SHARED_MEMORY
#include <semaphore.h>

#define MAX_SHM_NAME	16
#endif


#define IPC_MODE_SHM		1
#define IPC_MODE_INET		2
#define IPC_MODE_UNIX		3

/* Error defs */
#define E_OK		0
#define E_FAIL		-1

/* IPC context */
typedef struct {
	int	mode;	/* IPC mode */
#if USE_SHARED_MEMORY
	sem_t		s_data;		/* request semaphore */
	sem_t		s_result;	/* result semaphore */
	char		shm_name[MAX_SHM_NAME]; /* shared memory name */
	unsigned int	size; /* data size */
	unsigned char	io_buffer[MAX_IO_BUFFER];
#endif
} _ipc_cxt_t;

#ifdef __cplusplus
extern "C" {
#endif
#if USE_SOCKETS
int ipc_get_fd(_ipc_cxt_t *cxt);
#endif

int ipc_open(const char *ifc, int mode, _ipc_cxt_t *cxt);
void ipc_close(_ipc_cxt_t *cxt);
int ipc_listen(_ipc_cxt_t *cxt);
int ipc_connect(_ipc_cxt_t *cxt, const char *dst);

#ifdef __cplusplus
}
#endif

#endif /* __LIBIPC_H__ */
