#pragma once
#include <pthread.h>

struct tbf_st {
	int cps; //每秒字节速率
	int burst;//桶的容量
	int tokens; //桶中的令牌数
	pthread_mutex_t mutex; //互斥锁
	pthread_cond_t  cond; //条件变量
};

//构造
struct tbf_st *tbf_init(int cps, int burst);
//析构
int tbf_destroy(struct tbf_st *info);
//申请n字节令牌, 如果桶中令牌不足，可阻塞直到获得指定的令牌数
int tbf_get_token(struct tbf_st *tbf);
int tbf_get_token2(struct tbf_st *tbf, int n);
//返还n字节令牌
int tbf_return_token(struct tbf_st *tbf, int n);
