#include <stdio.h>
#include <string.h>
#include "upmedia.h"
#include "server_conf.h"

extern struct srv_conf_st srv_cfg;

static int read_chn(struct upmedia_st *info, int chnid, void *buf, size_t buf_size)
{
	int n, i;
	int fd = info->achn_stat[chnid].fd;
	int cur = info->achn_stat[chnid].cur;
	char *fname = NULL;
 
	while ((n = read(fd, buf, buf_size)) <= 0) {

		if (n < 0) {
			SYSLOG(LOG_ERR, "read_chn %d faild.(%s)", chnid, strerror(errno));
			return n;
		} else if (n == 0) {
			//TODO 循环播放，乱序播放
			SYSLOG(LOG_ERR, "file read end, turn to next.");
			for (cur += 1; cur <= info->achn_stat[chnid].glob_info.gl_pathc; ++cur) {

				if (cur == info->achn_stat[chnid].audio_nums) {
					SYSLOG(LOG_INFO, "channel:%d all audio send over. repaly again", chnid);
					SYSLOG(LOG_INFO, "channel:%d music:%s\n",chnid,info->achn_stat[chnid].glob_info.gl_pathv[0]);
					cur = 0;
				}
				fname = info->achn_stat[chnid].glob_info.gl_pathv[cur];
				fd = open(fname, O_RDONLY);
				if (fd < 0) {
					SYSLOG(LOG_ERR, "%s cant open.(%s),try next", fname, strerror(errno));
					continue;
				}
				
				SYSLOG(LOG_INFO, "channel:%d Now music:%s, cur:%d\n", chnid, info->achn_stat[chnid].glob_info.gl_pathv[cur], cur);
				break;
			}
			info->achn_stat[chnid].fd = fd;
        		info->achn_stat[chnid].cur = cur;	
		}
	}
	return n;
}

static int get_chn_list(struct upmedia_st *info, int *len)
{
	assert(info != NULL);
	assert(len != NULL);

	int i, n, count = 1;

	struct chnlist_info_st *pchnlist = (struct chnlist_info_st *)
		malloc(MAX_PACK_SIZE);
	if (pchnlist == NULL) {
		SYSLOG(LOG_ERR, "malloc failed.(%s)", strerror(errno));
		goto err;
	}

	pchnlist->chn_id = LIST_CHN_ID;
	struct chnlist_entry_st *pentry = pchnlist->entry;

	for (i = 1; i < info->chn_nums; ++i) {
		pentry->chn_id = info->achn_stat[i].ent.chn_id;
		n = strlen(info->achn_stat[i].ent.desc) + 1;
		pentry->len = htons(n);
		strncpy(pentry->desc, info->achn_stat[i].ent.desc, n);
		count += n + 3;
		DEBUG("chn_id:%d,chn_len:%hu,desc:%s,count:%d\n",
				pentry->chn_id, ntohs(pentry->len),
				pentry->desc, count);
		pentry = (struct chnlist_entry_st *)((char *)pentry + n + 3);
	}
	
	info->pchnlist = pchnlist;
	*len = count;

	return 0;
err:
	return -1;
}

/*构造和析构*/
int upmedia_init(struct upmedia_st *info)
{
	assert(info != NULL);
	
	int i, iret, n;
	glob_t glob_mdir;
	char path[PATH_SIZE] = {0};
	uint8_t chn_index = MIN_CHN_ID;
	int fd;

	n = snprintf(path, PATH_SIZE, "%s/*", srv_cfg.media_path);

	iret = glob(path, 0, NULL, &glob_mdir);
	if (iret != 0) {
		SYSLOG(LOG_ERR, "glob faild.(%s)", strerror(errno));
		goto err;
	}
	
	if (glob_mdir.gl_pathc <= 0) {
		SYSLOG(LOG_ERR, "media dir not found.");
		goto err;
	}

	for (i = 0; i < glob_mdir.gl_pathc; ++i) {
#ifdef  _DEBUG
		printf("%s\n", glob_mdir.gl_pathv[i]);
#endif
		/*检查名称是否是目录，不是目录则下一个*/
		char *dir_name = glob_mdir.gl_pathv[i];
	
		if (access(dir_name, X_OK) == -1) {
			SYSLOG(LOG_ERR, "%s dir_name cant enter.", dir_name);
			continue;
		}

		/*进入各个子目录, 遍历decr.txt *.mp3*/	
		snprintf(path, PATH_SIZE, "%s/%s", glob_mdir.gl_pathv[i], DESC_FNAME);
		fd = open(path, O_RDONLY);
		if (fd == -1) {
			SYSLOG(LOG_ERR, "%s cant open.(%s)", path, strerror(errno));
			info->achn_stat[chn_index].ent.desc[0] = '\0';
		}else {
			n = read(fd, info->achn_stat[chn_index].ent.desc, DESC_MAX_SIZE);
			if (n <= 0) {
				SYSLOG(LOG_ERR, "%s read failed.(%s)", path, strerror(errno));
				goto err;	
			}	
		}
		DEBUG("chn_id:%d,desc:%s\n", chn_index, 
				info->achn_stat[chn_index].ent.desc);
		info->achn_stat[chn_index].ent.chn_id = chn_index;
		snprintf(path, PATH_SIZE, "%s/%s", dir_name, AUDIO_FILE);
		if (glob(path, 0, NULL, &info->achn_stat[chn_index].glob_info)) {
			SYSLOG(LOG_ERR, "glob failed.(%s)", strerror(errno));
			goto err;
		}
		
		info->achn_stat[chn_index].cur = 0;
		info->achn_stat[chn_index].offset = 0;	
		info->achn_stat[chn_index].audio_nums = info->achn_stat[chn_index].glob_info.gl_pathc;
		for (n = 0; n < info->achn_stat[chn_index].glob_info.gl_pathc; ++n) {
			char *mp3file = info->achn_stat[chn_index].glob_info.gl_pathv[i];
			info->achn_stat[chn_index].fd = open(mp3file, O_RDONLY);
			if (info->achn_stat[chn_index].fd < 0) {
				SYSLOG(LOG_ERR, "%s cant open.(%s)", mp3file, strerror(errno));
				++info->achn_stat[chn_index].cur;
				continue;
			}
			break;
		}
		/*没有成功打开有效的音频文件*/
		if (n == info->achn_stat[chn_index].audio_nums) {
			SYSLOG(LOG_ERR, "all mp3 file cant open.");
			goto err;
		}
		info->achn_stat[chn_index].chn_tid = 0;
#ifdef  _DEBUG
	printf("--------channel_stat------\n");
	printf("chn_id:%d,descp:%s\n", chn_index, info->achn_stat[chn_index].ent.desc);
	printf("cur:%d, offset:%lld, fd:%d, audio_nums:%lu\n", 
		info->achn_stat[chn_index].cur,
		info->achn_stat[chn_index].offset,
		info->achn_stat[chn_index].fd,
		info->achn_stat[chn_index].audio_nums);
#endif
		if ((info->achn_stat[chn_index].ptbf = tbf_init(MP3_BITRATE, BURST_CACHE_SIZE)) == NULL) {
			SYSLOG(LOG_ERR, "tbf_init failed.");
			goto err;
		}

		chn_index++;
	}

	info->chn_nums = chn_index;
	info->get_chn_list = get_chn_list;
	info->read_chn = read_chn;
	globfree(&glob_mdir);
	return 0;
err:
	globfree(&glob_mdir);
	return -1;
}
int upmedia_destroy(struct upmedia_st *info)
{
	return 0;
}
