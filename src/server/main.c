#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <fcntl.h>
#include <proto.h>
#include <utils.h>
#include "server_conf.h"
#include <syslog.h>
#include <signal.h>
#include "upmedia.h"
#include <pthread.h>
#include <time.h>

/*1.初始化
	a. 配置
	b. 守护进程
	c. 日志
*/

struct srv_conf_st  srv_cfg =  {
	.rcv_port = DEFAULT_RCV_PORT,
	.mgroup   = DEFAULT_MGROUP,
	.media_path = DEFAULT_MEDIA_PATH,
	.run_daemon = DEFAULT_RUN_DAEMON,
	.nic_if	    = DEFAULT_NIC_IF,
};

static volatile int srv_shutdown = 0;
static struct upmedia_st *upmedia;
int srv_sock_fd;
struct sockaddr_in r_addr;
socklen_t  addr_len = sizeof(r_addr);
pthread_t chn_list_id;

int server_socket_init()
{
	int iret, sock_fd;
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (sock_fd < 0) {
		SYSLOG(LOG_ERR, "socket failed.(%s)", strerror(errno));
		goto err;
	}

	struct ip_mreqn  mreq;
        bzero(&mreq, sizeof(mreq));
        inet_pton(AF_INET, srv_cfg.mgroup, &mreq.imr_multiaddr);
        inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);
        mreq.imr_ifindex = if_nametoindex(srv_cfg.nic_if);

        iret = setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq));
	if (iret != 0) {
		SYSLOG(LOG_ERR, "setsockopt failed,(%s)", strerror(errno));
		goto err;
	}
#ifdef  _DEBUG
        int sw_loop_multicast = 1;
        /*可以收到本地发送的组播*/
        iret = setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &sw_loop_multicast, sizeof(sw_loop_multicast));
	if (iret != 0) {
		SYSLOG(LOG_ERR, "setsockopt failed,(%s)", strerror(errno));
		goto err;
	}
#endif

        bzero(&r_addr, sizeof(r_addr));
        r_addr.sin_family = AF_INET;
        r_addr.sin_port = htons(atoi(srv_cfg.rcv_port));
        inet_pton(AF_INET, srv_cfg.mgroup, &r_addr.sin_addr);

	srv_sock_fd = sock_fd;

	return 0;
err:
	return -1;
}


int run_daemon(void)
{

	pid_t p;
	p = fork();
	
	if (p < 0) {
		perror("fork");
		exit(-1);
	} else if (p > 0) {
		exit(0);
	}
	CHK_EXIT(setsid());
	CHK_EXIT(umask(0));
	CHK_EXIT(chdir("/"));	

	int i;
	for (i = getdtablesize(); i >= 0; --i) {
		close(i);
	}
	/*重定向 stdin stdout stderr 到 /dev/null */	
	int fd = open("/dev/null", O_RDWR);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
		
	return 0;	
}

static void sig_handler(int signo)
{
	switch (signo) {
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			srv_shutdown = 1;
			break;
		default:
			break;
	}
	//kill pthread
}

static int server_init(int argc, char *argv[])
{
	//配置

	//命令行参数设置	
#if 0
	struct option opt[] = {

	};
	int c;
	while ((c = getopt_long()) != -1) {
		switch (c) {
			case '':
				break;
			default:
				break;
		}
	}
#endif
	//守护进程
	if (srv_cfg.run_daemon) {
		run_daemon();
	}	
	//日志
	openlog(PROG_NAME, LOG_PERROR| LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "upRadio start.");

	//信号关联
	signal(SIGQUIT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	
	//加载upmdia
	upmedia = (struct upmedia_st *)malloc(sizeof(struct upmedia_st));
	if (upmedia == NULL) {
		SYSLOG(LOG_ERR, "malloc failed.(%s)", strerror(errno));
		goto err;
	}

	if (upmedia_init(upmedia) < 0) {
		SYSLOG(LOG_ERR, "upmedia_init failed.");
		goto err_upmedia_init;
	}

	if (server_socket_init() < 0) {
		SYSLOG(LOG_ERR, "server_sock_init failed.");
		goto err_sock_init;
	}

	return 0;

err_sock_init:
err_upmedia_init:
	free(upmedia);
err:
	return -1;
}

int server_destroy()
{
	upmedia_destroy(upmedia);
	free(upmedia);
	return 0;
}

void *thr_snd_chn_list(void *len)
{
	int n = (int)len;
	ssize_t iret;
	struct timespec t;

	while (1) {
		t.tv_sec = 1;
		t.tv_nsec = 0;
		while (nanosleep(&t, &t) != 0) {
			if (errno == EINTR) {
				continue;
			}
			SYSLOG(LOG_ERR, "nanosleep failed,(%s)", strerror(errno));
			return NULL;
		}
	
		iret = sendto(srv_sock_fd, upmedia->pchnlist, n, 0, (const struct sockaddr *)&r_addr, addr_len);
		//DEBUG("srv_sock_fd: %d,  pchnlist:%p, len: %d, (%s)%d\n", srv_sock_fd, upmedia->pchnlist, n, inet_ntoa(r_addr.sin_addr), addr_len);
		if (iret == -1) {
			SYSLOG(LOG_ERR, "sendto failed.(%s)", strerror(errno));
			goto err;
		}
		DEBUG("pthred_tid:%d,%s send(%ld)\n", pthread_self(), __FUNCTION__, iret);
	}

	return NULL;
err:
	return (void *)-1;
}


int thr_chn_list_create(int len)
{
	return pthread_create(&chn_list_id, NULL, thr_snd_chn_list, (void *)len);
}


void *thr_snd_data(void *i)
{
	int chn_id = (int)i;
	char buf[AUDIO_BUF_SIZE] = {0};
	struct  mesg_info_st *pmesg = (struct mesg_info_st *)malloc(MAX_PACK_SIZE);
        pmesg->chn_id = chn_id;
	int n, len, ret, pos;
	while (1) {
		n = upmedia->read_chn(upmedia, chn_id, buf, AUDIO_BUF_SIZE);
		if (n < 0) {
			SYSLOG(LOG_ERR, "read_chn failed.(%s)", strerror(errno));
			goto err;
		} else if (n == 0) {
			DEBUG("Next Audio.\n");
			continue;
		}
		pos = 0;
                while (n > 0) {
                        //取cps和n中的最小值，发包
                        len = tbf_get_token2(upmedia->achn_stat[chn_id].ptbf, n);
                        memcpy(pmesg->data, buf + pos, len);
                        if (len < 0) {
                                perror("memcpy");
                                goto err;
                        }
                        ret = sendto(srv_sock_fd, pmesg, len + 1, 0, (const struct sockaddr *)&r_addr, addr_len);
                        DEBUG("send:%d\n", ret);
                        pos += len;
                        n -= len;
                }
	}


	return NULL;
err:
	return (void *)-1;
}

int thr_chn_data_create()
{
	int i, iret;
	for (i = 1; i < upmedia->chn_nums; ++i) {
		iret = pthread_create(&upmedia->achn_stat[i].chn_tid, NULL, thr_snd_data, (void *)i);
		if (iret < 0) {
			SYSLOG(LOG_ERR, "pthread_create failed.(%s)", strerror(errno));
			return -1;
		}
	}

	return 0;
}

/*2. 频道的维护和访问统一有upmedia库提供*/

int main(int argc, char *argv[])
{
	/*初始化*/
	int n, iret;
	if (server_init(argc, argv) < 0) {
		SYSLOG(LOG_ERR, "server_init failed.");
		goto err;
	}
	
	int len;
	/*获得频道列表*/
	if (upmedia->get_chn_list(upmedia, &len) < 0) {
		SYSLOG(LOG_ERR, "get_chn_list failed.");
		goto err;
	}
	DEBUG("pchn_list:%p, %d\n", upmedia->pchnlist, sizeof(r_addr));
	/*1.开启报幕线程，发送频道列表信息*/
	iret = thr_chn_list_create(len);	
	
	if (iret < 0) {
		SYSLOG(LOG_ERR,"thr_chn_list_create failed.");
		goto err;
	}
	

	/*2.开启各个频道线程, 发送各自频道音频数据*/
	iret = thr_chn_data_create();
	if (iret < 0) {
                	SYSLOG(LOG_ERR,"thr_chn_list_create failed.");
                	goto err;
	}
	/*结束*/
	
	while (! srv_shutdown) {
		pause();
	}

	close(srv_sock_fd);
	syslog(LOG_INFO, "%s stoped.", PROG_NAME);
	return 0;
err:
	SYSLOG(LOG_ERR, "%s abort.", PROG_NAME);
	return -1;
}

