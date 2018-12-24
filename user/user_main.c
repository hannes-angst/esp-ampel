#include <user_interface.h>
#include <ets_sys.h>
#include <osapi.h>
#include <mem.h>
#include <gpio.h>

#include "user_config.h"
#include "wifi.h"
#include "info.h"
#include "app_config.h"
#include "httpServer.h"


static os_timer_t wait_timer;

#define INDEX_PAGE 		"GET /"
#define INDEX_CONTENT 	"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"description\" content=\"Wifi Configration\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Wifi Configration</title></head><body><h1>Wifi Configration</h1><form action=\"/set\" method=\"post\"><fieldset><legend>Please provide your SSID and Password (PSK)</legend><form method=\"post\" action=\"/set\"><p><label>SSID</label><br><input type=\"text\" name=\"ssid\"></p><p><label>Password</label><br><input type=\"password\" name=\"psk\"></p><button>Save</button></body></html>"

#define SET_PAGE 		"POST /set"
#define SET_CONTENT 	"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"description\" content=\"Wifi Configration\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Wifi Configration</title></head><body><h1>Wifi Configration</h1><p>All set. Device is going to reboot now.</p></body></html>"


#define RED_API 		"GET /ampel/red"
#define RED_LED 		6
#define RED_PIN 		GPIO_ID_PIN(13)
#define RED_MUX 		PERIPHS_IO_MUX_MTCK_U
#define RED_FUN 		FUNC_GPIO13

#define YELLOW_API 		"GET /ampel/yellow"
#define YELLOW_LED 		5
#define YELLOW_PIN 		GPIO_ID_PIN(12)
#define YELLOW_MUX 		PERIPHS_IO_MUX_MTDI_U
#define YELLOW_FUN		FUNC_GPIO12

#define GREEN_API 		"GET /ampel/green"
#define GREEN_LED 		3
#define GREEN_PIN 		GPIO_ID_PIN(14)
#define GREEN_MUX 		PERIPHS_IO_MUX_MTMS_U
#define GREEN_FUN		FUNC_GPIO14

#define API_OFF 		"GET /ampel/off"
#define LED_OFF 		7

#define API_ON 			"GET /ampel/on"
#define LED_ON 			0


static void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		INFO("Connected to AP.\r\n");
	} else if (status == STATION_WRONG_PASSWORD || status == STATION_NO_AP_FOUND || status == STATION_CONNECT_FAIL) {
		INFO("Error connecting to AP.\r\n");
	} else {
		INFO("Connecting to AP.\r\n");
	}
}

//Timer event.
static void ICACHE_FLASH_ATTR reboot(void *arg) {
	os_timer_disarm(&wait_timer);
	INFO("Reboot\r\n");
	system_restart();
}


/******************************************************************************
 * FunctionName : data_send
 * Description  : processing the data as http format and send to the client or server
 * Parameters   : arg -- argument to set for client or server
 *                responseOK -- true or false
 *                psend -- The send data
 * Returns      :
 *******************************************************************************/
static void ICACHE_FLASH_ATTR data_send(void *arg, bool responseOK, char *psend) {
	uint16 length = 0;
	char *pbuf = NULL;
	char httphead[256];
	struct espconn *ptrespconn = arg;
	os_memset(httphead, 0, 256);

	if (responseOK) {
		os_sprintf(httphead, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\nServer: IntelliHub/1.0.0\r\n", psend ? os_strlen(psend) : 0);
		if (psend) {
			os_sprintf(httphead + os_strlen(httphead), "Content-type: text/html\r\nPragma: no-cache\r\n\r\n");
			length = os_strlen(httphead) + os_strlen(psend);
			pbuf = (char *) os_zalloc(length + 1);
			os_memcpy(pbuf, httphead, os_strlen(httphead));
			os_memcpy(pbuf + os_strlen(httphead), psend, os_strlen(psend));
		} else {
			os_sprintf(httphead + os_strlen(httphead), "\r\n");
			length = os_strlen(httphead);
		}
	} else {
		os_sprintf(httphead, "HTTP/1.0 400 BadRequest\r\nContent-Length: 0\r\nServer: IntelliHub/1.0.0\r\n\r\n");
		length = os_strlen(httphead);
	}
	if (psend) {
		espconn_sent(ptrespconn, pbuf, length);
	} else {
		espconn_sent(ptrespconn, httphead, length);
	}

	if (pbuf) {
		os_free(pbuf);
		pbuf = NULL;
	}
}

static void ICACHE_FLASH_ATTR status_send(void *arg, bool responseOK) {
	data_send(arg, responseOK, NULL);
}


static void ICACHE_FLASH_ATTR set_led(uint8_t cmd) {
	INFO("[%s] Red (PIN %d)\r\n",    ((cmd & 1) == 0)?"x":" ", RED_PIN);
	INFO("[%s] Yellow (PIN %d)\r\n", ((cmd & 2) == 0)?"x":" ", YELLOW_PIN);
	INFO("[%s] Green (PIN %d)\r\n",  ((cmd & 4) == 0)?"x":" ", GREEN_PIN);

	uint8_t bit = cmd & 1;

	INFO("RED = %d\r\n", bit);
	GPIO_OUTPUT_SET(RED_PIN, bit);

	cmd = cmd >> 1;
	bit = cmd & 1;

	INFO("YELLOW = %d\r\n", bit);
	GPIO_OUTPUT_SET(YELLOW_PIN, bit);

	cmd = cmd >> 1;
	bit = cmd & 1;

	INFO("GREEN = %d\r\n", bit);
	GPIO_OUTPUT_SET(GREEN_PIN, bit);
}


//Called when new packet comes in.
static void ICACHE_FLASH_ATTR httpserver_recv(void *arg, char *pusrdata, uint16 len) {
	INFO("Received data.\r\n");
	INFO("%s\r\n", pusrdata);

	if(len >= os_strlen(RED_API) && strncmp((const char *) pusrdata, RED_API, strlen(RED_API)) == 0) {
		INFO("Set red color.\r\n");
		set_led(RED_LED);
		data_send(arg, true, NULL);
	} else if(len >= os_strlen(YELLOW_API) && strncmp((const char *) pusrdata, YELLOW_API, strlen(YELLOW_API)) == 0) {
		INFO("Set yellow color.\r\n");
		set_led(YELLOW_LED);
		data_send(arg, true, NULL);
	} else if(len >= os_strlen(GREEN_API) && strncmp((const char *) pusrdata, GREEN_API, strlen(GREEN_API)) == 0) {
		INFO("Set green color.\r\n");
		set_led(GREEN_LED);
		data_send(arg, true, NULL);
	} else if(len >= os_strlen(API_OFF) && strncmp((const char *) pusrdata, API_OFF, strlen(API_OFF)) == 0) {
		INFO("Set no color.\r\n");
		set_led(LED_OFF);
		data_send(arg, true, NULL);
	} else if(len >= os_strlen(API_ON) && strncmp((const char *) pusrdata, API_ON, strlen(API_ON)) == 0) {
			INFO("Set all colors.\r\n");
			set_led(LED_ON);
			data_send(arg, true, NULL);
	} else 	if (len >= os_strlen(INDEX_PAGE) && strncmp((const char *) pusrdata, INDEX_PAGE, strlen(INDEX_PAGE)) == 0) {
		INFO("Send index page.\r\n");
		data_send(arg, true, INDEX_CONTENT);
	} else if (len >= os_strlen(SET_PAGE) && strncmp((const char *) pusrdata, SET_PAGE, strlen(SET_PAGE)) == 0) {
		INFO("Set wifi credentials.\r\n");
		char *ssid = (char*) os_zalloc(32);
		char *psk = (char*) os_zalloc(32);

		uint16 pos;
		uint16 p_start = 4;
		uint16 v_start = 0;
		uint8 type = 0;

		const char *payload = os_strstr(pusrdata, "\r\n\r\n");
		uint16 pLen = os_strlen(payload);
		DEBUG("Payload: %s\r\n", &(payload[4]));
		DEBUG("Setting p_start: %d\r\n", p_start);

		for (pos = 4; pos < pLen; pos++) {
			if (payload[pos] == '=') {
				v_start = pos + 1;
				DEBUG("Setting v_start: %d\r\n", v_start);
				DEBUG("start: %s\r\n", &(payload[p_start]));
				if (os_strncmp((const char *) &(payload[p_start]), "ssid", os_strlen("ssid")) == 0) {
					type = 1;
					DEBUG("Setting param type to SSID.\r\n");
				} else if (os_strncmp((const char *) &(payload[p_start]), "psk", os_strlen("psk")) == 0) {
					type = 2;
					DEBUG("Setting param type to PSK.\r\n");
				} else {
					WARN("Invalid data received.\r\n");
					status_send(arg, false);
					return;
				}
				p_start = 0;
			} else if (payload[pos] == '&') {
				p_start = pos + 1;
				DEBUG("start: %s\r\n", &(payload[v_start]));
				DEBUG("Setting p_start: %d\r\n", p_start);
				if (type == 1) {
					//read ssid
					if (pos - v_start >= 32) {
						WARN("SSID value exceeds boundaries.\r\n");
						status_send(arg, false);
						return;
					}
					os_strncpy(ssid, (const char * )&(payload[v_start]), pos - v_start);
					ssid[31] = '\0';
					DEBUG("SSID: %s\r\n", ssid);
				} else if (type == 2) {
					//read psk
					if (pos - v_start >= 32) {
						WARN("SSID value exceeds boundaries.\r\n");
						status_send(arg, false);
						return;
					}
					os_strncpy(psk, (const char * )&(payload[v_start]), pos - v_start);
					psk[31] = '\0';
					DEBUG("PSK: %s\r\n", psk);
				} else {
					WARN("Invalid data received.\r\n");
					status_send(arg, false);
					return;
				}
				v_start = 0;
			}
			pos++;
		}
		if (v_start != 0) {
			if (type == 1) {
				//read ssid
				if (pos - v_start >= 32) {
					WARN("SSID value exceeds boundaries.\r\n");
					status_send(arg, false);
					return;
				}
				os_strncpy(ssid, (const char * )&(payload[v_start]), pos - v_start);
				ssid[31] = '\0';
				INFO("SSID: %s\r\n", ssid);
			} else if (type == 2) {
				//read psk
				if (pos - v_start >= 32) {
					WARN("SSID value exceeds boundaries.\r\n");
					status_send(arg, false);
					return;
				}
				os_strncpy(psk, (const char * )&(payload[v_start]), pos - v_start);
				psk[31] = '\0';
				INFO("PSK: %s\r\n", psk);
			} else {
				WARN("Invalid data received.\r\n");
				status_send(arg, false);
				return;
			}
		}

		INFO("Got ssid: %s\r\n", ssid);
		INFO("Got psk: %s\r\n", psk);

		os_memset(&sysCfg, 0x00, sizeof(sysCfg));
		sysCfg.cfg_holder = CFG_HOLDER;
		os_strcpy(sysCfg.sta_ssid, ssid);
		os_strcpy(sysCfg.sta_pwd, psk);

		INFO("ssid to save: %s\r\n", sysCfg.sta_ssid);
		INFO("psk to save:  %s\r\n", sysCfg.sta_pwd);

		//send page
		data_send(arg, true, SET_CONTENT);

		bool saved = CFG_Save();

		os_free(ssid);
		os_free(psk);


		os_timer_setfn(&wait_timer, (os_timer_func_t *) reboot, NULL);
		os_timer_disarm(&wait_timer);
		os_timer_arm(&wait_timer, 3000, 0);
	} else {
		status_send(arg, false);
	}
}

static void ICACHE_FLASH_ATTR app_init(void) {
	httpServer_init(httpserver_recv);
}

static void ICACHE_FLASH_ATTR led_init(void) {
	PIN_FUNC_SELECT(RED_MUX, RED_FUN);
	PIN_FUNC_SELECT(YELLOW_MUX, YELLOW_FUN);
	PIN_FUNC_SELECT(GREEN_MUX, GREEN_FUN);


}

void user_init(void) {
	print_info();
	led_init();

	if (CFG_Load()) {
		INFO("Config loaded.\r\n");
		WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
	} else {
		INFO("No config found.\r\n");
		WIFI_AP();
	}
	system_init_done_cb(app_init);
}
