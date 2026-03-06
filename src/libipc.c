#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libipc.h"
#include "trace.h"

#if USE_SHARED_MEMORY
_ipc_t *open_shared_memory(const char *ifc) {
	_ipc_t *r = NULL;
	int fd = shm_open(ifc, O_CREAT | O_EXCL | O_RDWR, 0600);

	if (fd > 0) {
		if (ftruncate(fd, sizeof(_ipc_t)) != -1) {
			if ((r = mmap(NULL, sizeof(_ipc_t), PROT_READ | PROT_WRITE,
						MAP_SHARED, fd, 0))) {
				if (sem_init(&(r->s_data), 1, 0) == 0 &&
						sem_init(&(r->s_result), 1, 0) == 0) {
					strncpy(r->shm_name, ifc, sizeof(r->shm_name) - 1);
					r->mode = IPC_MODE_SHM;
				} else {
					TRACE("libips: Failed to init semaphore\n");
					shm_unlink(ifc);
					r = NULL;
				}
			} else {
				TRACE("libipc: Failed to create memory mapping\n");
				shm_unlink(ifc);
			}
		} else {
			TRACE("libipc: Failed to resize mapping file\n");
			shm_unlink(ifc);
		}
	} else {
		TRACE("libipc: Failed to open mapping file '%s' #%d\n", ifc, fd);
	}

	return r;
}
#endif

_ipc_t *ipc_server(const char *ifc, int mode) {
	_ipc_t *r = NULL;

	switch (mode) {
#if USE_SHARED_MEMORY
		case IPC_MODE_SHM:
			/* Open server side shared area for connect only */
			r = open_shared_memory(ifc);
			break;
#endif
		case IPC_MODE_INET:
			break;
		case IPC_MODE_UNIX:
			break;
	}

	return r;
}

_ipc_t *ipc_client(const char *dst, int mode) {
	_ipc_t *r = NULL;
	char ifc[MAX_SHM_NAME];
	int sz = 0;

	memset(ifc, 0, sizeof(ifc));

	switch (mode) {
#if USE_SHARED_MEMORY
		case IPC_MODE_SHM:
			/* Create uniqie name for client shared memory */
			sz = snprintf(ifc, sizeof(ifc), "SMC%d", getpid());
			/* Open client side shared area for data transfer */
			if ((r = open_shared_memory(ifc))) {
				/* Set destination (server SHM) in own IO buffer */
				strncpy((char *)r->io_buffer, dst, sizeof(r->io_buffer) - 1);
				r->size = sz;
			}
			break;
#endif
		case IPC_MODE_INET:
			break;
		case IPC_MODE_UNIX:
			break;
	}

	return r;
}

void ipc_close(_ipc_t *cxt) {
	shm_unlink(cxt->shm_name);
}

void ipc_unmap(_ipc_t *cxt) {
	munmap(cxt, sizeof(_ipc_t));
}

_ipc_t *ipc_listen(_ipc_t *server_cxt) {
	_ipc_t *r = NULL;

	if (server_cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		if (sem_wait(&(server_cxt->s_data)) == 0) {
			/* The IO buffer is expected to contain the name of the client's shared area */
			int fd = shm_open((char *)server_cxt->io_buffer, O_RDWR, 0);

			if (fd > 0) {
				if ((r = mmap(NULL, sizeof(_ipc_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)))
					sem_post(&(server_cxt->s_result));
				else {
					TRACE("libipc: Unable to map client's shared area '%s'\n", server_cxt->io_buffer);
				}
			} else {
				TRACE("libipc: Failed to open client's shared area\n");
			}
		}
#endif
	}


	return r;
}

int ipc_connect(_ipc_t *client_cxt) {
	int r = E_IPC_FAIL;

	if (client_cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		/* According the expectations, IO buffer should contains the name of server shared area */
		int fd = shm_open((char *)client_cxt->io_buffer, O_RDWR, 0);

		if (fd > 0) {
			_ipc_t *server_cxt = mmap(NULL, sizeof(_ipc_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

			if (server_cxt) {
				strncpy((char *)server_cxt->io_buffer, client_cxt->shm_name,
						sizeof(server_cxt->io_buffer));
				sem_post(&(server_cxt->s_data));
				if (sem_wait(&(server_cxt->s_result)) == 0) {
					r = E_IPC_OK;
					TRACE("libipc: Established connection to server '%s'\n", server_cxt->shm_name);
				} else {
					TRACE("libipc: Unable to connect '%s'\n", server_cxt->shm_name);
				}

				munmap(server_cxt, sizeof(_ipc_t));
			}

			close(fd);
		} else {
			TRACE("libipc: Failed to open server's shared area '%s'\n", client_cxt->io_buffer);
		}
#endif
	}

	return r;
}

int ipc_write(_ipc_t *cxt, void *data, int size) {
	int r = 0;
	unsigned int n = size;

	if(cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		if (n > sizeof(cxt->io_buffer))
			n = sizeof(cxt->io_buffer);

		memcpy(cxt->io_buffer, data, n);
		cxt->size = n;
		if (sem_post(&(cxt->s_data)) == 0)
			r = n;
#endif
	}

	return r;
}

int ipc_read(_ipc_t *cxt, void *buffer, int size) {
	int r = 0;
	unsigned int n = size;

	if(cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		if (sem_wait(&(cxt->s_data)) == 0) {
			if (n > cxt->size)
				n = cxt->size;

			memcpy(buffer, cxt->io_buffer, n);
			r = n;
		} else {
			TRACE("libipc: Failed to read\n");
		}
#endif
	}

	return r;
}

