#pragma once

struct pwm_node_conf {
	const char * name;
	const char * topic;
	uint8_t id;
};

msg_t pwm_node(void *arg);
