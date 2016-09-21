/*
 * server.cpp
 *
 *  Created on: Oct 13, 2015
 *      Author: prerna
 */

/*
 * server_UDP.c
 *
 *  Created on: Oct 13, 2015
 *      Author: prerna
 */

/*
 * udpServer.c
 *
 *  Created on: Sep 20, 2015
 *      Author: prerna
 */

/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
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

int file_sz_gl;
int MAX=1300;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

char* getfile(char* fl_nm)/*this function retrives the file requested by client and responds back to the client.*/
{
	FILE *fp;
	int n;
	int newsockfd1;
	int file_size;
	char *fl_inside;
	char *file_name;

	file_name = strndup(fl_nm+1,strlen(fl_nm));
	fp=fopen(file_name,"r");
	if(fp==NULL){
		cout<<"404 Not Found\n";
		//strcpy(fl_inside,"HTTP/1.1 404 Not Found\nContent-Type: text/html; charset=utf-8\n Content-Length:0\n");
		return fl_inside;
	} else{
		fseek(fp,0,SEEK_END);
		file_size=ftell(fp);
		file_sz_gl=file_size;
		cout<<"The file size is:"<<file_sz_gl;
		fseek(fp,0,SEEK_SET);
		fl_inside=(char *)calloc(1,file_size);
		if(fl_inside){
			fread(fl_inside,file_size,1,fp);
		}
		/*puts(fl_inside);*/
		fclose(fp);
		return fl_inside;
	}

}

struct http_hdr { char rqst_type[20];
char rqst_file[200];
char rqst_http[20];
char con_type[50];
};
void parse_http(char buffer[],struct http_hdr *hdr){/*this function parses the request received by client and gets the filename,request type and HTTP protocol*/
	char *tok;
	char *flag;
	char *cur;
	int flag_p=0;


	tok=strtok(buffer,"\r\n");
	while(tok!=NULL)
	{
		flag=strstr(tok,"GET");
		if (flag!=NULL)
		{
			sscanf(tok,"%s %s %s",hdr->rqst_type,hdr->rqst_file,hdr->rqst_http);
		}
		tok=strtok(NULL,"\r\n");
	}


}



int main(int argc, char *argv[])
{
	int sockfd, length, portno;
	int n_pckt;
	int file_size;
	int if_p=0;
	int flag=1;
	int rcvd_bytes;
	char *fl_inside;
	char *file_name;
	struct http_hdr hdr;/*this contains the request type,file name requested and protocol*/
	FILE *fp=NULL;
	socklen_t fromlen;
	char buf[1400];
	char res_hdr[200];
	int i=0;
	char *file_sz_char;
	char* packet=new char[MAX];
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	int n;
	time_t t;
	time(&t);


	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) error("Error Opening socket");
	length = sizeof(serv_addr);
	bzero(&serv_addr,length);
	bzero(&cli_addr,length);
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(atoi(argv[1]));
	if (bind(sockfd,(struct sockaddr *)&serv_addr,length)<0)
		error("binding error");
	fromlen = sizeof(struct sockaddr_in);
	while (1)
	{                            /*server listens forever*/
		n = recvfrom(sockfd,buf,sizeof(buf),0,(struct sockaddr *)&cli_addr,&fromlen);/*receives request from client*/
		if (n < 0) error("error in recvfrom");
		write(1,"Received a request datagram:",30);
		write(1,buf,n);

		parse_http(buf,&hdr);                             /*parse the client request*/
		cout<<"Request type is:"<<hdr.rqst_type<<"\nRequest file is:"<<hdr.rqst_file+1<<"\nHttp version is:"<<hdr.rqst_http;
		fl_inside=getfile(hdr.rqst_file);
		/*if file size is more then break it in packets*/
		int k,j;
		n_pckt=floor(file_sz_gl/MAX)+1;
		cout<<"File siz in main is:"<<file_sz_gl<<endl;
		cout<<"MAX in main is:"<<MAX<<endl;
		cout<<"number of packets formed is"<<n_pckt;

		for(i=0;i<n_pckt;i++)
		{
			for(j=i*MAX,k=0;j<(i+1)*MAX && k<MAX;j++,k++)
			{
				packet[k]=fl_inside[j];

			}

			packet[k]='\0';
			cout<<endl<<"packet is"<<packet<<endl<<endl;
			strcpy(buf,packet);
			n = sendto(sockfd,buf,MAX, 0,(struct sockaddr *)&cli_addr,fromlen);
			if (n < 0)
				error("ERROR in sendto");
		}
		cout<<"last byte transmitted at time:"<<ctime(&t)<<endl;
		n = sendto(sockfd,0,0, 0,(struct sockaddr *)&cli_addr,fromlen);
		close(sockfd);
		return 0;

	}


}








