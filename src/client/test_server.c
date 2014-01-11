#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <proto.h>
#include <utils.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <arpa/inet.h>
#include <net/if.h>

int main()
{
	//1.初始化
	int sock_fd;
	CHK_RET_EXIT(sock_fd, socket(AF_INET, SOCK_DGRAM, 0));
	
	//组播
	
        //开启组播，指定 IP_MULTICAST_IF
        struct ip_mreqn  mreq;
        bzero(&mreq, sizeof(mreq));
        inet_pton(AF_INET, "225.5.5.5", &mreq.imr_multiaddr);
        inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);
        mreq.imr_ifindex = if_nametoindex("eth0");
        CHK_EXIT(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)));
#ifdef  _DEBUG
        int sw_loop_multicast = 1;
        /*可以收到本地发送的组播*/
        CHK_EXIT(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &sw_loop_multicast, sizeof(sw_loop_multicast)));
#endif
	struct sockaddr_in r_addr;
	socklen_t  addr_len = sizeof(r_addr);
	
	 bzero(&r_addr, sizeof(r_addr));
        r_addr.sin_family = AF_INET;
        r_addr.sin_port = htons(2000);
        inet_pton(AF_INET, "225.5.5.5", &r_addr.sin_addr);	


	//2. 构造列表信息
	struct chnlist_info_st *plist;
	plist  = (struct chnlist_info_st *)calloc(1, MAX_PACK_SIZE);
	
	if (plist == NULL) {
		perror("malloc");
		exit(1);
	}
	
	char *list[] = { "Jazz", "Pop", "Country", "Rock", "Children"};
	plist->chn_id = LIST_CHN_ID;
	
	struct chnlist_entry_st *pentry = plist->entry;
	int i, count = 1, n;
	for ( i = 0; i < (sizeof(list) / sizeof(list[0])); ++i) {
		pentry->chn_id = i + 1;
		n = strlen(list[i]) + 1;
		pentry->len = htons(n);
		strncpy(pentry->desc, list[i], n);
		count += n + 3;
		DEBUG("chn_id:%d,chn_len:%hu,desc:%s,count:%d\n",
			pentry->chn_id, ntohs(pentry->len),
			pentry->desc, count);
		pentry = (struct chnlist_entry_st *)((char *)pentry + n + 3);
	}

	DEBUG("count = %d\n", count);	
	//3. 每秒发一次列表信息
	//char buf[] = "hello, i am test server";
	i = 2;
	while (i-- > 0) {
		n = sendto(sock_fd, plist, count, 0, (const struct sockaddr *)&r_addr, addr_len);
		DEBUG("send(%d)\n", n);
		sleep(1);	
	}

	sleep(5);

	//4. 发出指定数据给客户端组播组
	char buf[] = "hello, i am test server";
	struct  mesg_info_st *pmesg = (struct mesg_info_st *)malloc(MAX_PACK_SIZE);
	pmesg->chn_id = 2;
	strncpy((char *)(pmesg->data), buf, sizeof(buf));
	while (1) {
		n = sendto(sock_fd, pmesg, sizeof(buf) + 1, 0, (const struct sockaddr *)&r_addr, addr_len);
		DEBUG("send(%d)%s\n", n, (char *)pmesg);
		sleep(1);	
	}
 
	
	//5. 结束
	close(sock_fd);

	return 0;
}
