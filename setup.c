#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "debug.h"

// This function is defined in user_main.c
extern void interrupt_function();

// Setup GPIO2 for either the "LOW" example or the "HIGH" example. Note that 
// there is no debounce on the momentary switches hardware or in this software.
// So you will get some extra output when the switches bounce.
void ICACHE_FLASH_ATTR setup_gpio (uint8 example)
{
  // Initialize the GPIO sub-system
  gpio_init();

  // Set GPIO2 to be GPIO2 ... yeah it sounds stupid but gotta do it because
  // the ESP8266 multiplexes all kinds of different functions for each pin.
  // If you need to see all the different pin functions (FUNC_GPIO2) or the
  // different multiplexers (PERIPHS_IO_MUX_GPIO2_U) then look in eagle_soc.h.
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
 
  // Temporarily disable all GPIO interrupts while we make changes to the configuration
  ETS_GPIO_INTR_DISABLE();

  if (!example) {

    // LOW example
    // Set GPIO2 as input and enable the internal pullup resistor. Connect GPIO2
    // to a momentary contact switch and then complete the circuit with a resistor
    // and on to ground. When you press the switch GPIO2 will transition to LOW and
    // the interrupt will fire.
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
  
    // Clear out the interrupt status for GPIO2
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT2);
  
    // Set the interrupt handler for the GPIO pins
    ETS_GPIO_INTR_ATTACH(interrupt_function, NULL);
  
    // Configure the interrupt handler to fire on change to the pin status ... in this 
    // example since we are holding the pin HIGH with the pullup let's fire the interrupt
    // when the pin goes LOW (NEGEDGE)
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_NEGEDGE);
  }

  else {

    // HIGH example. Note that Espressif took the old PIN_PULLDWN_EN out of the SDK.
    // So we have to manually use a resistor on the breadboard to pulldown the pin.
    // Set GPIO2 as input and disable the internal pullup resistor. Connect GPIO2
    // to a momentary contact switch and then complete the circuit by going straight
    // on to power. Put a resistor between the switch and ground ... this is our pulldown.
    //  When you press the switch GPIO2 will transition to HIGH and the interrupt will fire.
    // IMPORTANT NOTE: You have to connect the pulldown resistor AFTER you boot the ESP8266.
    // Otherwise you will get an endless loop of gibberish on the debug output..
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);
  
    // Clear out the interrupt status for GPIO2
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT2);
  
    // Set the interrupt handler for the GPIO pins
    ETS_GPIO_INTR_ATTACH(interrupt_function, NULL);
  
    // Configure the interrupt handler to fire on change to the pin status ... in this 
    // example since we are holding the pin LOW with the pulldown let's fire the interrupt
    // when the pin goes HIGH (POSEDGE)
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_POSEDGE);
  
  }

  // And finally enable the interrupt handler ... we're ready for action!
  ETS_GPIO_INTR_ENABLE();
  
}
