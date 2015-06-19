#include "ch.h"
#include "hal.h"

#include "r2p/Middleware.hpp"
#include "led/nodes/led.hpp"

#include "nodes/encoder_node.hpp"
#include "nodes/pid_node.hpp"

#define _TICKS 64.0f
#define _RATIO 30.0f
#define _PI 3.14159265359f

#define T2R 1 / ((1 / (2 * _PI)) * (_TICKS * _RATIO))

static WORKING_AREA(wa_info, 1024);
static r2p::RTCANTransport rtcantra(RTCAND1);

RTCANConfig rtcan_config = { 1000000, 100, 60 };

r2p::Middleware r2p::Middleware::instance(MODULE_NAME, "BOOT_"MODULE_NAME);

/*
 * Application entry point.
 */
extern "C" {
int main(void) {

	halInit();
	chSysInit();

	r2p::Middleware::instance.initialize(wa_info, sizeof(wa_info), r2p::Thread::LOWEST);
	rtcantra.initialize(rtcan_config);
	r2p::Middleware::instance.start();

	r2p::ledsub_conf ledsub_conf = { "led" };
	r2p::Thread::create_heap(NULL, THD_WA_SIZE(512), NORMALPRIO, r2p::ledsub_node, &ledsub_conf);

	pid_node_conf pid_conf = {"pid_node", "encoder0", 0, {100, 0, 0, 0.02}};
	r2p::Thread::create_heap(NULL, THD_WA_SIZE(2048), NORMALPRIO + 2, pid_node, &pid_conf);

	encoder_node_conf encoder_conf = {"encoder_node", "encoder0", T2R};
	r2p::Thread::create_heap(NULL, THD_WA_SIZE(2048), NORMALPRIO + 2, encoder_node, &encoder_conf);

	for (;;) {
		r2p::Thread::sleep(r2p::Time::ms(500));
	}

	return CH_SUCCESS;
}
}

