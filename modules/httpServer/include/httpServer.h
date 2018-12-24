#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_

#include <user_interface.h>
#include <espconn.h>


#define PORT 80
#define SERVER_TIMEOUT 1500
#define MAX_CONNS 1

void ICACHE_FLASH_ATTR httpServer_init(espconn_recv_callback cb);

#endif /* UDP_SERVER_H_ */
