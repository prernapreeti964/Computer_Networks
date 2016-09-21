/*
* reliableclient.cpp
 *
 *  Created on: Oct 11, 2015
 *      Author: prerna
 */

/* A simple reliable UDP client in the internet domain.
   The port number of server,hostname,filename requested and advertised window size is passed as an argument
   The file requested is downloaded from the server.*/

#include <iostream>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include<fstream>
#include<map>
#include<cmath>
#include <sstream>
#include <sys/time.h>
#include <signal.h>


using namespace std;

int MSS=1424;/*the maximum segement size transmitted */


void error(const char *msg)/*This function is a standard error throwing function*/
{
	cerr<<"Error: "<<msg<<endl;
	exit(1);
}


struct datagram_hdr /*This is the structure of the header which is used by reliable UDP server and client*/
{ int ack_flag;
unsigned long seq_num;
unsigned long ack_num;
char payload[1400];
}rcv_buffer[3000];


/*This method sets the header to the values passed by the main fucntion*/
void set_dgram_hdr(struct datagram_hdr *dhdr,unsigned long acknum,unsigned long seqnum,int ackflag,char *payld_packet)
{
	dhdr->ack_num=acknum;
	dhdr->seq_num=seqnum;
	dhdr->ack_flag=ackflag;
	strcpy(dhdr->payload,payld_packet);
}
/*This method sends the acknowledgement to the server that a packet corresponding to a particular sequence number has arrived*/
void send_ack(struct datagram_hdr rcv_hdr,int sock,struct sockaddr_in from1,socklen_t length1)
{

	socklen_t length=length1;
	struct sockaddr_in from=from1;
	unsigned long seq_no=0,ack_no=0;
	int ack_flag,no_bytes,sock1;
	sock1=sock;
	struct datagram_hdr send_hdr;
	send_hdr.seq_num=rcv_hdr.seq_num;
	send_hdr.ack_num=rcv_hdr.ack_num+1;
	send_hdr.ack_flag=1;
	strcpy(send_hdr.payload,"An ack for packet recieved");
	no_bytes=sendto(sock1,&send_hdr,sizeof(send_hdr), 0,(struct sockaddr *)&from,length);


}
/*This method displays the content of the header received from server*/
void print_hdr(struct datagram_hdr h)
{
	cout<<"printing header request.. \n";
	cout<<"Ack_no:"<<h.ack_num<<endl;
	cout<<"Seq_no:"<<h.seq_num<<endl;
	cout<<"Ack_flag"<<h.ack_flag<<endl;
	cout<<"Data:"<<endl<<h.payload<<endl;
}

/*The main function*/
int main(int argc, char *argv[])
{
	int sock, n,count=0,bytes=0,packet=0;
	int Win_size,N_win;
	int len=0,dup_flag=0;
	socklen_t length;
	datagram_hdr dgram_hdr,dgram_hdr1,dgram_hdr2;
	struct sockaddr_in server, from;
	struct hostent *hp;
	char buffer[256],fl_nm[100];
	fd_set rfds;
	struct timeval tv_to;
	ofstream outputFile;
    unsigned long next_expected_seq=1;

	if (argc < 5)
	{                                                                                                       /*break the program if arguments are not received*/
		fprintf(stderr,"usage %s hostname port filename window_size_in_bytes\n", argv[0]);
		exit(0);
	}
	Win_size=atoi(argv[4]);
	//cout<<"Window size is"<<Win_size;
	N_win=ceil(Win_size/MSS);
	//cout<<"N_win is"<<N_win;
	strcpy(fl_nm,argv[3]);
	puts(fl_nm);


	sock= socket(AF_INET, SOCK_DGRAM, 0);/*create the socket*/
	if (sock < 0) error("socket");
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp==0) error("Unknown host");
	bcopy((char *)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length=sizeof(struct sockaddr_in);
	bzero(buffer,256);
	sprintf(buffer,"GET /%s HTTP/1.1\r\nHost: \r\n\r\n", fl_nm);/*create the request for a file in HTTP format*/
	n=sendto(sock,buffer,256,0,(const struct sockaddr *)&server,length);/*send the request for a file*/
	if (n < 0) error("Sendto error");
	n=1;
	dgram_hdr1.ack_flag=0;
	int retval=1;
	outputFile.open(fl_nm);
	while(dgram_hdr1.ack_flag==0)  /*keep receiving the packets till all are received*/
	{
		//cout<<"The outer client while loop starting"<<endl;
		bzero(buffer,256);
		bzero(&dgram_hdr1,sizeof(dgram_hdr1));

		int cnt=0,no_of_dupack=0;
		dup_flag=0;
		while((cnt<N_win)&&(dgram_hdr1.ack_flag==0))  /*keep receiving the packets till all in receive window are received*/
		{


			FD_ZERO(&rfds);
			FD_CLR(sock,&rfds);

			FD_SET(sock,&rfds);
			tv_to.tv_sec=0;
			tv_to.tv_usec=100000;
			retval=select(sock+1, &rfds, NULL, NULL, &tv_to);/*set up a timer for the client in case no packet is received*/
			if(retval==0){                       /*client timeout*/
                //cout<<"timeout"<<endl;
				break;
			}
			if(retval==-1){
				cout<<"Error in rcvfrom()"<<endl;
				break;
			}
			if((FD_ISSET(sock,&rfds))&& (retval==1))/*if socket is ready to read and receive buffer is not blank*/
			{

				n=recvfrom(sock,&dgram_hdr1,sizeof(dgram_hdr1),0,(struct sockaddr *)&from, &length);

				if(dgram_hdr1.seq_num==next_expected_seq)
				{

					bytes=bytes+n;
					packet=packet+1;

					cout<<"\nGot a packet for sequence num:"<<dgram_hdr1.seq_num<<endl;
					//print_hdr(dgram_hdr1);
					printf("The total number of bytes received is: %d\n\n",bytes);
					printf("The total number of packets received is: %d\n\n",packet);
					rcv_buffer[len].ack_num=dgram_hdr1.ack_num;
					rcv_buffer[len].seq_num=dgram_hdr1.seq_num;
					rcv_buffer[len].ack_flag=dgram_hdr1.ack_flag;
					strcpy(rcv_buffer[len].payload,dgram_hdr1.payload);
					outputFile<<dgram_hdr1.payload;
					len++;
					next_expected_seq++;

				}
				else if((dgram_hdr1.seq_num!=next_expected_seq)&&(dgram_hdr1.ack_flag==0))/*dont send ack for out of order packets */
				{
					dup_flag=1;


				}  /*end of else if*/

				cnt++;
			}//end of select if

			else
			{
				cout<<"Receive from timeout on client"<<endl;
			}


		}//end of inner while

		if(dup_flag==0)/*send only one ack if correct packet has arrived*/
		{
			if(len==0){
				dgram_hdr2.ack_num=len+1;
				dgram_hdr2.seq_num=len+1;
			}
			else {dgram_hdr2.ack_num=len;
			dgram_hdr2.seq_num=len;
			}
			//sleep(2);cout<<"sleeping for 2 seconds to test timeout"<<endl;
			send_ack(dgram_hdr2,sock,from,length);
			cout<<"Sending acknowledgement with Ack no.:"<<dgram_hdr2.ack_num+1<<"and sequence no:"<<dgram_hdr2.ack_num<<endl;

		}//end of if

		else if(dup_flag==1){
			if(len==0)
			{
				dgram_hdr2.ack_num=len+1;
				dgram_hdr2.seq_num=len+1;
			}
			else {
				dgram_hdr2.ack_num=len;
				dgram_hdr2.seq_num=len;
			}

			for(no_of_dupack=0;no_of_dupack<3;no_of_dupack++)/*send three duplicate acks if out of order packet is received
				or to indicate a lost packet*/
			{
				//cout<<"Out of order packet received so sending a dup ack"<<endl;
				send_ack(dgram_hdr2,sock,from,length);
				cout<<"Sending duplicate acknowledgements with ack no.:"<<dgram_hdr2.ack_num+1<<"and seq no:"<<dgram_hdr2.ack_num<<endl;
			}
		}//end of else if
	}

outputFile.close();



	close(sock);//close the socket
	return 0;
}








