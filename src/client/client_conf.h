#pragma once

#include <unistd.h>
#include <getopt.h>

struct  client_conf_st {
	char *recv_port;
	char *mgroup;
	char *player;
};
