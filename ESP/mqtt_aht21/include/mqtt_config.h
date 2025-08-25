//RFID
#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__


typedef enum{
  NO_TLS = 0,                       // 0: disable SSL/TLS, there must be no certificate verify between MQTT server and ESP8266
  TLS_WITHOUT_AUTHENTICATION = 1,   // 1: enable SSL/TLS, but there is no a certificate verify
  ONE_WAY_ANTHENTICATION = 2,       // 2: enable SSL/TLS, ESP8266 would verify the SSL server certificate at the same time
  TWO_WAY_ANTHENTICATION = 3,       // 3: enable SSL/TLS, ESP8266 would verify the SSL server certificate and SSL server would verify ESP8266 certificate
}TLS_LEVEL;

#define TCP_SERVER_LOCAL_PORT   23 //telnet setup port
#define HTTP_SERVER_LOCAL_PORT  80 //web setup port

/*IMPORTANT: the following configuration maybe need modified*/
/***********************************************************************************************************************/

/*DEFAULT CONFIGURATIONS*/

//#define IIC //Use serial EEPROM
#define _LAB_CONFIG_


#ifdef _LAB_CONFIG_
#define CFG_HOLDER          0x02FF55A8  /* Change this value to load default configurations */
#define MQTT_HOST           "dorlov.no-ip.com"
#define MQTT_PORT            1883 //no SSL: 19001, with SSL: 39001, 39002 = 2048 bit, 29001 - Web
#define MQTT_TOPIC_BASE     "ORLOV"
#define MQTT_TOPIC_TYPE     "HUMT"
#define MQTT_USER           "dimorlus"
#define MQTT_PASS           "dorlov"
#define STA_SSID            "HONOR"
//#define STA_SSID            "HOME_IOT"
#define STA_PASS            "AAAAABBBBB"
#define PLACE               "LAB"
#define MQTT_CLIENT_ID      "HT_%02X%02X%02X%02X%02X%02X" //"DVES_%08X"
#define MQTT_DEV            "HT_%02X%02X%02X%02X%02X%02X"
#define DEFAULT_SECURITY    NO_TLS              // very important: you must config DEFAULT_SECURITY for SSL/TLS
#endif //_LAB_CONFIG_

//------------------------------MQTT Broker Section------------------------------------
//#define DEFAULT_SECURITY    NO_TLS              // very important: you must config DEFAULT_SECURITY for SSL/TLS
//#define DEFAULT_SECURITY    TLS_WITHOUT_AUTHENTICATION
//#define DEFAULT_SECURITY    ONE_WAY_ANTHENTICATION
//#define DEFAULT_SECURITY    TWO_WAY_ANTHENTICATION

//---------------------------------------------------------------------------------------

#define NODE_NAME           "HUMT"
#define UTC_OFFSET          2
#define TIMEZONE            "Asia/Jerusalem" //https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
//#define AP_PWD              "%02x%02x%02x%02x%02x%02x"
#define DEVICE_CAPABILITIES  "\"cap\":[{\"cname\":\"HUMT\",\"qty\":1}]"

/* topic = MQTT_TOPIC_BASE/GW_MAC/TOPIC_custom */

//Custom topics
#define TOPIC_ALIVE         "/alive"	//alive topic
#define TOPIC_INIT          "/lwt"	//last will topic
#define TOPIC_ANS           "/ANS"  //Publish answer
#define TOPIC_CFG           "/CFG"  //Subscribe to Device info request
#define TOPIC_SFG           "/SFG"  //Subscribe to config change
#define TOPIC_INF           "/INF"  //Publish WiFi info
#define TOPIC_FOTA          "/FOTA" //Subscribe to Firmware update
#define TOPIC_TZD           "/TZD"  //Subscribe to time zone difference
#define TOPIC_TZR           "/TZR"  //Publish time zone difference request
#define TOPIC_HUMT          "/HUMT" //Publish Humidity and Temperature
//define AP password
//#define MAC2PWD(a) (a)[0], (a)[1], (a)[2], (a)[5], (a)[4], (a)[3]
//#define MAC2PWD(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

#define STA_TYPE AUTH_WPA2_PSK

#define CA_CERT_FLASH_ADDRESS 0x77              // CA certificate address in flash to read, 0x77 means address 0x77000
#define CLIENT_CERT_FLASH_ADDRESS 0x78          // client certificate and private key address in flash to read, 0x78 means address 0x78000
/***********************************************************************************************************************/
/*Please Keep the following configuration if you have no very deep understanding of ESP SSL/TLS*/
#define CFG_LOCATION	        0x79	/* 0x79 Please don't change or if you know what you doing */
#define MQTT_BUF_SIZE		    1024
#define MQTT_KEEPALIVE		    120	    /*second*/
#define MQTT_QOS                1
#define MQTT_RETRAIN            0
#define MQTT_RECONNECT_TIMEOUT  5       /*second*/
#define MQTT_SSL_ENABLE                 /* Please don't change or if you know what you doing */

#define STA_TYPE AUTH_WPA2_PSK
#define QUEUE_BUFFER_SIZE		2048

#define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif // __MQTT_CONFIG_H__
