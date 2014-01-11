#pragma once
#include <proto.h>
#include <utils.h>

struct   upmedia_st {
	struct chnlist_info_st *pchnlist;
	int (*get_chn_list)(struct upmedia_st *info, size_t *len);
	int (*read_chn)(uint8_t chnid, void *buf, size_t buf_size);
};

/*构造和析构*/
int upmedia_init(struct upmedia_st *info, int chn_nums);
int upmedia_destroy(struct upmedia_st *info);
