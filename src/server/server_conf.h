#pragma once

struct srv_conf_st {
	char *snd_port;
	char *mgroup;
	char *media_path;
	char *help;
	int  run_daemon;
};

#define DEFAULT_MEDIA_PATH "../../media_dir"
