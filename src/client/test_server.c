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
	
        int sw_loop_multicast = 1;
        //加入到组播域
        struct ip_mreqn  mreq;
        bzero(&mreq, sizeof(mreq));
        inet_pton(AF_INET, "225.5.5.5", &mreq.imr_multiaddr);
        inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);
        mreq.imr_ifindex = if_nametoindex("eth0");
        /*加入指定组播组*/
        CHK_EXIT(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)));
        /*可以收到本地发送的组播*/
        CHK_EXIT(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &sw_loop_multicast, sizeof(sw_loop_multicast)));

	struct sockaddr_in r_addr;
	socklen_t  addr_len = sizeof(r_addr);
	
	 bzero(&r_addr, sizeof(r_addr));
        r_addr.sin_family = AF_INET;
        r_addr.sin_port = htons(2000);
        inet_pton(AF_INET, "225.5.5.5", &r_addr.sin_addr);	


	//2. 构造列表信息
	
	//3. 每秒发一次列表信息
	char buf[] = "hello, i am test server";
	int n;
	while (1) {
		n = sendto(sock_fd, buf, strlen(buf) + 1, 0, (const struct sockaddr *)&r_addr, addr_len);
		DEBUG("send(%d)%s\n", n, buf);
		sleep(1);	
	}

	
	//4. 结束
	close(sock_fd);

	return 0;
}
