/*
 * server_TCP.c
 *
 *  Created on: Oct 13, 2015
 *      Author: prerna
 */




/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void error(const char *msg)/*this function reports the standard error and aborts the program*/
{
	perror(msg);
	exit(1);
}

struct http_hdr { char rqst_type[20];
char rqst_file[200];
char rqst_http[20];
char con_type[50];
};

void process_new_thread(newsockfd)
          {
               char buffer[500]={0};
                int n=0;
                int if_p=0;
                int alive=1;
                struct http_hdr hdr;

		bzero(buffer,500);

		n = recv(newsockfd,buffer,500,0);
		printf("this is the request rcvd from client:\n%s\n\n", buffer);
		if (n < 0) {
			error("ERROR reading from socket");
			close(newsockfd);

		}
		if_p=parse_http(buffer,&hdr,newsockfd);/*parse the header of client request and check if it is persistent or non persistent*/

		/*if ( strcmp( hdr.rqst_http, "HTTP/1.0")==0 || strcmp( hdr.rqst_http, "HTTP/1.1")==0 )
                 {
                     printf("400:Bad Request\n");
                     exit(0);
                 }*/
		if(if_p==0)
		{

                getfile(hdr.rqst_file,newsockfd,if_p);
                close(newsockfd);


		}

		else if(if_p==1){/*if client is persistent keep reading the files and responding till close request received*/
                        while(alive){

                                    getfile(hdr.rqst_file,newsockfd,if_p);
                                    bzero(buffer,500);

                                    n = recv(newsockfd,buffer,500,0);
                                    printf("This is the request received from client:\n%s\n\n", buffer);
                                    if_p=parse_http(buffer,&hdr,newsockfd);

                                    if(if_p==0){
                                    	       alive=0;
                                               break;
                                               }
                                    }

                 getfile(hdr.rqst_file,newsockfd,if_p);
                 close(newsockfd);

		}
	   }
void getfile(char* fl_nm,int newsockfd,int if_p_np)/*this function retrieves the file requested from client and responds back with HTTP response header and the file requested*/
{
	FILE *fp=NULL;
	int n;
	char *final=NULL;
        int if_p_np1;
	int newsockfd1;
	int file_size;
	char *fl_inside;
	char *file_name;

        char line1[200] = {0};
        char line2[200] = {0};
        char line3[200] = {0};
        char line4[200] = {0};
        char file_name_buf[50]={0};
        char finalline[1000]={0};
	newsockfd1=newsockfd;
        if_p_np1=if_p_np;

	file_name = strndup(fl_nm+1,strlen(fl_nm));
        strcpy(file_name_buf,file_name);
	fp=fopen(file_name,"rb");
        printf("file name to send is :%s\n",file_name_buf);
	if(fp==NULL){/*display 404 not found if file is not in server*/

                strcat(line1, "HTTP/1.1 404 Not Found\n");
                strcat(line2, "Content-Type: text/html; charset=utf-8\n Connection: close\n Content-Length:24");
                strcat(line3, "<html><title>FileNotFound</title>\n <body>\n 404, FileNotFound \n </body>\n </html>\n");

                strcat(finalline, line3);
		printf("%s \n\n",finalline);
		n = send(newsockfd,finalline, strlen(finalline),0);
		if (n < 0)
							{
								error("ERROR writing to socket");
								exit(1);
							}
	}

                else if((fp!=NULL)&&(if_p_np1==1)){/*if file is found on server and client is persistent respond back with header and file*/
                	fseek(fp,0,SEEK_END);/*getting the size of requested file*/
                			file_size=ftell(fp);
                			printf("the file size is:%d\n",file_size);
                			fseek(fp,0,SEEK_SET);
                			fl_inside=(char *)calloc(1,file_size);
                			if(fl_inside){
                				fread(fl_inside,file_size,1,fp);
                			}
                			//puts(fl_inside);
                			fclose(fp);


                strcat(line1, "HTTP/1.1 200 OK\n");
                strcat(line2, "Content-Type: text/html; charset=utf-8\n");
                strcat(line3, "Connection: keep-alive\n");
                sprintf(line4,"Content-Length:%d\n\n",file_size);

                strcat(finalline, line1);
                strcat(finalline, line2);
                strcat(finalline, line3);
                strcat(finalline, line4);
                final=(char *)calloc(1,file_size+strlen(finalline));

                strcpy(final,finalline);
                strcat(final,fl_inside);


                n = send(newsockfd1,final,strlen(final),0);
                					if (n < 0)
                					{
                						error("ERROR writing to socket");
                						exit(1);
                					}
	  }

        else if((fp!=NULL)&&(if_p_np1==0)){/*if requested file is found and client is not persistent send back the response*/
        	fseek(fp,0,SEEK_END);
        			file_size=ftell(fp);
        			printf("the file size is:%d\n",file_size);//%ld\n",file_size);
        			fseek(fp,0,SEEK_SET);
        			fl_inside=(char *)calloc(1,file_size);
        			if(fl_inside){
        				fread(fl_inside,file_size,1,fp);
        			}

        			fclose(fp);
                strcat(line1, "HTTP/1.1 200 OK\n");
                strcat(line2, "Content-Type: text/html; charset=utf-8\n");
                strcat(line3, "Connection:close\n");
                sprintf(line4,"Content-Length:%d\n\n",file_size);

                strcat(finalline, line1);
                strcat(finalline, line2);
                strcat(finalline, line3);
                strcat(finalline, line4);


                final=(char *)calloc(1,file_size+strlen(finalline));

                                strcpy(final,finalline);
                                strcat(final,fl_inside);

                n = send(newsockfd1,final, strlen(final),0);
                					if (n < 0)
                					{
                						error("ERROR writing to socket");
                						exit(1);
                					}

                					printf("bytes sent: %d\n",n);
	}

}


int  parse_http(char buffer[],struct http_hdr *hdr,int socket){
	char *tok;
	char *flag;
	char *cur;
	int flag_p=0;
	int j;
	char line[200] = {0};

	tok=strtok(buffer,"\r\n");
	flag=strstr(tok,"GET");
	if (flag==NULL)
					{
						strcat(line, "HTTP/1.1 Bad request\n");
									printf("Bad request from the client\n\n");
									j = send(socket,line,200,0);
														if (j < 0)
														{
															error("ERROR writing to socket");
															exit(1);
														}
												exit(1);
					}

	while(tok!=NULL)
	{
		flag=strstr(tok,"GET");
		if (flag!=NULL)
				{
					sscanf(tok,"%s %s %s",hdr->rqst_type,hdr->rqst_file,hdr->rqst_http);
				}






		cur=strstr(tok,"Connection");

		if(cur!=NULL)
		{
			sscanf(tok,"Connection:%s",hdr->con_type);
		}
		tok=strtok(NULL,"\r\n");
	}

	if(strncmp(hdr->con_type,"keep-alive",strlen(hdr->con_type))==0)
		flag_p=1;
	else flag_p=0;

	return flag_p;

}


int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;                                         /*declare socket descriptors*/
	int file_size;
	pthread_t arr_thread[1000];
	int thread_count=0;
	int flag=1;
	int j=0;

	char *fl_inside;
	char *file_name;
	struct http_hdr hdr;
	FILE *fp=NULL;
	socklen_t clilen;
	char buffer[500];char res_hdr[200];
	int i=0;
	struct sockaddr_in serv_addr, cli_addr;                               /*stores address of client and server*/
	int n;
	if (argc < 2) {                                            /*break the program if port number is not provided*/
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);                             /*open socket*/
	if (sockfd < 0){
		error("ERROR opening socket");
		exit(1);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));                     /*assign null values to sever address variable*/
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	while(i<1000){
		listen(sockfd,5);                                                           /*keep listening on server*/

		clilen = sizeof(cli_addr);


		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);                                                           /*accept client on new socket*/
		if (newsockfd < 0){
			error("ERROR on accept");
                          }

		//process_new_thread(newsockfd);
        printf("Creating a new thread\n");
		pthread_create(&arr_thread[i], NULL, &process_new_thread,newsockfd);

		i++;
         }
	close(sockfd);/*after close request received close the socket*/


	return 0;
}


