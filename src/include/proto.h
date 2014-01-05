#pragma once
#ifndef  __PROTO_H
#define  __PROTO_H

#include <stdint.h>
#define   PROG_NAME	"upRaido"
#define	  PROG_VER	"V0.0.1"
#define   PROG_AHTHOR	"Richard.xia8@gmail.com"

#define   DEFAULT_RCV_PORT	"2000"
#define   DEFAULT_MGROUP	"225.5.5.5"
#define   DEFAULT_PLAYER	"mpg123"

#define   NUM_CHNS	20
#define   MIN_CHN_ID	1
#define   MAX_CHN_ID	(MIN_CHN_ID + NUM_CHNS - 1)

/*专门用来发送频道列表信息的频道*/
#define   LIST_CHN_ID	0

#define   MAX_PACK_SIZE	(65536 - 20 - 8)


typedef   uint8_t  chnid_t;

/*收发信息结构体
@data: 变长数组技术，主要待发送的数据大小是未知的，所以采用变长数组，发送之前由使用者决定大小
   data[0],技术是GCC编译平台单独支持的技术，对于其他平台没有很好的兼容性，所以我们选择 data[1],有一个元素	
*/
/*之前Linux中默认会以CPU字长对齐，在网络中传输，没有这样的必要，故选择1字节对齐 */
#pragma pack(1)
struct  mesg_info_st {
	chnid_t chn_id;
	uint8_t data[1];		
};

/*频道列表的条目*/
struct chnlist_entry_st {
	chnid_t chn_id;
	size_t len; /*desc字符串的长度*/
	char *desc[1];
};

/*频道列表*/
struct  chnlist_info_st {
	chnid_t chn_id;
	struct chnlist_entry_st data[1];
};
#pragma pack(0)
/* 恢复为默认对齐方式 */


#endif
