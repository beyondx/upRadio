#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <proto.h>
#include <time.h>
#include "tbf.h"

static struct tbf_st *atbf[CHN_NUMS];

pthread_mutex_t tbf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid_timer;
pthread_once_t once_control = PTHREAD_ONCE_INIT;
static int get_index(void)
{
	int i;
	for (i = 0; i < sizeof atbf / sizeof atbf[0]; ++i) {
		if (atbf[i] == NULL) {
			return i;
		}
	}

	return -1;
}

static void *thr_timer_token(void *unused)
{
	struct timespec t;
	
	while (1) {
		/*sleep 不能使用，应为其底层是使用了SIGALRM信号，如果其他函数使用了alarm 函数就可能唤醒sleep，导致sleep失效*/
		//sleep(1);
		t.tv_sec = 1;
		t.tv_nsec = 0;
		while (nanosleep(&t, &t) != 0) {
			if (errno == EINTR) {
				continue;
			}
			syslog(LOG_ERR,"nanosleep failed(%s)", strerror(errno));
			goto err;
		}
		int i;
		for (i = 0; i < sizeof atbf / sizeof atbf[0]; ++i) {
			if (atbf[i] != NULL) {
				atbf[i]->tokens += atbf[i]->cps;
				if (atbf[i]->tokens > atbf[i]->burst) {
					atbf[i]->tokens = atbf[i]->burst;
				}
				//一旦令牌添加成功，就广播通知对方解除阻塞
				pthread_cond_broadcast(&atbf[i]->cond);
			}
		}
	}
	return NULL;
err:
	return (void *)-1;
}

void module_load(void)
{
	pthread_create(&tid_timer, NULL, thr_timer_token, NULL);
}

void module_unload(void)
{
	pthread_cancel(tid_timer);

	pthread_join(tid_timer, NULL);
}


struct tbf_st *tbf_init(int cps, int burst)
{
	struct tbf_st *me;

	pthread_once(&once_control, module_load);
	
	me = (struct tbf_st *)malloc(sizeof(*me));
	
	if (me == NULL) {
		syslog(LOG_ERR, "malloc failed: %s", strerror(errno));
		return NULL;
	}

	me->cps = cps;
	me->burst = burst;
	me->tokens = 0;
	//增加多线程互斥锁,避免不同线程同时执行get_index导致下标获取重复
	pthread_mutex_lock(&tbf_mutex);	
	//临界区域
	int index = get_index();
	if (index == -1) {
		syslog(LOG_ERR, "Out of boud of array atbf");
		pthread_mutex_unlock(&tbf_mutex);
		free(me);
		return NULL;
	}
	atbf[index] = me;
	//临界区域结束
	pthread_mutex_unlock(&tbf_mutex);

	pthread_mutex_init(&me->mutex, NULL);
	pthread_cond_init(&me->cond, NULL);
	//挂载卸载函数到exit
	atexit(module_unload);
	
	return me;
}

int tbf_destroy(struct tbf_st *ptbf)
{
	pthread_mutex_destroy(&ptbf->mutex);
	pthread_cond_destroy(&ptbf->cond);

	free(ptbf);

	int i;
	for (i = 0; i < sizeof atbf / sizeof atbf[0]; ++i) {
		if (atbf[i] == ptbf) {
			atbf[i] = NULL;
			return 0;
		}
	}
	syslog(LOG_ERR, "ptbf not found in atbf");

	return -1;
}

int min(int a, int b)
{
	return ((a) < (b) ? (a) : (b));
}


int tbf_get_token(struct tbf_st *ptbf)
{
	pthread_mutex_lock(&ptbf->mutex);

	if (ptbf->tokens <= 0) {
		//阻塞直到有更多令牌
		pthread_cond_wait(&ptbf->cond, &ptbf->mutex);
	}
	//取两者的最小值	
	int token = min(ptbf->tokens, ptbf->cps);
	//桶中令牌数,减去指定请求数
	ptbf->tokens -= token;

	pthread_mutex_unlock(&ptbf->mutex);

	return token;
}

int tbf_get_token2(struct tbf_st *ptbf, int n)
{
	pthread_mutex_lock(&ptbf->mutex);

	if (ptbf->tokens <= 0) {
		//阻塞直到有更多令牌
		pthread_cond_wait(&ptbf->cond, &ptbf->mutex);
	}
	//取两者的最小值	
	int token = min(ptbf->tokens, n);
	//桶中令牌数,减去指定请求数
	ptbf->tokens -= token;

	pthread_mutex_unlock(&ptbf->mutex);

	return token;
}
int tbf_return_token(struct tbf_st *ptbf, int n)
{
	pthread_mutex_lock(&ptbf->mutex);

	//桶中令牌数,增加指定请求数
	ptbf->tokens += n;

	if (ptbf->tokens > ptbf->burst) {
		ptbf->tokens = ptbf->burst;
	}
	//广播通知有新的令牌
	pthread_cond_broadcast(&ptbf->cond);

	pthread_mutex_unlock(&ptbf->mutex);

	return ptbf->tokens;
}
