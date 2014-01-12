#pragma once
#include <proto.h>
#include <utils.h>
#include <glob.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "tbf.h"

#define  DESC_MAX_SIZE	128
#define  AUDIO_BUF_SIZE	81920

struct  chn_list_entry {
	uint8_t chn_id;
	char desc[DESC_MAX_SIZE];
};

/*主要是描述各个频道的详细信息*/
struct  chn_stat_st {
	struct chn_list_entry ent;
	glob_t glob_info;
	int cur; //下一个文件在glob_info中的索引号
	int fd; //文件描述符
	off_t offset;//偏移量
	size_t audio_nums; //mp3文件数
	struct tbf_st *ptbf;
	pthread_t chn_tid;	
};

struct   upmedia_st {
	struct  chn_stat_st achn_stat[CHN_NUMS];
	uint8_t chn_nums; //可用的频道数
	struct chnlist_info_st *pchnlist; //频道列表信息
	int (*get_chn_list)(struct upmedia_st *info, int *len);
	int (*read_chn)(struct upmedia_st *info, int chnid, void *buf, size_t buf_size);
};

/*构造和析构*/
int upmedia_init(struct upmedia_st *info);
int upmedia_destroy(struct upmedia_st *info);
