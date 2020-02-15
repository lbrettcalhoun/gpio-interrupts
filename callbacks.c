#include "user_interface.h"
#include "espconn.h"
#include "osapi.h"
#include "user_config.h"
#include "debug.h" // Remove this line to disable debugging

// Sent callback function. Not much to do here so let's just output some debug. 
void ICACHE_FLASH_ATTR sent_callback(void *arg) 
{
  #ifdef DEBUG_ON
    os_printf("Sent message to destination\n"); 
  #endif
}

// Receive callback function. Just in case we get something sent to us.
void ICACHE_FLASH_ATTR receive_callback(void *arg, char *p_data, unsigned short len) 
{
  #ifdef DEBUG_ON
    os_printf("Received %d bytes: %s\n", len, p_data);
  #endif
}
