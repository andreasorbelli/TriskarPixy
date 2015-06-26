
#include "ch.h"
#include "hal.h"
#include "r2p/Middleware.hpp"
#include <r2p/msg/pixy.hpp>



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

	r2p::Publisher<r2p::PixyMsg> pixy_pub;
	r2p::Node node("pixy");
	r2p::PixyMsg * msgp;
	uartStart(&UARTD3,&uart_cfg_1);
    char buffer[14] ;

   	node.advertise(pixy_pub,"pixy");

	for (;;) {
		uartStartReceive(&UARTD3,1,buffer);
		if(buffer[0]=="55"){
			uartStartReceive(&UARTD3,1,buffer);
			if(buffer[0]=="AA"){
				uartStartReceive(&UARTD3,1,buffer);
				if(buffer[0]=="55"){
					uartStartReceive(&UARTD3,1,buffer);
					if(buffer[0]=="AA"){
						uartStartReceive(&UARTD3,40,buffer);
						if (pixy_pub.alloc(msgp)) {
							memcpy(msgp->buffer, buffer, 40);
							pixy_pub.publish(*msgp);
							r2p::Thread::sleep(r2p::Time::ms(500));
						}

					}

				}

			}

		}
		r2p::Thread::sleep(r2p::Time::ms(50));
	}

	return CH_SUCCESS;
}
