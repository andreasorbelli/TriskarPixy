#include "ch.h"
#include "hal.h"

#include "r2p/Middleware.hpp"
#include <r2p/msg/motor.hpp>

#include "pwm_node.hpp"

int16_t pwm = 0;

/*
 * PWM cyclic callback.
 */
static void pwmcb(PWMDriver *pwmp) {

	(void) pwmp;
	chSysLockFromIsr()
	;
	if (pwm >= 0) {
		pwm_lld_enable_channel(&PWMD1, 0, pwm);
		pwm_lld_enable_channel(&PWMD1, 1, 0);
	} else {
		pwm_lld_enable_channel(&PWMD1, 0, 0);
		pwm_lld_enable_channel(&PWMD1, 1, -pwm);
	}
	chSysUnlockFromIsr();
}

/*
 * PWM configuration.
 */
static PWMConfig pwmcfg = { 36000000, /* 72MHz PWM clock frequency.   */
4096, /* 12-bit PWM, 17KHz frequency. */
pwmcb, { { PWM_OUTPUT_ACTIVE_HIGH, NULL }, { PWM_OUTPUT_ACTIVE_HIGH, NULL }, { PWM_OUTPUT_DISABLED, NULL }, {
	PWM_OUTPUT_DISABLED, NULL } }, 0, 0};

/*
 * PWM subscriber node
 */

msg_t pwm_node(void * arg) {
	pwm_node_conf * conf = (pwm_node_conf *) arg;
	r2p::Node node(conf->name);
	r2p::Subscriber<r2p::PWM2Msg, 5> pwm_sub;
	r2p::PWM2Msg * msgp;

	(void) arg;

	chRegSetThreadName(conf->name);

	/* Enable the h-bridge. */
	palSetPad(GPIOB, GPIOB_MOTOR_ENABLE);
	palClearPad(GPIOA, GPIOA_MOTOR_D1);
	chThdSleepMilliseconds(500);
	pwmStart(&PWM_DRIVER, &pwmcfg);

	node.subscribe(pwm_sub, conf->topic);

	for (;;) {
		if (node.spin(r2p::Time::ms(1000))) {
			if (pwm_sub.fetch(msgp)) {
				pwm = msgp->value[conf->id];
				chSysLock()
				;
				if (pwm >= 0) {
					pwm_lld_enable_channel(&PWMD1, 0, pwm);
					pwm_lld_enable_channel(&PWMD1, 1, 0);
				} else {
					pwm_lld_enable_channel(&PWMD1, 0, 0);
					pwm_lld_enable_channel(&PWMD1, 1, -pwm);
				}
				chSysUnlock();
				pwm_sub.release(*msgp);
			}
		} else {
			// Stop motor if no messages for 1000 ms
			pwm_lld_disable_channel(&PWM_DRIVER, 0);
			pwm_lld_disable_channel(&PWM_DRIVER, 1);
		}
	}
	return CH_SUCCESS;
}
