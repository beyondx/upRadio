#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <proto.h>
#include <utils.h>
#include <strings.h>
#include "client_conf.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/stat.h>

static struct client_conf_st  cli_conf;
/*
  -p --port: port
  -P --player: Player
  -m --mgroup: multicase address
  -h --help : help
*/
void usage(const char *prgname)
{
	printf("Usage: %s [-p port| -m mgroup| -P player | -h help]\n",	prgname);
}



int client_init(int argc, char *const argv[])
{
	/*配置
		1. 默认设置
		2. 配置文件 *
		3. 命令参数
	*/
	/*默认设置*/
	cli_conf.recv_port = DEFAULT_RCV_PORT; 
	cli_conf.mgroup = DEFAULT_MGROUP; 
	cli_conf.player = DEFAULT_PLAYER;
	
	struct option  cli_option[] = {
		{ "port", required_argument, NULL, 'p'},
		{ "player", required_argument, NULL, 'P'},
		{ "mgroup", required_argument, NULL, 'm'},
		{ "help", no_argument, NULL, 'h'},
	};
	
	/*命令行参数*/
	int c;
	
	while ((c = getopt_long(argc, argv, "p:P:m:h", cli_option, NULL)) != -1) {
		switch (c) {
			case 'p':
				cli_conf.recv_port = optarg;
				break;
			case 'P':
				cli_conf.player = optarg;
				break;
			case 'm':
				cli_conf.mgroup = optarg;
				break;
			case 'h':
			case '?':
				usage(argv[0]);
				exit(0);
				break;
			default:
				fprintf(stderr, "ERROR: unkown opt '%c'\n", c);
				exit(-1);
				break;
		}
	}
#ifdef  _DEBUG
	printf("port: %s\n", cli_conf.recv_port);
	printf("mgroup: %s\n", cli_conf.mgroup);
	printf("player: %s\n", cli_conf.player);
#endif
	
	/*网络连接*/
	int sock_fd;
	
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (sock_fd < 0) {
		perror("socket");
		exit(-1);
	}
	
	struct sockaddr_in l_addr;
	socklen_t  addr_len = sizeof(l_addr);

	int sw_loop_multicast = 1;
	//加入到组播域
	struct ip_mreqn  mreq;
	bzero(&mreq, sizeof(mreq));
	inet_pton(AF_INET, cli_conf.mgroup, &mreq.imr_multiaddr);
	inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);
	mreq.imr_ifindex = if_nametoindex("eth0");
	/*加入指定组播组*/
	CHK_EXIT(setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)));
	/*可以收到本地发送的组播*/
	CHK_EXIT(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &sw_loop_multicast, sizeof(sw_loop_multicast)));

	
	bzero(&l_addr, sizeof(l_addr));
	l_addr.sin_family = AF_INET;
	l_addr.sin_port = htons(atoi(cli_conf.recv_port));
	inet_pton(AF_INET, "0.0.0.0", &l_addr.sin_addr);	
	
	CHK_EXIT(bind(sock_fd, (const struct sockaddr *)&l_addr, addr_len));
	DEBUG("bind ok, sock_fd: %d, listen: %s\n", sock_fd, cli_conf.recv_port);
	
	return sock_fd;
}

int show_chn_list(const struct  chnlist_info_st *plist, size_t pack_size)
{
	struct chnlist_entry_st *pentry = 
		(struct chnlist_entry_st *)(plist->entry);
	
	uint16_t len = 0; 
	printf("----chn_list_info-----\n");
	for(; pentry < (struct chnlist_entry_st *)((char *)plist + pack_size);) {
		len = ntohs(pentry->len);
		printf("chnid:%d,chn_len:%hu,chn_desc:%s\n",
			pentry->chn_id, ntohs(pentry->len),
			pentry->desc);
		pentry = (struct chnlist_entry_st *)
				((char *)pentry + len + 3);
	}
	printf("----chn_list_info-----\n");
	return 0;
}


int main(int argc, char *argv[])
{
	/*初始化*/
	int sock_fd, n;
	struct sockaddr_in r_addr, srv_addr;
	socklen_t addr_len = sizeof(r_addr);

	CHK_RET_EXIT(sock_fd, client_init(argc, argv));

	char buf[MAX_PACK_SIZE] = {0};
	
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, DEFAULT_SERVER_IP, &srv_addr.sin_addr);


	/*拉去频道列表，选择频道号*/
	bzero(&r_addr, addr_len);

	while (1) {
		/*收包*/
		n = recvfrom(sock_fd, buf, sizeof(buf) - 1, 0, 
			(struct sockaddr *)&r_addr, &addr_len);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			}
			perror("recvfrom");
			break;
		}
		//DEBUG("buf:%s\n", buf);	
#if 1
		struct  chnlist_info_st *plist = (struct  chnlist_info_st *)buf;

		if (plist->chn_id == LIST_CHN_ID) {
			/*展示频道信息*/
			DEBUG("recv:chnlist_info package,(%d)\n", n);
			show_chn_list(plist, n);	
			break;
		}
#endif
	}
	
	/*选择频道号*/
	int chose_id = 2;
	#if 0
	printf("chose_id:");
	fflush(stdout);
	n = scanf("%d", &chose_id);
	if (n != 1) {
		perror("scanf");
		exit(1);
	}
	#endif
	printf("your chose chanel:%d\n", chose_id);
	
	int pipe_fd[2];

	pipe(pipe_fd);

	pid_t pid = fork();

	if (pid == 0) {
		close(pipe_fd[1]);
		dup2(pipe_fd[0], STDIN_FILENO);
		close(pipe_fd[0]);

		execlp("mpg123", "mpg123", "-q", "-", NULL);
		exit(0);
	}

	/*收到指定频道数据*/
	while (1) {
		/*收包*/
		n = recvfrom(sock_fd, buf, sizeof(buf) - 1, 0, 
			(struct sockaddr *)&r_addr, &addr_len);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			}
			perror("recvfrom");
			break;
		}
		
		struct mesg_info_st *pmesg = (struct mesg_info_st *)buf;
		/*判断是否为指定ID的消息*/
		if (pmesg->chn_id != chose_id) {
			continue;
		}
		DEBUG("chn_id:%d\n", pmesg->chn_id);
		#if 0
		/*判断是否从服务器中发出的消息*/
		if (r_addr.sin_addr.s_addr != srv_addr.sin_addr.s_addr) {
			fprintf(stderr,"收到非发主机的组播信息,from '%s:%hu'\n", inet_ntoa(r_addr.sin_addr), ntohs(r_addr.sin_port));
			continue;
		}
		#endif
		DEBUG("(%d)%s\n", n, (char *)(pmesg->data));
#if 1
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		write(STDOUT_FILENO, pmesg->data, n - 1);		
		/*关闭*/
#endif
	}
	wait(NULL);

	return 0;
}
