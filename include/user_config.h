#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define USE_OPTIMIZE_PRINTF


#define STATUS_ENDPOINT "http://iih.daimler.com/v1/status/simple"

#define ERROR_LEVEL
#define WARN_LEVEL
#define INFO_LEVEL
#define DEBUG_LEVEL


#define APP_NAME        "Ampel"
#define APP_VER_MAJ		1
#define APP_VER_MIN		1
#define APP_VER_REV		1


#ifdef ERROR_LEVEL
#define ERROR( format, ... ) os_printf( "[ERROR] " format, ## __VA_ARGS__ )
#else
#define ERROR( format, ... )
#endif

#ifdef WARN_LEVEL
#define WARN( format, ... ) os_printf( "[WARN] " format, ## __VA_ARGS__ )
#else
#define WARN( format, ... )
#endif

#ifdef INFO_LEVEL

#define INFO( format, ... ) os_printf( "[INFO] " format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

#ifdef DEBUG_LEVEL
#define MQTT_DEBUG_ON 1
#define DEBUG( format, ... ) os_printf( "[DEBUG] " format, ## __VA_ARGS__ )
#else
#define DEBUG( format, ... )
#endif
#endif

