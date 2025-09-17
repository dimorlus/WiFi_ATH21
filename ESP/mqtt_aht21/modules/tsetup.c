#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "user_config.h"
#include "mqtt_config.h"
#include "parser.h"
#include "tsetup.h"
#include "config.h"

//#define PRN     os_printf
#define PRN

//---------------------------------------------------------------------------
//Telnet setup
LOCAL struct espconn esp_conn;
LOCAL esp_tcp esptcp;

#define TSTRLEN 128
#define RSTRLEN 1024
static uint32_t tidx = 0;
static char *rstr = 0L;
static char tstr[TSTRLEN];

extern void ICACHE_FLASH_ATTR EnterSetup(void);
extern void ICACHE_FLASH_ATTR LeaveSetup(void);

//---------------------------------------------------------------------------
LOCAL void tcp_server_multi_send(char *pbuf, int len);
//---------------------------------------------------------------------------

LOCAL void tcp_server_str(char *pbuf)
 {
  tcp_server_multi_send(pbuf, os_strlen(pbuf));
 }
//---------------------------------------------------------------------------

/******************************************************************************
 �* FunctionName : tcp_server_recv_cb
 �* Description� : receive callback.
 �* Parameters�� : arg -- Additional argument to pass to the callback function
 �* Returns����� : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
  // received data from tcp connection
  uint32_t i;
  struct espconn *pespconn = arg;

  // Ensure response buffer exists
  if (!rstr) rstr = (char *)os_zalloc(RSTRLEN);
  if (!rstr) return; // OOM safeguard

  if (tidx == 0) tstr[0] = '\0';

  bool should_send = false;
  for (i = 0; i < length; i++)
  {
    char c = pusrdata[i];
    // Like the original logic: end-of-line is LF only; CR is ignored.
    if (c != '\n')
    {
      if (tidx < TSTRLEN - 1)
      {
        if (c >= ' ') tstr[tidx++] = c; // ignore control chars like CR
        tstr[tidx] = '\0';
      }
      else
      {
        // Overflow protection: reset the line to avoid drifting state
        tidx = 0;
        tstr[0] = '\0';
      }
    }

    if (c == '\n')
    {
      // Complete current line and parse
      tstr[tidx] = '\0';
      PRN("%s\n", tstr);
      parse(tstr, rstr, RSTRLEN);
      PRN("%s\n", rstr);
      tidx = 0; // reset for the next line in this same packet
      should_send = true; // Defer sending until after processing all bytes of this packet
    }
  }
  if (should_send)
  {
    espconn_sent(pespconn, rstr, os_strlen(rstr));
  }
}
//---------------------------------------------------------------------------
/******************************************************************************
 �* FunctionName : tcp_server_sent_cb
 �* Description� : data sent callback.
 �* Parameters�� : arg -- Additional argument to pass to the callback function
 �* Returns����� : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR tcp_server_sent_cb(void *arg)
 {
  if (rstr)
   {
    os_free(rstr);
    rstr = 0L;
   }
  //data sent successfully
  PRN("tcp sent cb \r\n");
 }
//---------------------------------------------------------------------------
/******************************************************************************
 �* FunctionName : tcp_server_discon_cb
 �* Description� : disconnect callback.
 �* Parameters�� : arg -- Additional argument to pass to the callback function
 �* Returns����� : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR tcp_server_discon_cb(void *arg)
 {
  //tcp disconnect successfully
  tidx = 0;
  if (rstr)
   {
    os_free(rstr);
    rstr = 0L;
   }
  parse("", 0L, 0);
  PRN("tcp disconnect succeed !!! \r\n");
  CFG_Load();
  LeaveSetup();
 }
//---------------------------------------------------------------------------
/******************************************************************************
 �* FunctionName : tcp_server_recon_cb
 �* Description� : reconnect callback, error occured in TCP connection.
 �* Parameters�� : arg -- Additional argument to pass to the callback function
 �* Returns����� : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR tcp_server_recon_cb(void *arg, sint8 err)
 {
  //error occured , tcp connection broke.
  tidx = 0;
  parse("", 0L, 0);
  if (rstr)
   {
    os_free(rstr);
    rstr = 0L;
   }
  PRN("reconnect callback, error code %d !!! \r\n", err);
 }
//---------------------------------------------------------------------------
LOCAL void tcp_server_multi_send(char *pbuf, int len)
 {
  struct espconn *pesp_conn = &esp_conn;

  remot_info *premot = NULL;
  uint8 count = 0;
  sint8 value = ESPCONN_OK;
  if (espconn_get_connection_info(pesp_conn, &premot, 0) == ESPCONN_OK)
   {
    for (count = 0; count < pesp_conn->link_cnt; count++)
     {
      pesp_conn->proto.tcp->remote_port = premot[count].remote_port;

      pesp_conn->proto.tcp->remote_ip[0] = premot[count].remote_ip[0];
      pesp_conn->proto.tcp->remote_ip[1] = premot[count].remote_ip[1];
      pesp_conn->proto.tcp->remote_ip[2] = premot[count].remote_ip[2];
      pesp_conn->proto.tcp->remote_ip[3] = premot[count].remote_ip[3];

      espconn_sent(pesp_conn, pbuf, len);
     }
   }
 }
//---------------------------------------------------------------------------
/******************************************************************************
 �* FunctionName : tcp_server_listen
 �* Description� : TCP server listened a connection successfully
 �* Parameters�� : arg -- Additional argument to pass to the callback function
 �* Returns����� : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR tcp_server_listen(void *arg)
 {
  struct espconn *pesp_conn = arg;
  tidx = 0;
  parse("", 0L, 0);
  EnterSetup();
  espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);
  espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
  espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);
  espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
  //tcp_server_multi_send();
  tcp_server_str("Telnet setup\r\n");
  PRN("tcp_server_listen !!! \r\n");
 }

//---------------------------------------------------------------------------
/******************************************************************************
 �* FunctionName : user_tcpserver_init
 �* Description� : parameter initialize as a TCP server
 �* Parameters�� : port -- server port
 �* Returns����� : none
 *******************************************************************************/
void ICACHE_FLASH_ATTR user_tcpserver_init(uint32_t port)
 {
  tidx = 0;
  parse("", 0L, 0);
  esp_conn.type = ESPCONN_TCP;
  esp_conn.state = ESPCONN_NONE;
  esp_conn.proto.tcp = &esptcp;
  esp_conn.proto.tcp->local_port = port;
  espconn_regist_connectcb(&esp_conn, tcp_server_listen);

  sint8 ret = espconn_accept(&esp_conn);

  PRN("espconn_accept [%d] !!! \r\n", ret);

  //espconn_set_opt(&esp_conn, ESPCONN_KEEPALIVE);
  espconn_set_opt(&esp_conn, ESPCONN_REUSEADDR|ESPCONN_NODELAY);
  espconn_tcp_set_max_con_allow(&esp_conn, 1);

  espconn_regist_time(&esp_conn, 60, 0);
 }
//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR tsetup_stop(void)
{
  // Disconnect all active clients
  remot_info *premot = NULL;
  if (espconn_get_connection_info(&esp_conn, &premot, 0) == ESPCONN_OK) {
    uint8 count;
    for (count = 0; count < esp_conn.link_cnt; count++) {
      esp_conn.proto.tcp->remote_port = premot[count].remote_port;
      esp_conn.proto.tcp->remote_ip[0] = premot[count].remote_ip[0];
      esp_conn.proto.tcp->remote_ip[1] = premot[count].remote_ip[1];
      esp_conn.proto.tcp->remote_ip[2] = premot[count].remote_ip[2];
      esp_conn.proto.tcp->remote_ip[3] = premot[count].remote_ip[3];
      espconn_disconnect(&esp_conn);
    }
  }
  tidx = 0;
  if (rstr) { os_free(rstr); rstr = 0; }
  parse("", 0L, 0);
  LeaveSetup();
  PRN("tsetup_stop done\n");
}
//---------------------------------------------------------------------------
