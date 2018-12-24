#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_



#define CFG_HOLDER		0x00FF55B1
#define CFG_LOCATION	0x79


typedef struct{
  uint32_t cfg_holder; //2
  char sta_ssid[32];   //32
  char sta_pwd[32];    //32

} SYSCFG;

bool ICACHE_FLASH_ATTR CFG_Save();
bool ICACHE_FLASH_ATTR CFG_Load();
bool ICACHE_FLASH_ATTR CFG_Reset();


extern SYSCFG sysCfg;

#endif /* USER_CONFIG_H_ */
