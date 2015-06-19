#include "ch.h"
#include "hal.h"

#include "r2p/Middleware.hpp"
#include <r2p/msg/motor.hpp>

#include "nodes/encoder_node.hpp"
#include "qei.h"

/*
 * QEI configuration
 */
QEIConfig qeicfg = { QEI_MODE_QUADRATURE, QEI_BOTH_EDGES, QEI_DIRINV_TRUE, };

/*
 * Encoder publisher node
 */
msg_t encoder_node(void *arg) {
	encoder_node_conf * conf = (encoder_node_conf *) arg;
	r2p::Node node(conf->name);
	r2p::Publisher<r2p::EncoderMsg> enc_pub;
	systime_t time;
	qeidelta_t delta;
	r2p::EncoderMsg *msgp;

	(void) arg;
	chRegSetThreadName(conf->name);

	/* Enable the QEI driver. */
	qeiInit();
	qeiStart(&QEI_DRIVER, &qeicfg);
	qeiEnable (&QEI_DRIVER);

	node.advertise(enc_pub, conf->topic);

	for (;;) {
		time = chTimeNow();
		delta = qeiUpdate(&QEI_DRIVER);

		if (enc_pub.alloc(msgp)) {
			msgp->delta = delta * conf->t2r;
			enc_pub.publish(*msgp);
		}

		time += MS2ST(20);
		chThdSleepUntil(time);
	}

	return CH_SUCCESS;
}
