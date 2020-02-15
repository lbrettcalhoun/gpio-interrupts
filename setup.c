#include "credentials.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "espconn.h"
#include "mem.h"
#include "debug.h"

// Setup GPIO2
void ICACHE_FLASH_ATTR setup_gpio ()
{
  // Initialize the GPIO sub-system
  gpio_init();

  // Set GPIO2 to be GPIO2 ... yeah it sounds stupid to do this but you
  // don't know how GPIO2 was previously configured ... it could have been
  // configured for something completely different
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
 
  // Set GPIO2 as output and set it to LOW
  gpio_output_set(0, BIT2, BIT2, 0);

}

// Setup udp_espconn structure as a UDP connection block. Notice that we
// pass in a pointer to udp_espconn. Also notice that because our parameter
// is a pointer we have to use -> to set the members.
void ICACHE_FLASH_ATTR setup_udp(struct espconn *p_espconn) 
{
  
  // Set the connection type to UDP
  p_espconn->type = ESPCONN_UDP;

  // Zero out the potential junk that may be in our structure's proto.udp member
  p_espconn->proto.udp = (esp_udp *) os_zalloc(sizeof (esp_udp));

  // Set a local port for our UDP server. Just pick any available port.
  p_espconn->proto.udp->local_port = espconn_port();

}

// Setup the WiFi.
void ICACHE_FLASH_ATTR setup_wifi (void) {

  char const *SSID = WIFI_SSID;
  char const *PASSWORD = WIFI_PASSWORD;

  // Get the current AP configuration
  struct softap_config config;
  wifi_softap_get_config(&config);

  // Don't forget that config.ssid needs to be cast to a pointer because SSID is
  // itself a pointer (look above where you defined it). Also notice how we have to
  // null the SSID and password pointers or you will have junk left over from the
  // previous run.  We use os_bzero to null these pointers (aka char arrays).
  config.ssid_len = 8;
  os_bzero(&config.ssid, 32);
  os_memcpy(&config.ssid, SSID, 8);
  os_bzero(&config.password, 64);
  os_memcpy(&config.password, PASSWORD, 10);
  config.authmode = AUTH_WPA2_PSK;
  wifi_softap_set_config(&config);

  #ifdef DEBUG_ON
    os_printf("WIFI Configuration:\n");
    os_printf("SSID: %s\n", config.ssid);
    os_printf("Auth Mode: %d\n", config.authmode);
  #endif

}

