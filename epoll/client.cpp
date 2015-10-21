#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>


using namespace std;
#define MAXLINE  800 
int main(int argc, char **argv)
 {
      int                    sockfd;
      char                recvline[MAXLINE + 1];
      struct sockaddr_in    servaddr;
  
      if (argc != 2)
	  {
	  	cout<<"usage: a.out <IPaddress>"<<endl;
		return 1;
	}
 
     if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	 	cout<<"socket error"<<endl;
		return 1;
	}
 
     bzero(&servaddr, sizeof(servaddr));
     servaddr.sin_family = AF_INET;
     servaddr.sin_port   = htons(5000);   
     if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	 	cout<<"inet_pton error for "<<argv[1]<<endl;
 
     if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	 {
	 	cout<<"connect error "<<endl;
		return 1;
	 }
 
     char input[100];
     while (fgets(input, 100, stdin) != NULL)
     {
         write(sockfd, input, strlen(input));
 
         int n = 0;
         int count = 0;
         while (1)
         {
             n = read(sockfd, recvline + count, MAXLINE);
             if (n == MAXLINE)
             {
                 count += n;
                 continue;
             }
             else 
                 break;
         }
         printf("%s\n", recvline);
     }
     exit(0);
}
