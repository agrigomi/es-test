#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ipc_defs.h"
#include "libipc.h"
#include "trace.h"

#if USE_SHARED_MEMORY
/*
* Creates a shared memory context.
* ifc: Interface, SHM name, IP or lan interface (depends on selected mode
* pfd: Pointer to file descriptor
* return pointer to IPC context
*/
static _ipc_t *open_shared_memory(const char *ifc, int *pfd) {
	_ipc_t *r = NULL;
	int fd = shm_open(ifc, O_CREAT | O_EXCL | O_RDWR, 0600);

	if (fd > 0) {
		if (pfd) /* backup file descriptor */
			*pfd = fd;

		if (ftruncate(fd, sizeof(_ipc_t)) != -1) {
			if ((r = mmap(NULL, sizeof(_ipc_t), PROT_READ | PROT_WRITE,
						MAP_SHARED, fd, 0))) {
				if (sem_init(&(r->s_data), 1, 0) == 0 &&
						sem_init(&(r->s_ready), 1, 0) == 0) {
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

/*
 * Server side: Initialize and create an IPC context.
 *
 * ifc: Interface name or path to bind to.
 * mode: The IPC mode (SHM, INET, or UNIX).
 * pfd: Pointer to an integer where the file descriptor will be stored.
 * return _ipc_t* Pointer to the allocated context, or NULL on failure.
 */
_ipc_t *ipc_server(const char *ifc, int mode, int *pfd) {
	_ipc_t *r = NULL;

	switch (mode) {
		case IPC_MODE_SHM:
#if USE_SHARED_MEMORY
			/* Open server side shared area for connect only */
			r = open_shared_memory(ifc, pfd);
#endif
			break;
		case IPC_MODE_INET:
			break;
		case IPC_MODE_UNIX:
			break;
	}

	return r;
}

/*
 * Client side: Initialize an IPC context for a client.
 *
 * dst: Destination address or resource name.
 * mode: The IPC mode to use pfd Pointer to store the client file descriptor.
 * return _ipc_t* Pointer to the client context, or NULL on failure.
 */
_ipc_t *ipc_client(const char *dst, int mode, int *pfd) {
	_ipc_t *r = NULL;
	char ifc[MAX_SHM_NAME];
	static int counter = 0;

	memset(ifc, 0, sizeof(ifc));

	switch (mode) {
		case IPC_MODE_SHM:
#if USE_SHARED_MEMORY
			/* Create uniqie name for client shared memory */
			counter++;
			snprintf(ifc, sizeof(ifc), "c%dp%d", counter, getpid());

			/* Open client side shared area for data transfer */
			if ((r = open_shared_memory(ifc, pfd))) {
				/* Set destination (server SHM) in own IO buffer */
				strncpy((char *)r->io_buffer, dst, sizeof(r->io_buffer) - 1);
				r->size = strlen(dst);
			}
#endif
			break;
		case IPC_MODE_INET:
			break;
		case IPC_MODE_UNIX:
			break;
	}

	return r;
}

/* Sync FS (because it's not a realtime) */
static void ipc_sync(void) {
	fflush(NULL);
	usleep(10);
}

/*
 * Close the IPC connection and clean up resources.
 *
 * cxt: The IPC context to close.
 * pfd: Pointer to the file descriptor to be closed.
 */
void ipc_close(_ipc_t *cxt, int *pfd) {
	if (cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		TRACE("libipc: Close SHM '%s'\n", cxt->shm_name);
		cxt->size = 0;
		sem_post(&(cxt->s_data));
		sem_destroy(&(cxt->s_data));
		sem_post(&(cxt->s_ready));
		sem_destroy(&(cxt->s_ready));
		shm_unlink(cxt->shm_name);
		ipc_sync();
#endif
	}

	if (pfd)
		close(*pfd);
}

#if USE_SHARED_MEMORY
 /*  Unmap shared memory segments (SHM mode specific).
 *
 *  cxt: The IPC context.
 *  pfd: Pointer to the associated file descriptor.
 */
void ipc_unmap_shm(_ipc_t *cxt, int *pfd) {
	munmap(cxt, sizeof(_ipc_t));
	if (pfd)
		close(*pfd);
}
#endif

/*  Server side: Listen for incoming client connections.
 *
 * server_cxt: The server context created by ipc_server.
 * pfd: Pointer to store the connection file descriptor.
 * return _ipc_t* Connection-specific IPC context, or NULL on failure.
 */
_ipc_t *ipc_listen(_ipc_t *server_cxt, int *pfd) {
	_ipc_t *r = NULL;

	if (server_cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		/* Clear server IO area */
		server_cxt->size = 0;
		memset(server_cxt->io_buffer, 0, sizeof(server_cxt->io_buffer));

		/* Starts listening for incoming connection */
		if (sem_wait(&(server_cxt->s_data)) == 0) {
			/* The IO buffer is expected to contain the name of the client's shared area */
			int fd = shm_open((char *)server_cxt->io_buffer, O_RDWR, 0);

			if (fd > 0) {
				if (pfd) /* backup file descriptor */
					*pfd = fd;

				if ((r = mmap(NULL, sizeof(_ipc_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0))) {
					/* Send ready signal */
					sem_post(&(server_cxt->s_ready));
					ipc_sync();
				} else {
					TRACE("libipc: Unable to map client's shared area '%s'\n", server_cxt->io_buffer);
				}
			} else {
				TRACE("libipc: Failed to open client's shared area '%s'\n", server_cxt->io_buffer);
			}
		}
#endif
	} else if (server_cxt->mode == IPC_MODE_INET) {
#if USE_SOCKETS
		/* ToDo INET */
#endif
	} else if (server_cxt->mode == IPC_MODE_UNIX) {
#if USE_UNIX_SOCKETS
		/* ToDo UNIX sockets */
#endif
	}

	return r;
}

/*
 * Client side: Establish connection to the server.
 *
 * client_cxt: The client context to connect.
 * return int E_IPC_OK on success, E_IPC_FAIL on failure.
 */
int ipc_connect(_ipc_t *client_cxt) {
	int r = E_IPC_FAIL;

	if (client_cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		/* According the expectations, IO buffer should contains the name of server shared area */
		int fd = shm_open((char *)client_cxt->io_buffer, O_RDWR, 0);

		if (fd > 0) {
			_ipc_t *server_cxt = mmap(NULL, sizeof(_ipc_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

			if (server_cxt) {
				/* Copy client SHM name to server IO buffer */
				strncpy((char *)server_cxt->io_buffer, client_cxt->shm_name,
						sizeof(server_cxt->io_buffer));

				/* send connection request */
				if (sem_post(&(server_cxt->s_data)) == 0) {
					ipc_sync();

					/* Waiting for ready signal */
					if (sem_wait(&(server_cxt->s_ready)) == 0) {
						r = E_IPC_OK;
						TRACE("libipc: Established connection to server '%s'\n", server_cxt->shm_name);
					} else {
						TRACE("libipc: Unable to connect '%s'\n", server_cxt->shm_name);
					}
				}

				munmap(server_cxt, sizeof(_ipc_t));
			}

			close(fd);
		} else {
			TRACE("libipc: Failed to open server's shared area '%s'\n", client_cxt->io_buffer);
		}
#endif
	} else if (client_cxt->mode == IPC_MODE_INET) {
#if USE_SOCKETS
		/* ToDo INET */
#endif
	} else if (client_cxt->mode == IPC_MODE_UNIX) {
#if USE_UNIX_SOCKETS
		/* ToDo UNIX sockets */
#endif
	}

	return r;
}

/*
 * Write data to the IPC channel.
 *
 * cxt: The active IPC context.
 * data: Pointer to the source data buffer.
 * size: Number of bytes to write.
 * return int Number of bytes written, or E_IPC_FAIL on error.
 */
int ipc_write(_ipc_t *cxt, void *data, int size) {
	int r = 0;
	unsigned int n = size;

	if(cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		if (n > sizeof(cxt->io_buffer))
			n = sizeof(cxt->io_buffer);

		/* copy user data to IO buffer */
		memcpy(cxt->io_buffer, data, n);
		cxt->size = n;

		/* send data */
		if (sem_post(&(cxt->s_data)) == 0) {
			r = n;

			/* sync (ugly but needed) */
			ipc_sync();

			/* waiting for ready signal */
			sem_wait(&(cxt->s_ready));
		} else
			r = E_IPC_FAIL;
#endif
	}

	return r;
}

/*
 * Read data from the IPC channel.
 *
 * cxt: The active IPC context.
 * buffer: Pointer to the destination buffer.
 * size: Maximum number of bytes to read.
 * return int Number of bytes read, or E_IPC_FAIL on error.
 */
int ipc_read(_ipc_t *cxt, void *buffer, int size) {
	int r = 0;
	unsigned int n = size;

	if(cxt->mode == IPC_MODE_SHM) {
#if USE_SHARED_MEMORY
		if (sem_wait(&(cxt->s_data)) == 0) {
			if (n > cxt->size)
				n = cxt->size;

			/* copy IO data to user */
			memcpy(buffer, cxt->io_buffer, n);

			/* clear IO data */
			memset(cxt->io_buffer, 0, n);
			r = n;
			cxt->size = 0;

			/* send ready signal */
			sem_post(&(cxt->s_ready));

			/* sync (ugly but needed) */
			ipc_sync();
		} else {
			TRACE("libipc: Failed to read\n");
			r = E_IPC_FAIL;
		}
#endif
	}

	return r;
}

