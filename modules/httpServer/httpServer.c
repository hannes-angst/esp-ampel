#include <c_types.h>
#include <mem.h>
#include <user_interface.h>
#include <ets_sys.h>
#include <osapi.h>
#include <espconn.h>

#include "user_config.h"
#include "httpServer.h"

espconn_recv_callback reqCb = NULL;

static struct espconn *pHttpServer;

void ICACHE_FLASH_ATTR httpServer_init(espconn_recv_callback cb) {
	reqCb = cb;
	int wifiMode = wifi_get_opmode();

	wifi_set_opmode(2);

	pHttpServer = (struct espconn *) os_zalloc(sizeof(struct espconn));
	ets_memset(pHttpServer, 0, sizeof(struct espconn));
	espconn_create(pHttpServer);
	pHttpServer->type = ESPCONN_TCP;
	pHttpServer->proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	pHttpServer->proto.tcp->local_port = PORT;
	espconn_regist_recvcb(pHttpServer, reqCb);
	espconn_tcp_set_max_con_allow(pHttpServer, MAX_CONNS);

	/*	wifi_station_dhcpc_start();
	 */
	if (espconn_accept(pHttpServer)) {
		while (1) {
			ERROR("Error creating TCP Server.\r\n");
		}
	}
}
