# Add WiFi Capabilities to a LED trafic sign 

got a kind of [gift](https://www.amazon.com/Traffic-Light-Lamp-Discontinued-Manufacturer/dp/B0001X0DTO/ref=pd_lpo_sbs_60_t_2?_encoding=UTF8&psc=1&refRID=BS7SSESZWZY2FP7PSNWA) and decided to make a WiFi upgrade, using the ESP8266 chip.

Along the line of making the build, I decided to replace the provided LED PCB and replaced it with four LEDs in Red, Amber and White (for green).

*TODO*
- Exmplain the build

## Pictures
*todo*


# BOM
- The used PCB can be found at [EasyEAD](https://easyeda.com/hannes-angst/ESP12F-Programming-board). 
- AI-Thinker ESP8266 ESP-12F

*todo*
- BOM of PCB (-> EasyEDA)
- BOM of Project
- Lipo link

#Programming
Checkout the [SDK](https://github.com/pfalcon/esp-open-sdk).

*TODO* 
- FTDI
- Eclipse link


#Next steps
- Add MQTT client
- Integrate push button from hardware
- Implement transitions, for example from red to green: (1) red, (2) yellow+red, (3) green
- Integrate a propper OFF switch for conversing battery
- Find a new supplyer for the trafic sign hardware. The one referenced has discontinued the product