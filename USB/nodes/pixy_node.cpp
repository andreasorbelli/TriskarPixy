
#include "ch.h"
#include "hal.h"
#include "r2p/Middleware.hpp"
#include <r2p/msg/motor.hpp>



/*
 * UART driver configuration structure.
 */
static UARTConfig uart_cfg_1 = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  19200,
  0,
  USART_CR2_LINEN,
  0
};

msg_t pixy_node(void *arg) {
	r2p::Node node("pixy");
	uartStart(&UARTD3,&uart_cfg_1);
    char buffer[14] ;


	for (;;) {
		uartStartReceive(&UARTD3,14,buffer);
		r2p::Thread::sleep(r2p::Time::ms(500));
	}

	return CH_SUCCESS;
}
