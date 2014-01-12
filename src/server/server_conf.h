#pragma once

struct srv_conf_st {
	char *rcv_port;
	char *mgroup;
	char *media_path;
	char *help;
	char *nic_if;
	int  run_daemon;
};

#define DEFAULT_MEDIA_PATH "../../media_dir"
#define DEFAULT_RUN_DAEMON    0
#define DEFAULT_NIC_IF	    "eth0"

#define BUF_SIZE	4096
#define PATH_SIZE	255

#define DESC_FNAME	"desc.txt"
#define AUDIO_FILE	"*.mp3"
#define MP3_BITRATE	(65536 / 8)
#define BURST_CACHE_SIZE	(MP3_BITRATE * 10)

