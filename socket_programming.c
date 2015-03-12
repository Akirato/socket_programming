
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h> 

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>

#include <errno.h> 

#include <sys/stat.h> 
#include <ctype.h> 
#include <fcntl.h> 
#include <signal.h>

pid_t pid;
int server_portno,connect_portno;

void server_side();
void client_side();
void func_IndexGet();
void cIndexget();
void func_FileDownload();
void cFiledownload();
void func_FileUpload();
void cfileupload();

int main()
{
	char transfer_prot[10];
	printf("Server-side Port no: ");
	scanf("%d",&server_portno);
	printf("Connect Port no: ");
	scanf("%d",&connect_portno);
	printf("Transfer Protocol: ");
	scanf("%s",transfer_prot);

	pid = fork();
	if(pid == -1)
	{
		printf("Error: Fork !!!\n");
		exit(0);
	}
	else if(pid == 0)
	{
		server_side();
	}
	else
	{
		client_side();
	}
	kill(pid,SIGQUIT);

	return 0;
}

void server_side()
{
	int listenfd = 0,connfd = 0;

	socklen_t cli_len;

	struct sockaddr_in serv_addr,cli_addr;

	char read_buffer[1024];
	int n;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		error("ERROR opening socket");
	}
	else
	{
		printf("[Server] Socket intialized \n");
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(server_portno);

	if( bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
	{
		error("ERROR on binding");
	}
	else
	{
		printf("[Server] Socket Binded to the Server Address \n");
	}

	if(listen(listenfd, 10) == -1)	
	{
		error("ERROR in listening");
	}

	printf("[Server] Server waiting for an client\n");


	cli_len = sizeof(cli_addr);
	if( ( connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_len) ) < 0)
		error("ERROR on accept");

	while(1)
	{

		memset(read_buffer, '0', sizeof(read_buffer));

		if( (n = read(connfd, read_buffer, 1024)) < 0 )
			error("ERROR writing to socket");

		read_buffer[n] = '\0';

		if( strcmp(read_buffer,"exit") == 0 || strcmp(read_buffer,"q") == 0 || strcmp(read_buffer, "Q") == 0 )
		{
			printf("\n Message: %s \n",read_buffer);
			kill(pid,SIGTERM);
			break;
		}
		else if( strncmp(read_buffer, "IndexGet",8) == 0 )
		{
			func_IndexGet(read_buffer,&connfd);
		}
		else if( strncmp(read_buffer, "FileHash",8) == 0 )
		{
			//	func_FileHash(read_buffer,&connfd);
		}
		else if( strstr(read_buffer, "FileDownload") != NULL)
		{
			func_FileDownload(read_buffer,&connfd);
		}
		else if( strncmp(read_buffer, "FileUpload",10) == 0)
		{		
			func_FileUpload(read_buffer,&connfd);
		}
		while(waitpid(-1, NULL, WNOHANG) > 0);
	}
	close(connfd);
	printf("\n Connection closed by Client");

	close(listenfd);
	exit(1);
	return ;
}

void client_side()
{
	int sockfd = 0;
	char recvBuff[1024],write_buffer[1024];
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		error("ERROR opening socket");	
	}
	else
	{
		printf("[Client] Socket created\n");
	}

	server = gethostbyname("127.0.0.1");
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(connect_portno);
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

	while(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0);

	while(1)
	{
		memset(write_buffer, '0', sizeof(write_buffer));
		printf("Enter the message : ");
		gets(write_buffer);
		printf("\n");

		if(strcmp(write_buffer, "exit") == 0 || strcmp(write_buffer, "Q") == 0 || strcmp(write_buffer, "q") == 0)
		{
			if(write(sockfd,write_buffer,1024) < 0)
				error("ERROR writing to socket");
			printf("[Client] Connection closed\n");
			kill(pid,SIGTERM);
			break;
		}
		else if(strncmp(write_buffer,"IndexGet",8)==0)
		{
			cIndexget(write_buffer,&sockfd);
		}
		else if(strncmp(write_buffer,"FileHash",8)==0)
		{
			//	cFilehash(write_buffer,&sockfd);
		}
		else if(strncmp(write_buffer,"FileDownload",12)==0)
		{
			cFiledownload(write_buffer,&sockfd);
		}
		else if(strncmp(write_buffer,"FileUpload",10)==0)
		{
			cfileupload(write_buffer,&sockfd);
		}
	}
	close(sockfd);
	_exit(0);
	return;
}

void func_IndexGet(char read_buffer[],int *newsockfd)
{

	char write_buffer[1024];
	bzero(write_buffer,1024);
	char l[8]="LongList",r[5]="RegEx",s[9]="ShortList";

	int t=-1,i;
	int flag;

	flag = 0;
	for(i=0;i<8;i++)
		if(l[i]!=read_buffer[9+i])
			flag =1;
	if(flag == 0)
	{
		t = 0;
	}

	if(t < 0)
	{
		flag = 0;
		for(i=0;i<5;i++)
			if(r[i]!=read_buffer[9+i])
				flag = 1;

		if(flag == 0)
		{
			t = 1;
		}
	}
	if(t < 0)
	{
		flag = 0;
		for(i=0;i<9;i++)
			if(s[i]!=read_buffer[9+i])
				flag = 1;
		if(flag == 0)
		{
			t = 2;
		}
	}

	if(t==0)
	{
		system("touch Result");
		system("ls -l |grep -v ^d | tr -s ' ' | awk '{print $9\"\\t\"$5\"\\t\"$8}'| tail -n +2 > Result");
		bzero(write_buffer, 1024);
		int f=0,flag=0;
		FILE *fs = fopen("Result", "r");
		if(fs == NULL)
		{
			error("ERROR: File not found");
			exit(1);
		}
		while((f= fread(write_buffer, sizeof(char),1024, fs)) > 0)
		{
			if(write(*newsockfd, write_buffer, 1024) < 0)
			{
				error("\nERROR: Writing to socket");
				exit(1);
				break;
			}
			bzero(write_buffer, 1024);
			flag=1;
		}
		if(flag==1)
		{
			bzero(write_buffer, 1024);
			strcpy(write_buffer,"END");
			if(write(*newsockfd, write_buffer, 1024) < 0)
			{
				error("\nERROR: Writing to socket");
				exit(1);
			}
		}
		system("rm -rf Result");

	}



	/*
	   char write_buffer[1024];
	   memset(write_buffer, '0', sizeof(write_buffer));

	   char parameter[10];
	   int parameter_flag;
	   int i,j;

	   i = 0;
	//	while(read_buffer[9 + i] == ' ')
	//	{
	//		i++;
	//	}
	//	j = 0;
	while(read_buffer[9 + i] != '\0' || read_buffer[9 + i] != '\n')
	{
	parameter[j] = read_buffer[9 + i];
	j++;
	i++;
	}
	parameter[j] = '\0';

	if( strncmp(parameter,"ShortList",9) == 0 )
	{
	parameter_flag = 0;
	}
	else if( strncmp(parameter,"LongList",8) == 0 )
	{
	printf("wooh server\n");
	parameter_flag = 1;
	}
	else if( strncmp(parameter,"RegEx",5) == 0 )
	{
	parameter_flag = 2;
	}
	else
	{
	parameter_flag = -1;
	}


	if(parameter_flag == 1)
	{
	system("touch Result");
	system("ls -l |grep -v ^d | tr -s ' ' | awk '{print $9\"\\t\"$5\"\\t\"$8}'| tail -n +2 > Result");

	FILE *fd;
	int f,flag;

	fd = fopen("Result", "r");
	if(fd == NULL)
	{
	error("ERROR: File not found/not formed");
	exit(1);
	}
	while( ( f = fread(write_buffer, sizeof(char), 1024, fd) ) > 0)
	{
	if(write(*connfd, write_buffer, 1024) < 0)
	{
	error("\nERROR: Writing to socket");
	exit(1);
	break;
	}
	memset(write_buffer, '0', sizeof(write_buffer));
	flag=1;
	}
	if(flag == 1)
	{
	strcpy(write_buffer,"END");
	if( write(*connfd, write_buffer, 1024) < 0)
	{
	error("\nERROR: Writing to socket");
	exit(1);
}
}
system("rm -rf Result");
}*/
}


void cIndexget(char write_buffer[],int *sockfd)
{
	int n;
	n = write(*sockfd,write_buffer,1024);
	if (n < 0)
		error("ERROR writing to socket");
	write_buffer[n] = '\0';

	char l[8]="LongList",read_buffer[1024],r[5]="RegEx",s[9]="ShortList";

	int t=-1,i;
	int flag;

	flag = 0;
	for(i=0;i<8;i++)
		if(l[i]!=write_buffer[9+i])
			flag =1;
	if(flag == 0)
	{
		t = 0;
	}

	if(t < 0)
	{
		flag = 0;
		for(i=0;i<5;i++)
			if(r[i]!=write_buffer[9+i])
				flag = 1;

		if(flag == 0)
		{
			t = 1;
		}
	}
	if(t < 0)
	{
		flag = 0;
		for(i=0;i<9;i++)
			if(s[i]!=write_buffer[9+i])
				flag = 1;
		if(flag == 0)
		{
			t = 2;
		}
	}
	if(t==0)
	{

		int f=0,flag=1;
		bzero(read_buffer, 1024);
		printf("\nRecieved data : %s\n",read_buffer);
		printf("File-name Size Timestamp\n");
		while((f= read(*sockfd, read_buffer,1024)) > 0)
		{
			if(strcmp(read_buffer,"END")==0)
			{flag=0;
				break;
			}
			else
				printf("%s\n",read_buffer);
		}
		if(flag==1)
			exit(0);
	}



	/*
	   if ( write(*sockfd,write_buffer,1024) < 0 )
	   error("ERROR writing to socket");
	   char read_buffer[1024];
	   char parameter[10];
	   int parameter_flag;
	   int i,j;

	   i = 0;
	//	while(write_buffer[9 + i] == ' ')
	//	{
	//		i++;
	//	}
	j = 0;
	printf("client write buffer--->> %s\n",write_buffer);
	while(write_buffer[9 + i] != '\n' || write_buffer[9 + i] != '\0' || write_buffer[9 + i] != '0')
	{
	parameter[j] = write_buffer[9 + i];
	j++;
	i++;
	}
	parameter[j] = '\0';

	printf("parameter--->> %s\n",parameter);

	if( strcmp(parameter,"ShortList") == 0 )
	{
	parameter_flag = 0;
	}
	else if( strcmp(parameter,"LongList") == 0 )
	{
	printf("wooh client\n");
	parameter_flag = 1;
	}
	else if( strcmp(parameter,"RegEx") == 0 )
	{
	parameter_flag = 2;
	}
	else
	{
	parameter_flag = -1;
	}

	if(parameter_flag == 1)
	{
	int f=0,flag=1;
	memset(read_buffer, '0', sizeof(read_buffer));
	printf("\nRecieved data : %s\n",read_buffer);
	printf("File-name Size Timestamp\n");
	while((f= read(*sockfd, read_buffer,1024)) > 0)
	{
	if(strcmp(read_buffer,"END")==0)
	{
	flag=0;
	break;
	}
	else
	printf("%s\n",read_buffer);
	}
	if(flag==1)
	exit(0);
	}*/
	/*
	   if(t==1)
	   {
	   int f=0,flag=1;
	   bzero(read_buffer, 1024);
	   printf("\nRecieved data : %s\n",read_buffer);
	   printf("File-name Size\n");
	   while((f= read(*sockfd, read_buffer,1024)) > 0)
	   {
	   if(strcmp(read_buffer,"END")==0)
	   {flag=0;
	   break;
	   }
	   else
	   printf("%s\n",read_buffer);
	   }
	   if(flag==1)
	   exit(0);
	   }
	   if(t==2)
	   {
	   int f=0,flag=1;
	   bzero(read_buffer, 1024);
	   printf("\nRecieved data : %s\n",read_buffer);
	   printf("File-name Size Last-Modified Timestamp\n");
	   while((f= read(*sockfd, read_buffer,1024)) > 0)
	   {
	   if(strcmp(read_buffer,"END")==0)
	   {flag=0;
	   break;
	   }
	   else
	   printf("%s\n",read_buffer);
	   }
	   if(flag==1)
	   exit(0);
	   }*/
}


void func_FileDownload(char read_buffer[],int *connfd)
{
	int i,garbage_int = 0;
	char file_name[1024];

	for(i=13;read_buffer[i]!='\0';i++)
	{
		file_name[i-13] = read_buffer[i];
	}	
	file_name[i-13] = '\0';

	char ch;
	FILE *fp;
	fp = fopen(file_name,"r");

	int count;
	char file_data[1024];
	while(fscanf(fp,"%c",&ch)!=EOF)
	{
		count=0;
		file_data[count++]=ch;
		while(count<1024 && fscanf(fp,"%c",&ch)!=EOF)
			file_data[count++]=ch;
		send(*connfd,&count,sizeof(int),0);
		send(*connfd,file_data,1024,0);
	}
	send(*connfd,&garbage_int,sizeof(int),0);
	send(*connfd,"File Ended",1024,0);
	fclose(fp);
}

void cFiledownload(char write_buffer[],int *sockfd)
{
	int i,n;
	n = write(*sockfd,write_buffer,1024);
	if (n < 0)
		error("ERROR writing to socket");
	write_buffer[n] = '\0';

	char file_name[1024];
	for(i=13;write_buffer[i]!='\0';i++)
	{
		file_name[i-13] = write_buffer[i];
	}	
	file_name[i-13] = '\0';

	int bytes_recv,recv_chunk_size;
	char recv_data[1024];

	recv(*sockfd,&recv_chunk_size,sizeof(recv_chunk_size),0);
	bytes_recv = recv(*sockfd,recv_data,1024,0);

	/**************************************************************/

	FILE *fp;
	fp = fopen(file_name,"w");
//	printf("<");
	while(strcmp(recv_data,"File Ended")!=0)
	{
//		printf(".");
		for(i=0;i<recv_chunk_size;i++)
			fprintf(fp,"%c",recv_data[i]);
		recv(*sockfd,&recv_chunk_size,sizeof(int),0);
		bytes_recv = recv(*sockfd,recv_data,1024,0);
		recv_data[bytes_recv]='\0';
		printf("%s",recv_data);
	}
	fclose(fp);
//	printf(">\n");
}


void func_FileUpload(char read_buffer[],int *sockfd)
{
	int i;
	char file_name[1024];

	for(i=11;read_buffer[i]!='\0';i++)
	{
		file_name[i-11] = read_buffer[i];
	}	
	file_name[i-11] = '\0';

	int bytes_recv,recv_chunk_size;
	char recv_data[1024];

	recv(*sockfd,&recv_chunk_size,sizeof(int),0);
	bytes_recv = recv(*sockfd,recv_data,1024,0);

	FILE *fp;
	fp = fopen(file_name,"w");
//	printf("<");
	while(strcmp(recv_data,"File Ended")!=0)
	{
//		printf(".");
		for(i=0;i<recv_chunk_size;i++)
			fprintf(fp,"%c",recv_data[i]);
		recv(*sockfd,&recv_chunk_size,sizeof(int),0);
		bytes_recv = recv(*sockfd,recv_data,1024,0);
		recv_data[bytes_recv]='\0';
		printf("%s",recv_data);
	}
	fclose(fp);
//	printf(">\n");
}

void cfileupload(char write_buffer[],int *connfd)
{
	int i,n;
	int garbage_int = 0;
	n = write(*connfd,write_buffer,1024);
	if (n < 0)
		error("ERROR writing to socket");
	write_buffer[n] = '\0';

	char file_name[1024];
	for(i=11;write_buffer[i]!='\0';i++)
	{
		file_name[i-11] = write_buffer[i];
	}	
	file_name[i-11] = '\0';

	char ch;
	FILE *fp;
	fp = fopen(file_name,"r");

	int count;
	char file_data[1024];
	while(fscanf(fp,"%c",&ch)!=EOF)
	{
		count=0;
		file_data[count++]=ch;
		while(count<1024 && fscanf(fp,"%c",&ch)!=EOF)
			file_data[count++]=ch;
		send(*connfd,&count,sizeof(int),0);
		send(*connfd,file_data,1024,0);
	}
	send(*connfd,&garbage_int,sizeof(int),0);
	send(*connfd,"File Ended",1024,0);
	fclose(fp);
}
