#ifndef __IPC_DEFS_H__
#define __IPC_DEFS_H__

#define IPC_VERSION		"1.0.0"

#define USE_SHARED_MEMORY	1
#define USE_SOCKETS		0
#define USE_UNIX_SOCKETS	0

#if USE_SHARED_MEMORY
#define MAX_IO_BUFFER		1024
#endif

typedef struct _ipc_cxt_t	_ipc_t;

#endif
