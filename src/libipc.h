/**
 * @file libipc.h
 * @brief Inter-Process Communication (IPC) Library.
 * @details This library provides an interface for IPC using Shared Memory (SHM),
 * Internet sockets (INET), and Local Unix sockets.
 *
 */

#ifndef __LIBIPC_H__
#define __LIBIPC_H__

#include <semaphore.h>

/** @name Configuration Constants */
/** @{ */
#define MAX_SHM_NAME            32   /**< Maximum length for shared memory object names */
#define MAX_IO_BUFFER           1024 /**< Maximum size of the I/O buffer */
/** @} */

/** @name IPC Modes */
/** @{ */
#define IPC_MODE_SHM            1    /**< Mode: Shared memory */
#define IPC_MODE_INET           2    /**< Mode: Internet sockets (TCP/IP) */
#define IPC_MODE_UNIX           3    /**< Mode: Local Unix domain sockets */
/** @} */

/** @name Error Codes */
/** @{ */
#define E_IPC_OK                0    /**< Operation successful */
#define E_IPC_FAIL              -1   /**< Operation failed */
/** @} */

/**
 * @struct _ipc_t
 * @brief IPC context structure.
 * @note In future implementations (INET and UNIX), this structure
 *       should be abstracted behind a void pointer to ensure encapsulation.
 */
typedef struct {
        int             mode;                   /**< Selected IPC mode */
        sem_t           s_data;                 /**< Semaphore for signaling requests */
        sem_t           s_ready;                /**< Semaphore for signaling results/readiness */
        char            shm_name[MAX_SHM_NAME]; /**< Shared memory segment name */
        unsigned int    size;                   /**< Current data size in buffer */
        unsigned char   io_buffer[MAX_IO_BUFFER]; /**< Internal data buffer */
        /* ... */
} _ipc_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Server side: Initialize and create an IPC context.
 *
 * @param ifc Interface name or path to bind to.
 * @param mode The IPC mode (SHM, INET, or UNIX).
 * @param pfd Pointer to an integer where the file descriptor will be stored.
 * @return _ipc_t* Pointer to the allocated context, or NULL on failure.
 */
_ipc_t *ipc_server(const char *ifc, int mode, int *pfd);

/**
 * @brief Server side: Listen for incoming client connections.
 *
 * @param server_cxt The server context created by ipc_server.
 * @param pfd Pointer to store the connection file descriptor.
 * @return _ipc_t* Connection-specific IPC context, or NULL on failure.
 */
_ipc_t *ipc_listen(_ipc_t *server_cxt, int *pfd);

/**
 * @brief Client side: Initialize an IPC context for a client.
 *
 * @param dst Destination address or resource name.
 * @param mode The IPC mode to use.
 * @param pfd Pointer to store the client file descriptor.
 * @return _ipc_t* Pointer to the client context, or NULL on failure.
 */
_ipc_t *ipc_client(const char *dst, int mode, int *pfd);

/**
 * @brief Client side: Establish connection to the server.
 *
 * @param client_cxt The client context to connect.
 * @return int E_IPC_OK on success, E_IPC_FAIL on failure.
 */
int ipc_connect(_ipc_t *client_cxt);

/**
 * @brief Write data to the IPC channel.
 *
 * @param cxt The active IPC context.
 * @param data Pointer to the source data buffer.
 * @param size Number of bytes to write.
 * @return int Number of bytes written, or E_IPC_FAIL on error.
 */
int ipc_write(_ipc_t *cxt, void *data, int size);

/**
 * @brief Read data from the IPC channel.
 *
 * @param cxt The active IPC context.
 * @param buffer Pointer to the destination buffer.
 * @param size Maximum number of bytes to read.
 * @return int Number of bytes read, or E_IPC_FAIL on error.
 */
int ipc_read(_ipc_t *cxt, void *buffer, int size);

/**
 * @brief Close the IPC connection and clean up resources.
 *
 * @param cxt The IPC context to close.
 * @param pfd Pointer to the file descriptor to be closed.
 */
void ipc_close(_ipc_t *cxt, int *pfd);

/**
 * @brief Unmap shared memory segments (SHM mode specific).
 *
 * @param cxt The IPC context.
 * @param pfd Pointer to the associated file descriptor.
 */
void ipc_unmap_shm(_ipc_t *cxt, int *pfd);

#ifdef __cplusplus
}
#endif

#endif /* __LIBIPC_H__ */
