

/*
 * rudpserver.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: prerna
 */

/* A simple reliable UDP server in the internet domain.
   The port number and advertised window size is passed as an argument */

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

float file_sz_gl;
int count=0;
int MAX=500;
double alpha=0.125;
double beta=0.25;
int MSS=1424;
int cnwd=1;
int ssthresh=20000;
int min_window;
struct timeval sample_RTT,estimated_RTT,dev_RTT,timeout_interval;




struct http_hdr { char rqst_type[20];/*This is the structure of the header which is used by HTTP*/
char rqst_file[200];
char rqst_http[20];
char con_type[50];
};


struct datagram_hdr{ int ack_flag;/*This is the structure of the header which is used by reliable UDP server and client*/
unsigned long seq_num;
unsigned long ack_num;
char payload[1400];
}send_buffer[10000]={'\0'};

void error(const char *msg)/*This is the standard error throwing function*/
{
	cerr<<"Error: "<<msg<<endl;
	exit(1);
}

/*This method displays the content of the header received from server*/
void print_hdr(struct datagram_hdr h)
{
	cout<<"printing header request.. \n";
	cout<<"Ack no.:"<<h.ack_num<<endl;
	cout<<"Seq_num:"<<h.seq_num<<endl;
	cout<<"Ack flag"<<h.ack_flag<<endl;
	//cout<<"Payload:"<<h.payload<<endl;
}
/*This method holds the packets sent to the client while it is being transmitted so that it can re-transmit if needed*/
void store_send_buffer(struct datagram_hdr hs)
{


	send_buffer[count].ack_num=hs.ack_num;
	send_buffer[count].seq_num=hs.seq_num;
	send_buffer[count].ack_flag=hs.ack_flag;
	strcpy(send_buffer[count].payload,hs.payload);
	count++;
}


char* getfile(char* fl_nm)   /*this function retreives the file requested by client and responds back to the client.*/
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

		cout<< "File Not Found on server\n";
		return NULL;
	}

	else   {
		fseek(fp,0,SEEK_END);
		file_size=ftell(fp);
		file_sz_gl=file_size;
		fseek(fp,0,SEEK_SET);
		fl_inside=(char *)calloc(1,file_size);
		if(fl_inside){
			fread(fl_inside,file_size,1,fp);
		}
		fclose(fp);
		return fl_inside;
	}

}

/*this function parses the request received by client and gets the filename,request type and HTTP protocol*/
void parse_http(char buffer[],struct http_hdr *hdr){
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

/*This method sets the header to the values passed by the main fucntion*/
void set_dgram_hdr(struct datagram_hdr *dhdr,unsigned long acknum,unsigned long seqnum,int ackflag,char *payld_packet)
{
	dhdr->ack_num=acknum;
	dhdr->seq_num=seqnum;
	dhdr->ack_flag=ackflag;
	strcpy(dhdr->payload,payld_packet);
}

void print_send_buffer()
{
	int n_pckt1=ceil(file_sz_gl/MAX);
	cout<<"stored packets are:"<<endl;
	for(int pk=0;pk<n_pckt1;pk++)
	{
		print_hdr(send_buffer[pk]);
	}
}
/*This method calculates the round trip time according to Jacobson algorithm*/
struct timeval calc_RTT(struct timeval tv1,struct timeval tv2)
{

	cout<<endl<<"Calculating Round trip time"<<endl;
	sample_RTT.tv_sec=(tv2.tv_sec-tv1.tv_sec);
	sample_RTT.tv_usec=(tv2.tv_usec-tv1.tv_usec);
	//cout<<"Sample RTT is:"<<sample_RTT.tv_sec<<" seconds and "<<sample_RTT.tv_usec<<" microseconds"<<endl;
	estimated_RTT.tv_sec=((1-alpha)*estimated_RTT.tv_sec + (alpha*sample_RTT.tv_sec));
	estimated_RTT.tv_usec=((1-alpha)*estimated_RTT.tv_usec + (alpha*sample_RTT.tv_usec));
	//cout<<"Estimated RTT is:"<<estimated_RTT.tv_sec<<"seconds and "<<estimated_RTT.tv_usec<<" microseconds"<<endl;
	dev_RTT.tv_sec=((1-beta)*dev_RTT.tv_sec + beta*(abs(sample_RTT.tv_sec-estimated_RTT.tv_sec)));
	dev_RTT.tv_usec=((1-beta)*dev_RTT.tv_usec + beta*(abs(sample_RTT.tv_usec-estimated_RTT.tv_usec)));
	//cout<<"Dev RTT is:"<<estimated_RTT.tv_sec<<"seconds and "<<estimated_RTT.tv_usec<<" microseconds"<<endl;
	timeout_interval.tv_sec=estimated_RTT.tv_sec+4*dev_RTT.tv_sec;
	timeout_interval.tv_usec=estimated_RTT.tv_usec+4*dev_RTT.tv_usec;
	//cout<<"timeout_interval is:"<<timeout_interval.tv_sec<<"seconds and "<<timeout_interval.tv_usec<<" microseconds"<<endl;

	return timeout_interval;
}


/*This method makes the packet with acknowledgement number,seq number and chunks of file data*/
struct datagram_hdr make_packet(int sequence_num)/*makes packet with sequence number passed*/
{
	struct datagram_hdr new_packet;
	new_packet.ack_num=send_buffer[sequence_num-1].ack_num;
	new_packet.seq_num=send_buffer[sequence_num-1].seq_num;
	new_packet.ack_flag=send_buffer[sequence_num-1].ack_flag;
	strcpy(new_packet.payload,send_buffer[sequence_num-1].payload);
	return new_packet;
}

unsigned long int segment_the_file(char *fl_inside) /*if file size is more then break it in packets and store it in send buffer*/
{
	unsigned long int n_pckt=0;
	int i=0;
	int k=0,j;
	unsigned long int seq=1;
	unsigned long int ack=1;
	char* packet=new char[MAX];
	struct datagram_hdr dgram_hdr_seg;
	n_pckt=ceil(file_sz_gl/MAX);
	for(i=0;i<n_pckt;i++)
	{
		for(j=i*MAX,k=0;j<(i+1)*MAX && k<MAX;j++,k++)
		{
			packet[k]=fl_inside[j];

		}

		packet[k]='\0';
		set_dgram_hdr(&dgram_hdr_seg,ack,seq,0,packet);
		store_send_buffer(dgram_hdr_seg);
		seq++;
		ack++;
	}
	return n_pckt;
}
/* calculates packets to drop*/

/*This function implements the sliding window and congestion window functionality and then sends packet accordingly*/
/*Also this function sends acks to the server according to packets delivered in sequence number or out of order*/
unsigned long  send_packets_window(int * packetsToDrop, int noOfPacketsToDrop1,unsigned long nextseqnum1,unsigned long sendbase1, int N_win,int sockfd1,struct sockaddr_in cli_addr1,int fromlen1,int num_pckts1)
{
	int no_bytes=0;
	struct datagram_hdr new_packet;


	min_window=(cnwd>N_win?N_win:cnwd);


	while(nextseqnum1<sendbase1+min_window &&  nextseqnum1<=num_pckts1)
	{


		new_packet=make_packet(nextseqnum1);
		if(nextseqnum1==num_pckts1){new_packet.ack_flag=1;}
		bool flag=true;
		for(int i=0;i<noOfPacketsToDrop1;i++)
		{
			if(nextseqnum1==1)
			{cout<<"Sequence numbers to drop "<<packetsToDrop[i]<<endl;}

			//sleep(2);
			if(nextseqnum1==packetsToDrop[i])
			{
				flag=false;
			}
		}

		if(flag)
		{
			//sleep(4);
			no_bytes = sendto(sockfd1,&new_packet,sizeof(new_packet), 0,(struct sockaddr *)&cli_addr1,fromlen1);
			if (no_bytes < 0)
				error("ERROR in sendto");
			cout<<"Sending packet:"<<nextseqnum1<<endl;
			//sleep(1);


		}
		nextseqnum1=nextseqnum1+1;




	}

	return nextseqnum1;
}




int main(int argc, char *argv[])
{
	int sockfd, length, portno;
	double n_pckt;
	int file_size[2000000];
	int if_p=0;
	int flag=1;
	int rcvd_bytes;
	char *fl_inside;
	char filebuf[200000];
	char *file_name;
	struct http_hdr hdr;                                                      /*this contains the request type,file name requested and protocol*/
	FILE *fp=NULL;
	socklen_t fromlen;
	char buf[2000];
	char res_hdr[200];
	int i=0;
	char *file_sz_char;
	char *packet=new char[MAX];
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	unsigned long n;
	time_t t;
	time(&t);
	int Win_size;
	unsigned long int seq=1;
	unsigned long int ack=1;
	unsigned long int num_pckts=0;
	int ackf=0;

	sample_RTT.tv_sec=0;sample_RTT.tv_usec=0;
	estimated_RTT.tv_sec=0;estimated_RTT.tv_usec=0;
	dev_RTT.tv_sec=0;dev_RTT.tv_usec=0;
	timeout_interval.tv_sec=2;timeout_interval.tv_usec=1000;
	struct timeval t1,t2,tv;
	unsigned long nextseqnum=1,sendbase=1;
	int N_win=0;
	struct datagram_hdr dgram_hdr,dgram_hdr_rcv;
	fd_set rfds;
	int retval=1;





	if(argc < 3) {
		fprintf(stderr,"ERROR, Usage:1.Port 2.Window size in bytes\n");
		exit(1);
	}
	Win_size=atoi(argv[2]);
	N_win=ceil(Win_size/MSS);

	int dropPercent=40;
	int noOfPacketsToDrop=(dropPercent*N_win)/100;
	int *packetsToDrop=new int [noOfPacketsToDrop];

	cout<<"No of packets to drop:"<<noOfPacketsToDrop<<endl;
	cout<<"The size of datagram is:"<<sizeof(dgram_hdr);
	sockfd=socket(AF_INET, SOCK_DGRAM, 0);/*create the socket*/
	if (sockfd < 0) error("Error Opening socket");
	length = sizeof(serv_addr);
	bzero(&serv_addr,length);
	bzero(&cli_addr,length);
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(atoi(argv[1]));
	if (bind(sockfd,(struct sockaddr *)&serv_addr,length)<0)/*bind the sever socket*/
		error("binding error");
	fromlen = sizeof(struct sockaddr_in);
	while (1)
	{                                                                          /*server listens forever*/
		n = recvfrom(sockfd,buf,1024,0,(struct sockaddr *)&cli_addr,&fromlen);                /*receives request from client*/
		if (n < 0) error("error in recvfrom");
		write(1,"Received a request datagram: ",30);
		write(1,buf,n);
		parse_http(buf,&hdr);/*parse the client request*/
		fl_inside=getfile(hdr.rqst_file);

		cout<<"Preparing to send.. \n";
		num_pckts=segment_the_file(fl_inside);
		cout<<"The file is segmented and stored in send buffer.."<<endl;

		cout<<"The number of packets is:"<<num_pckts<<endl;


		/*start a while loop untill the whole file is sent*/
		while(nextseqnum<=num_pckts)
		{   int dup_cnt_ack=1;

		if(sendbase==1 && nextseqnum==1)
		{
			noOfPacketsToDrop=(dropPercent*N_win)/100;
			for(int ii=0;ii<noOfPacketsToDrop;ii++)
			{
				packetsToDrop[ii]=rand()%N_win+1;

			}
		}
		//sleep(10);
		gettimeofday(&t1,NULL);
		nextseqnum=send_packets_window(packetsToDrop,noOfPacketsToDrop,nextseqnum,sendbase,N_win,sockfd,cli_addr,fromlen,num_pckts);
		cout<<"Sent all packets in window.."<<endl;


		FD_ZERO(&rfds);/*start timer on socket*/
		FD_CLR(sockfd,&rfds);

		FD_SET(sockfd,&rfds);

		retval=select(sockfd+1, &rfds, NULL, NULL, &timeout_interval);
		if(retval==0){  /*server timeout so retransmit*/

			ssthresh=cnwd/2*MSS;
			cnwd=1;
			timeout_interval.tv_sec=2*(timeout_interval.tv_sec);
			timeout_interval.tv_usec=2*(timeout_interval.tv_usec);

			continue;
		}
		if(retval==-1){
			cout<<"Error in rcvfrom()"<<endl;
			exit(0);
		}
		if((FD_ISSET(sockfd,&rfds))&& (retval==1)){ /*server receives ack if no timed out*/
			n = recvfrom(sockfd,&dgram_hdr_rcv,sizeof(dgram_hdr_rcv),0,(struct sockaddr *)&cli_addr,&fromlen);
			gettimeofday(&t2,NULL);/*take time when last ack received*/
			if(dgram_hdr_rcv.ack_num<nextseqnum)
			{

				for(int dup_cnt=1;dup_cnt<3;dup_cnt++)
				{
					n = recvfrom(sockfd,&dgram_hdr_rcv,sizeof(dgram_hdr_rcv),0,(struct sockaddr *)&cli_addr,&fromlen);

					cout<<"Received Acknowledgement Number :"<<dgram_hdr_rcv.ack_num<<" for sequence number: "<<dgram_hdr_rcv.seq_num<<endl;
					if (n < 0)
						error("ERROR in rcv from");

					dup_cnt_ack++;
					for(int k=0;k<noOfPacketsToDrop;k++)
					{
						if(dgram_hdr_rcv.ack_num==packetsToDrop[k])
							packetsToDrop[k]=-1;
					}


				}

			}
			else{
				cout<<"Received Acknowledgement Number :"<<dgram_hdr_rcv.ack_num<<" for sequence number:"<<dgram_hdr_rcv.seq_num<<endl;
				if (n < 0){
					error("ERROR in rcv from");


				}
				if((dup_cnt_ack!=3)&&(min_window==cnwd))
				{
					if(cnwd*MSS>=ssthresh)
					{
						cout<<"Congestion avoidance mode...."<<endl;
						cnwd=cnwd+1;
					}
					else
					{
						cout<<"Slow start mode...."<<endl;
						cnwd=cnwd*2;
					}

				}



			}

			sendbase=dgram_hdr_rcv.ack_num;
			nextseqnum=dgram_hdr_rcv.seq_num +1;


			timeout_interval=calc_RTT(t1,t2);//calculate RTT for current round trip

		}

		}

		printf("last byte transmitted at time:%s\n\n",ctime(&t));
		exit(0);



	}//end of while


	return 0;
}//end of main














