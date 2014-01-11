#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <proto.h>
#include <utils.h>
#include "server_conf.h"
#include <syslog.h>
#include <signal.h>
/*1.初始化
	a. 配置
	b. 守护进程
	c. 日志
*/

struct srv_conf_st  srv_cfg =  {
	.snd_port = DEFAULT_SND_PORT,
	.mgroup   = DEFAULT_MGROUP,
	.path     =  DEFAULT_PATH,
	.run_daemon = DEFAULT_RUN_DAEMON,
};

static volatile int srv_shutdown = 0;


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
	openlog("upRadio", LOG_PERROR| LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "upRadio start.");

	//信号关联
	signal(SIGQUIT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	

	return 0;
}


/*2. 频道的维护和访问统一有upmedia库提供*/

int main(int argc, char *argv[])
{
	/*初始化*/
	int n, iret;
	CHK_EXIT(server_init(argc, argv));
	
	/*获得频道列表*/

	/*1.开启报幕线程，发送频道列表信息*/

	/*2.开启各个频道线程, 发送各自频道音频数据*/

	/*结束*/
	
	while (! srv_shutdown) {
		pause();
	}

	syslog(LOG_INFO, "%s stoped.", PROG_NAME);
	return 0;
}

