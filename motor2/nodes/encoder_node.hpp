#pragma once

struct encoder_node_conf {
	const char * name;
	const char * topic;
	float t2r;
};

msg_t encoder_node(void *arg);
