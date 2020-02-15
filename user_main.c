// Configure ESP8266 to send a UDP datagram to 192.168.4.2

// Compile, link and then convert to a bin using the Makefile:  
//      make clean 
//      make
// Then flash to device using the Makefile.  Device must be in program mode:
//      make flash

// IMPORTANT: Remember that the ESP8266 NON-OS firmware does not contain an
// operating system. No OS means no task scheduler. This means we have to use
// some other means of putting our code on the SoC for execution.

#include "user_interface.h"
#include "espconn.h"
#include "osapi.h"
#include "debug.h" // Remove this line to disable debugging

#define MESSAGE "ESP8266"

LOCAL struct espconn udp_espconn;
LOCAL os_timer_t the_timer;


// RF Pre-Init function ... according to SDK API reference this needs to be
// in user_main.c even though we aren't using it.  It can be used to set RF
// options. We aren't setting any options so just leave it empty.
void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

// RF calibrate sector function ... again ... SDK API says to add this to
// user_main.c.  It is used to set the RF calibration sector.  Note that the
// SDK API says that you have to add it to user_main.c but you don't ever have
// to call it; it will be called by the SDK itself.
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:

            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

// Timer function. This function will run every 2 seconds. In this function we
// reset the remote IP and port (something the SDK says you have to do each
// time you send a packet) and then we send the data to our destination.
LOCAL void ICACHE_FLASH_ATTR timer_function()
{

  // A simple counter to count each time we send a message. Static so it will
  // retain it's value each time the function runs.
  static int counter = 0;

  sint16 result = 0;

  const char udp_remoteip[4] = {192,168,4,2};
  os_memcpy(udp_espconn.proto.udp->remote_ip, udp_remoteip, 4);
  udp_espconn.proto.udp->remote_port = 8266;

  // Let's create a pointer to our esp_udp structure so we can more succinctly 
  // parse the individual elements of our remote IP array. In other words, we
  // don't have to repeatedly type "udp_espconn.proto.udp->remote_ip[x]" in
  // our degbug output. Remember that the esp_udp structure is within the proto 
  // union within our espconn structure. It's buried kind of deep!
  esp_udp *p_udp;
  p_udp = udp_espconn.proto.udp;

  #ifdef DEBUG_ON
    os_printf("Reset Remote IP: %d.%d.%d.%d\n", p_udp->remote_ip[0],p_udp->remote_ip[1],p_udp->remote_ip[2],p_udp->remote_ip[3]);
    os_printf("Reset Remote port: %d\n", p_udp->remote_port);
    os_printf("Counter: %d\n", counter);
  #endif

  result = espconn_sendto(&udp_espconn, MESSAGE, 8);
 
  #ifdef DEBUG_ON
    os_printf("espconn sendto status: %d\n", result);
  #endif

  counter++;

}

// Sent callback function. Not much to do here so let's just output some debug. 
LOCAL void ICACHE_FLASH_ATTR sent_callback(void *arg) 
{
  #ifdef DEBUG_ON
    os_printf("Sent message to destination\n"); 
  #endif
}

// Create UDP function. This is where we setup our espconn connection block
// including the desination IP and port. Then we create the connection.
LOCAL void ICACHE_FLASH_ATTR create_udp() 
{
  sint16 result = 0;
 
  const char udp_remoteip[4] = {192,168,4,2};
  os_memcpy(udp_espconn.proto.udp->remote_ip, udp_remoteip, 4);
  udp_espconn.proto.udp->remote_port = 8266;
  
  // Create the UDP connection 
  result = espconn_create(&udp_espconn);

  #ifdef DEBUG_ON
    os_printf("espconn create status: %d\n", result);
  #endif

  // Let's create a pointer to our esp_udp structure so we can more succinctly 
  // parse the individual elements of our remote IP array. In other words, we
  // don't have to repeatedly type "udp_espconn.proto.udp->remote_ip[x]" in
  // our degbug output. Remember that the esp_udp structure is within the proto 
  // union within our espconn structure. It's buried kind of deep!

  esp_udp *p_udp;
  p_udp = udp_espconn.proto.udp;
  
  #ifdef DEBUG_ON
    os_printf("Remote IP: %d.%d.%d.%d\n", p_udp->remote_ip[0],p_udp->remote_ip[1],p_udp->remote_ip[2],p_udp->remote_ip[3]);
    os_printf("Remote port: %d\n", p_udp->remote_port);
  #endif

  // Register our sent callback ... see the sent_callback function
  espconn_regist_sentcb(&udp_espconn, sent_callback);

  // And last but not least, setup and arm the timer ... see timer_function
  os_timer_disarm(&the_timer);
  os_timer_setfn(&the_timer, (os_timer_func_t *)timer_function, NULL);
  os_timer_arm(&the_timer, 2000, 1);
 
}

// This is the system init done callback. Once SoC has initialized the chip it
// will execute this function. This is where we do all our setup and then call
// create_udp to create the connection. From there, create_udp will set and
// arm the timer and we'll send a message each time the timer executes.
LOCAL void ICACHE_FLASH_ATTR init_done_callback(void) 
{

  uart_div_modify(0, UART_CLK_FREQ / 115200);
  #ifdef DEBUG_ON
    os_printf("\n\nDEBUG_ON\n");
  #else
    setup_gpio();
  #endif
  setup_wifi();
  setup_udp(&udp_espconn);
  #ifdef DEBUG_ON
    os_printf("espconn type: %d\n", udp_espconn.type);
    os_printf("espconn local port: %d\n", udp_espconn.proto.udp->local_port);
  #endif
  create_udp();
}

// Entry function ... execution starts here. Remember this ain't like regular
// c main function!
void ICACHE_FLASH_ATTR user_init (void) {
  
  // And here is our system init done callback. Once the SoC has done its 
  // setup it will execute the function init_done_callback.
  system_init_done_cb(init_done_callback);

}
