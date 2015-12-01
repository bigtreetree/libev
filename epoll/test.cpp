#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
 
#define EPOLL_MAXEVENTS 64
 
int main(int argc, char *argv[])
{
    int fd, epfd, flags, status, ret, nevents, i;
	socklen_t slen;
    struct sockaddr_in addr;
    struct in_addr remote_ip;
    struct epoll_event ev, events[EPOLL_MAXEVENTS];;
 
 
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        perror("socket failed");
        return -1;
    }
 
    status = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &status, sizeof(int))) {
        perror("setsockopt failed");
        return -1;
    }
 
   flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "192.168.11.173", &addr.sin_addr) <= 0) {
        perror("inet_pton error");
        return -1;
    }
 
 
 
    ret = connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr));
    if (ret == 0) {
        printf("non-blocking connect success. connect complete immediately");
        close(fd);
        return -1;
    }
 
    if (ret < 0 && errno != EINPROGRESS) {
        perror("connect error!");
        return -1;
    }
 
 
    epfd = epoll_create(EPOLL_MAXEVENTS);
 ev.events = EPOLLOUT;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1 ) {
        perror("epoll_ctl error");
        goto finish;
    }
    printf("add connect fd into epoll");
 
    memset(events, 0, sizeof(events));
 
    for (;;) {
 
        nevents = epoll_wait(epfd, events, EPOLL_MAXEVENTS, -1);
 
        if (nevents < 0) {
            perror("epoll_wait failed");
            goto finish;
        }
 
        for (i = 0; i < nevents; i++) {
 
            if (events[i].data.fd == fd) {
 
				slen = sizeof(int);
				status = -3;
				int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&status, &slen);
				if(res < 0)
                {
                    perror("getsockopt error!");
                    goto finish;
                }
                if (status != 0) {
                    perror("connect error!");
                    goto finish;
                }
 
                printf("non-blocking connect success!");
 
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1 ) {
                    perror("epoll_ctl error");
                    return 0;
                }
 
                /* DO write... */
            }
        }
    }
 
 finish:
    close(fd);
    close(epfd);
 
    return 0;
}
