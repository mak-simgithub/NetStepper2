# NetStepper2
A networked stepper motor controller based on the ESP-WROOM-32 and the L6470. Examples built with netStepper can be seen in this [video](https://vimeo.com/278343342).  
![Board](https://github.com/IAD-ZHDK/NetStepper2/raw/master/pcb/net-stepper2.png)
## Parts

### Controllers

- Stepper Controller: `L6470` ([Datasheet](https://www.mouser.com/ds/2/389/l6470-954753.pdf), [Mouser](https://www.mouser.ch/ProductDetail/STMicroelectronics/L6470H?qs=sGAEpiMZZMuP%2fQeRSdvksPGSQkvtk%2fGb))
- SoC: `ESP-WROOM-32` ([Datasheet](https://espressif.com/sites/default/files/documentation/esp-wroom-32_datasheet_en.pdf), [Mouser](https://www.mouser.ch/ProductDetail/Adafruit/3320?qs=%2fha2pyFaduhvAZY8Ie1SD0odCwfSxZwX5aiqddL%252ba6k%3d))
- USB/UART: `CP2104` ([Datasheet](https://www.mouser.com/ds/2/368/cp2104-37496.pdf), [Mouser](https://www.mouser.ch/ProductDetail/Silicon-Labs/CP2104-F03-GMR?qs=sGAEpiMZZMtXqW1IUNX6MBk8lKhQ2c4Y))

### Interface

- RGB LED: `CLMVB-FKA-CFHEHLCBB7a363` ([Datasheet](https://www.mouser.com/ds/2/90/CLMVBFKA-276021.pdf), [Mouser](https://www.mouser.ch/ProductDetail/Cree-Inc/CLMVB-FKA-CFHEHLCBB7a363/?qs=sGAEpiMZZMvyj6n1w4pZD44mowbLOn3YXn9F0G7z9j4=))
- Micro USB: `ZX62-B-5PA(33)` ([Datasheet](https://www.mouser.com/ds/2/185/ZX_catalog-939768.pdf), [Mouser](https://www.mouser.ch/ProductDetail/Hirose-Connector/ZX62-B-5PA33?qs=%2fha2pyFadujrkQEnlOn9YJamDi2lLfztUsBlF%252bMpnrr%2ffu%252bTTtxTbg%3d%3d))

### Other

- NPN Transistor: `MMBT2222` ([Datasheet](https://www.mouser.com/ds/2/308/MMBT2222LT1-D-80103.pdf), [Mouser](https://www.mouser.ch/ProductDetail/863-SMMBT2222ALT3G?r=863-SMMBT2222ALT3G))
- Double Diodes: `BAV99` ([Datasheet](https://www.mouser.com/ds/2/308/BAV99-1118535.pdf), [Mouser](https://www.mouser.ch/ProductDetail/ON-Semiconductor-Fairchild/BAV99?qs=sGAEpiMZZMudZehw8RjeZe6BnmRwGdH1))
- Traco: `TSR 1-2433`: ([Datasheet](https://www.mouser.com/ds/2/687/tsr1-537631.pdf), [Mouser](https://www.mouser.ch/ProductDetail/TRACO-Power/TSR-1-2433?qs=sGAEpiMZZMsF1ODjcwEocIuESINJH25XntreftV5zWI%3d))
- Button: `B3AL1002P-MS`: ([Datasheet](https://www.mouser.com/ds/2/307/en-b3al-6968.pdf), [Mouser](https://www.mouser.ch/ProductDetail/Omron-Electronics/B3AL-1002P?qs=sGAEpiMZZMsgGjVA3toVBH4vymNSXBJAvFITsJt6vAk%3d))
- Power Jack: `PTH-Lock`: ([Datasheet](https://www.sparkfun.com/datasheets/Prototyping/Barrel-Connector-PJ-202A.pdf), [Mouser](https://www.mouser.ch/ProductDetail/SparkFun/PRT-00119?qs=%2fha2pyFaduhW%2fk%2fYYulWfhGNKo7ZHzyg2PgAdV2a06daJ1r7ZioXdg%3d%3d))
- Tantalum Capacitor: `10uF`: ([Datasheet](https://www.mouser.com/ds/2/40/f93-776559.pdf), [Mouser](https://www.mouser.ch/ProductDetail/AVX/F931D106MAA?qs=%2fha2pyFadugBNWncAbXQXxEHQJISJiFBJ7QXp8FDj7ScO5vdUpgUZA%3d%3d))
- Tantalum Capacitor: `47uF`: ([Datasheet](https://www.mouser.com/ds/2/40/f93-776559.pdf), [Mouser](https://www.mouser.ch/ProductDetail/AVX/F931A476KBA?qs=sGAEpiMZZMuEN2agSAc2puC4lhRhLeolV5NgMLhJRWs%3d))
- Electrolyte Capacitor: `100uF`: ([Datasheet](https://www.mouser.com/ds/2/88/SML%20series-552997.pdf), [Mouser](https://www.mouser.ch/ProductDetail/Illinois-Capacitor-CDE/107SML063M?qs=%2fha2pyFaduhe2UJZSX2WvbxZYvhC4L3TD5ZVG2tL5rqP%252bovMAc8Hgw%3d%3d))

## Notes

- ESP32 from Adafruit do not come in a tape/reel.

## Usage
- install [NAOS](https://github.com/256dpi/naos) on your machine
- power and connect NetStepper2 via USB to your machine
- try to connect to NAOS with `naos-explorer`. if it doesn't work, the NAOS version on the NetStepper 2 is to old and you have to update it (see below)
- with `naos attach` you can read diagnostics of your NAOS over USB
- configure NAOS in `naos-explorer`
    - `device-name`: no relevance
    - `base-topic`: base topic to adress specific device `<MQTT_BASETOPIC>`
    - `wifi-ssid`: SSID of WiFi network
    - `wifi-password`: password of WiFi network
    - `mqtt-host`: hostname or IP of MQTT broker `<MQTT_HOST>`
    - `mqtt-port`: MQTT port `<MQTT_PORT>`
    - `mqtt-client-id`: no relevance
    - `mqtt-username`: MQTT username `<MQTT_USER>`
    - `mqtt-password`: MQTT password `<MQTT_PWD>`
    - `mqtt-configure`: send action after setting above values
- if diagnostics look ok, you can start to publish and subscribe to topics

### MQTT Topics

publish to topics over mqtt `mosquitto_pub -h <MQTT_HOST> -p <MQTT_PORT> -u <MQTT_USER> -P <MQTT_PWD> -t <MQTT_BASETOPIC>/<MQTT_TOPIC> -n`

- `forward`: move forward
- `backward`: move backward
- `stop`: stop
- `target <TARGET>`: move to position `<TARGET>`
- `reset`: set home
- `home`: move home

subscribe to topics with `mosquitto_sub -h <MQTT_HOST> -p <MQTT_PORT> -u <MQTT_USER> -P <MQTT_PWD> -t <MQTT_BASETOPIC>/# -v`

- `position`: position of stepper motor
- `speed`: speed of stepper motor
- `running`: true if stepper motor is running
- `#`: all topics


### Update
- install [NAOS](https://github.com/256dpi/naos) on your machine
- clone this repo
- power and connect NetStepper2 via USB to your machine
- `naos install`
- `naos build --reconfigure`
- `naos run`
- if `naos-explorer` can connect, you were succesful
