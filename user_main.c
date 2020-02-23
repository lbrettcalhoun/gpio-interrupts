// Configure ESP8266 to read digital inputs via GPIO interrupts
// Read GPIO2 and print message when GPIO2 changes state (LOW - HIGH
// or HIGH - LOW)

// Compile, link and then convert to a bin using the Makefile:  
//      make clean 
//      make
// Then flash to device using the Makefile.  Device must be in program mode:
//      make flash

// IMPORTANT: Remember that the ESP8266 NON-OS firmware does not contain an
// operating system. No OS means no task scheduler. This means we have to use
// some other means of putting our code on the SoC for execution.

#include "user_interface.h"
#include "osapi.h"
#include "gpio.h"
#include "debug.h"

// The GPIO interrupt handler function.
void ICACHE_FLASH_ATTR interrupt_function()
{

  uint32 gpio_status;
  uint8 pin_status;
  static uint32 counter = 1;

  // First disable the interrupts (temporarily)
  ETS_GPIO_INTR_DISABLE();

  // Read the pin mask ... you get back a mask of ALL pins
  gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

  // Read the individual GPIO2 pin status ... you will get back a 0 or 1 (low or high)
  pin_status = GPIO_INPUT_GET(GPIO_ID_PIN(2));

  #ifdef DEBUG_ON
  os_printf("Intr function fired %d! Pin status is: %d decimal Mask status is %x hex\n", counter, pin_status, gpio_status);
  #endif

  // Now clear the interrupt status
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

  // And finally re-enable the interrupts ... we're now ready to go again!
  ETS_GPIO_INTR_ENABLE();

  counter++;

}


// This is the system init done callback function. It is called once the SoC
// completes all it's initialization activities. This is where we do our setup
LOCAL void ICACHE_FLASH_ATTR init_done_callback(void) 
{

  uart_div_modify(0, UART_CLK_FREQ / 115200);
  #ifdef DEBUG_ON
    os_printf("\n\nDEBUG_ON\n");
  #endif

  // Now do our setup ... let's do the LOW example this time so pass in a 0 as the arg
  //setup_gpio(0);
  // Now do our setup ... let's do the HIGH example this time so pass in a 1 as the arg
  setup_gpio(1);

}

// Entry function ... execution starts here. Remember this ain't like regular
// c main function!
void ICACHE_FLASH_ATTR user_init (void) {
  
  // And here is our system init done callback. Once the SoC has done its 
  // setup it will execute the function init_done_callback.
  system_init_done_cb(init_done_callback);

}
