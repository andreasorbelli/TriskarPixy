#pragma once

/*
 * PID configuration
 */

struct pid_conf_t {
	float k;
	float ti;
	float td;
	float ts;
};

struct pid_node_conf {
	const char * name;
	const char * encoder_topic;
	uint8_t id;
	pid_conf_t pid_conf;
};


msg_t pid_node(void * arg);
