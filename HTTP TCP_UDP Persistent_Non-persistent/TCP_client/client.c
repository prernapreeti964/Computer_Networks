/*
 * client.c
 *
 *  Created on: Oct 14, 2015
 *      Author: prerna
 */

/*
 * webclient.c
 *
 *  Created on: Oct 13, 2015
 *      Author: prerna
 */

/*
 * client.c
 *
 *  Created on: Sep 20, 2015
 *      Author: prerna
 */

/*the client program*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<math.h>

struct timeval t1,t2,tv;

void error(const char *msg)/*standard error function*/
{
	perror(msg);
	exit(0);
}


void send_req(int sockfd1,char *fl_nm,char *conc)/*this function generates the GET request by taking file name and connection type as parameters and sends it to server*/
{
	char con_type_code[20]={0};
	int n,len,num=0;
	long cnt=1;
	char contentLength[7]={0};
	char *tok=NULL;
	char *flag=NULL;
	FILE *fp1=NULL;
	int bk = 1;
	fp1=fopen(fl_nm,"w");
	char rbuff[3056000],sbuff[3056],tbuff[32001];
	if(strncmp(conc,"np",strlen(conc))==0)
	{
		strcpy(con_type_code,"close");
	}
	else if(strncmp(conc,"p",strlen(conc))==0)
	{
		strcpy(con_type_code,"keep-alive");
	}
	else
	{
		printf("wrong code entered for connection type");
		exit(0);
	}
	bzero(sbuff,3056);

	sprintf(sbuff,"GET /%s HTTP/1.1\r\nHost: \r\nConnection:%s\r\n\r\n", fl_nm, con_type_code);
	printf("%s",sbuff);


	gettimeofday(&t1,NULL);
	n = send(sockfd1, sbuff, strlen(sbuff),0);

	if (n < 0) {
		error("ERROR writing to socket");
		exit(0);
	}

	bzero(rbuff,3056000);
	n = read(sockfd1,rbuff,1000);
	if (n < 0)
		error("ERROR reading from socket");
	gettimeofday(&t2,NULL);
	printf("%s",rbuff);
	fprintf(fp1,"%s",rbuff);
	strcpy(tbuff,rbuff);
	tok=strtok(tbuff,"\r\n");
	while(tok!=NULL)
	{
		flag=strstr(tok,"Content-Length:");
		if(flag!=NULL)
		{
			sscanf(tok,"Content-Length:%s",contentLength);
		}
		tok=strtok(NULL,"\r\n");
	}

	len=atoi(contentLength);
	while(n<=len)
	{
		bzero(rbuff,1000);
		n+=read(sockfd1,rbuff,1000);
		printf("%s",rbuff);
		fprintf(fp1,"%s",rbuff);
	}

	fclose(fp1);
	tv.tv_sec=t2.tv_sec-t1.tv_sec;
	tv.tv_usec=t2.tv_usec-t1.tv_usec;
	printf("Time elapsed is :%d seconds and %d microseconds\n",tv.tv_sec,tv.tv_usec);
	sleep(3);
}

int main(int argc, char *argv[])
{
	int sockfd, portno,n; /*socket file descriptors*/
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char con_type[4],file_nm[100];

	if (argc < 5) {/*abort if all arguments are not passed*/
		fprintf(stderr,"usage %s hostname port conn_type filename\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	strcpy(con_type,argv[3]);
	puts(con_type);
	strcpy(file_nm,argv[4]);
	puts(file_nm);


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
		exit(0);
	}
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR in connecting");
		exit(0);
	}
	if(strncmp(con_type,"np",strlen(con_type))==0)/*send request for non persistent client*/
	{

		send_req(sockfd,file_nm,con_type);
	}

	else if(strncmp(con_type,"p",strlen(con_type))==0)/*keep sending requests till close request in case of persistent client*/
	{
		FILE *fp;
		FILE *fp1;
		int file_size;
		char line[200];
		int line_cnt=0,l=1;

		fp1=fopen(file_nm,"r");
		while (fgets(line,20,fp1)) /*keep sending request from the file list */
		{
			line_cnt++;

		}
		printf("Line count is:%d\n",line_cnt);

		fclose(fp1);
		fp=fopen(file_nm,"r");
		if(fp==NULL){
			printf("File Not Found on client side\n");
			exit(0);
		}
		else{

			while (l<=line_cnt) /*keep sending request from the file list */
			{
				fgets(line,200,fp);
				strcpy(con_type,"p");
				if(l==line_cnt)strcpy(con_type,"np");
				printf("sending request for file %d\n",l);

				strtok(line,"\n");
				send_req(sockfd,line,con_type);
				l++;
			}




		}
		close(sockfd);


	}
	return 0;
}






