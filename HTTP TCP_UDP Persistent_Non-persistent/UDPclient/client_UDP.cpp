/*
 * client_UDP.
 *
 *  Created on: Oct 13, 2015
 *      Author: prerna
 */

/*
 * udpclient.c
 *
 *  Created on: Sep 20, 2015
 *      Author: prerna
 */
/* UDP client in the internet domain */
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <math.h>
#include <cmath>
#include <netdb.h>
#include <sys/time.h>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int main(int argc, char *argv[])
{
   int sock, n,count=0,bytes=0,packet=0;
   socklen_t length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   char buffer[1300],fl_nm[100];
   ofstream outputfile;
   struct timeval t1,t2,tv;

   if (argc < 4) {/*break the program if arguments are not received*/
       fprintf(stderr,"usage %s hostname port filename\n", argv[0]);
       exit(0);
    }
   strcpy(fl_nm,argv[3]);

   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("socket");

   server.sin_family = AF_INET;
   hp = gethostbyname(argv[1]);
   if (hp==0) error("Unknown host");

   bcopy((char *)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
   server.sin_port = htons(atoi(argv[2]));
   length=sizeof(struct sockaddr_in);
   bzero(buffer,256);
   cout<<"Sending the get request for file:"<<fl_nm<<endl;
   sprintf(buffer,"GET /%s HTTP/1.1\r\nHost:localhost", fl_nm);
   printf("%s",buffer);
   gettimeofday(&t1,NULL);
   n=sendto(sock,buffer,256,0,(const struct sockaddr *)&server,length);
   if (n < 0) error("Sendto error");
   n=1;
   outputfile.open("Downloaded_UDP.txt");
   while(n>0){                           /*keep receving the packets till all are received*/
   bzero(buffer,1300);
   n=recvfrom(sock,buffer,1300,0,(struct sockaddr *)&from, &length);
   if (n < 0) error("recvfrom error");
   outputfile<<buffer;
   bytes=bytes+n;
   packet=packet+1;
   cout<<"\nGot a packet :\n";

   cout<<"The total number of bytes received is:"<<bytes-1<<endl;
   cout<<"The total number of packets received is:"<<packet<<endl;



   printf("\n");

   }
   gettimeofday(&t2,NULL);
   tv.tv_sec=t2.tv_sec-t1.tv_sec;
   	tv.tv_usec=t2.tv_usec-t1.tv_usec;
   	printf("Time elapsed is :%d seconds and %d microseconds\n",tv.tv_sec,tv.tv_usec);
   	sleep(3);
   close(sock);
   return 0;
}



