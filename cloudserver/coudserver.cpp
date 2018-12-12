#pragma once
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <time.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
/*����windows socket�Ŀ⺯��*/
#pragma comment(lib,"ws2_32.lib")
using namespace std;
using namespace cv;
#define DEBUG
WSAData wsaData;
SOCKET listenfd, connfd;
SOCKADDR_IN servaddr, clientaddr;
unsigned int seq_number;
int clientaddr_len, sel;
int flag = 1;
//�ö���
fd_set read_fd;
timeval tt;
int set_non_Block(SOCKET socket);//����socket������
int send_non_Block(SOCKET socket, char *buffer, int length, int flags);//��������������
int recv_non_Block(SOCKET socket, char *buffer, int length, int flags);//��������������
int server_transfer_Init(SOCKET *listenfd, SOCKET *connfd, struct WSAData *wsaData);//��ʼ��socket����
void server_transfer_Destroy(SOCKET *listenfd, SOCKET *connfd);//���ٷ���˴���socket															   
int parase_image_number(SOCKET sockfd, short int seq_number);//���������ͼƬ��Ų�����
int WatiForConnect(SOCKET*connfd, SOCKET *listenfd, WSAData *wsaData);
void ReadYUVInit();
void DestoryYUV();
void SendYUV(SOCKET sockfd, short int seq_number);
/*�������������г���*/
#define MAX_LIS_NUM 10
#define PORT_NUMBER 6666
#define WID 1344
#define YUV_BUF WID*WID*12/8 
ifstream fin;
char filename[500];
//
FILE *(fp[30]);//
char YUVname[30][100];
unsigned char YUVbuffer[YUV_BUF];
Mat mm;
bool flag_conncet = false;
void conv_yuv420_to_mat(Mat &dst, unsigned char* pYUV420, int width, int height)
{
	if (!pYUV420) {
		return;
	}

	IplImage *yuvimage, *rgbimg, *yimg, *uimg, *vimg, *uuimg, *vvimg;

	int nWidth = width;
	int nHeight = height;
	rgbimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);
	yuvimage = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);

	yimg = cvCreateImageHeader(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	uimg = cvCreateImageHeader(cvSize(nWidth / 2, nHeight / 2), IPL_DEPTH_8U, 1);
	vimg = cvCreateImageHeader(cvSize(nWidth / 2, nHeight / 2), IPL_DEPTH_8U, 1);

	uuimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	vvimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);

	cvSetData(yimg, pYUV420, nWidth);
	cvSetData(uimg, pYUV420 + nWidth * nHeight, nWidth / 2);
	cvSetData(vimg, pYUV420 + long(nWidth*nHeight*1.25), nWidth / 2);
	cvResize(uimg, uuimg, CV_INTER_LINEAR);
	cvResize(vimg, vvimg, CV_INTER_LINEAR);

	cvMerge(yimg, uuimg, vvimg, NULL, yuvimage);
	cvCvtColor(yuvimage, rgbimg, CV_YCrCb2RGB);

	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);

	cvReleaseImage(&yuvimage);

	//dst = Mat(*rgbimg,int(1));
	dst = cvarrToMat(rgbimg, true);
	//rgbimg->
	cvReleaseImage(&rgbimg);
}
int main()
{	
	
	ReadYUVInit();
	while (1) {
		
		if (server_transfer_Init(&connfd, &listenfd, &wsaData) == -1) {
			printf("Socket error!\n");
			exit(1);
		}
		if (WatiForConnect(&connfd, &listenfd, &wsaData) == -1) {
			printf("waitfor connect error!\n");
			//exit(1);
			server_transfer_Destroy(&connfd, &listenfd);
			continue;
		}		
		while (1) {
			seq_number = -1;
			if (recv_non_Block(connfd, (char *)&seq_number, 4, 0) <= 0) {
				bool flag_conncet = false;
				for (int i = 0; i < 30; i++)
					fseek(fp[i], 0, SEEK_SET);
				break;
			}
			if (ntohl(seq_number) != -1) {
				//parase_image_number(connfd, ntohl(seq_number));			
				//cout << ntohl(seq_number) << endl;
				SendYUV(connfd, ntohl(seq_number));
			}			
		}
		server_transfer_Destroy(&connfd, &listenfd);
		
	}	
	DestoryYUV();
	
	return 0;
}
void ReadYUVInit() {
	int i = 0;
	for (; i < 30; ++i) {
		sprintf_s(YUVname[i], "C:\\Users\\cic-vr\\Documents\\Visual Studio 2015\\Projects\\TestVideo\\1344-1344_%d.yuv", i);
		fopen_s(&fp[i], YUVname[i], "rb+");
		if (fp[i] == NULL)		{
			printf("Can't open yuv file!\n");	
		}
		//cout << YUVname[i] << endl;		
	}	
}
void DestoryYUV() {
	int i = 0;
	for (; i < 30; ++i) {
		fclose(fp[i]);
	}
}
void SendYUV(SOCKET sockfd, short int seq_number) {
	int j;
	bool end_flag = 0;
	for (int i = 0; i < 30; ++i) {
		if (i == seq_number) {
			j=fread(YUVbuffer, 1, YUV_BUF, fp[seq_number]);
			if (j == 0)
			{
				fseek(fp[i], 0, SEEK_SET);
				fread(YUVbuffer, 1, YUV_BUF, fp[seq_number]);
				end_flag = 1;
			}
			else
				end_flag = 0;
		}
		else
		{
			if(end_flag)
				fseek(fp[i], 0, SEEK_SET);
			else
				fseek(fp[i], YUV_BUF, SEEK_CUR);
		}
		
	}
	send_non_Block(sockfd, (char*)&YUVbuffer, YUV_BUF, 0);
}
/*���������ͼƬ��Ų�����*/
int parase_image_number(SOCKET sockfd, short int seq_number)
{
	int length;
	char *buffer;
	sprintf_s(filename, "C:\\Users\\cic-vr\\Documents\\Visual Studio 2015\\Projects\\VRPlayer\\VRPlayer\\small\\%d.jpg", seq_number);
#ifdef DEBUG
	cout << seq_number <<" " << filename;
#endif // DEBUG
	fin.open(filename,ios::in|ios::binary);
	fin.seekg(0, ios::end);
	length = fin.tellg();
	fin.seekg(0, ios::beg);
	buffer = new char[length+4]; //ƴ���ĸ��ֽڵ��ļ�����    
	if (buffer == NULL) { 
		cout<<"Memory error"<<stderr<<endl;         
		exit(2); 
	}    
	/* ���ļ�������buffer�� */  
	int temp = htonl(length);
	memcpy(buffer, (char*)&temp, 4);

	cout << length << endl;

	fin.read(buffer+4, length);
	send_non_Block(sockfd, buffer, length+4, 0);
	/*if (read_len != length) {
		cout<<"Reading error"<<stderr<<endl;        
		exit(3); 
	}*/    
	fin.close();
	delete []buffer;   
	return 0;
}


/*�������׽��ֵ�recv*/
int recv_non_Block(SOCKET socket, char *buffer, int length, int flags)
{
	int recv_len, ret_val, sel;
	struct timeval tm;

	for (recv_len = 0; recv_len < length;)
	{
		/*�ö���*/
		fd_set read_fd;
		FD_ZERO(&read_fd);
		FD_SET(socket, &read_fd);
		//�ȴ�1s���ղ����ͷ���
		tm.tv_sec = 20;    //1��
		tm.tv_usec = 1;    //1u��

						   /*����select*/
		sel = select(socket + 1, &read_fd, NULL, NULL, &tm);
		if (sel < 0) {   //����ʧ��
			printf("select socket error: (errno: %d)\n", WSAGetLastError());
			return -1;
		}
		else if (sel == 0) {//��ʱ���ؽ��յ�����
			printf("Recv timout!: (errno: %d)\n", WSAGetLastError());
			return recv_len;
		}
		else {
			if (FD_ISSET(socket, &read_fd)) { //���������д
				ret_val = recv(socket, buffer + recv_len, length - recv_len, flags);
				if (ret_val < 0) {
					printf("recv error\n");
					return -2;
				}
				else if (ret_val == 0) {
					printf("connection closed\n");
					return 0;
				}
				else
					recv_len += ret_val;
			}
		}
	}
	return recv_len;
}

/*�������׽��ֵ�send*/
int send_non_Block(SOCKET socket, char *buffer, int length, int flags)
{
	int send_len, ret_val, sel;
	struct timeval tm;

	for (send_len = 0; send_len < length;)
	{
		/*��д��*/
		fd_set write_fd;
		FD_ZERO(&write_fd);
		FD_SET(socket, &write_fd);
		//�����������ϴ󣬵�1s���Ͳ��˾ͷ���
		tm.tv_sec = 2;
		tm.tv_usec = 1;

		sel = select(socket + 1, NULL, &write_fd, NULL, &tm);/*����select*/
		if (sel <0) {   //����ʧ��
			printf("select socket error: (errno: %d)\n", WSAGetLastError());
			return -1;
		}
		else if (sel == 0) {
			printf("Send time out(2s)\n");
			return send_len;
		}
		else {
			if (FD_ISSET(socket, &write_fd)) { //���������д
				ret_val = send(socket, buffer + send_len, length - send_len, flags);
				if (ret_val < 0) {
					printf("send socket error: (errno: %d)\n", WSAGetLastError());
					return -2;
				}
				else if (ret_val == 0) {
					printf("connection closed\n");
					return 0;
				}
				else
					send_len += ret_val;

			}
		}
	}
	return send_len;
}


/*�����׽���Ϊ������ģʽ*/
int set_non_Block(SOCKET socket)
{
	/*��ʶ����0���������ģʽ*/
	int ret;
	unsigned long flag = 1;
	ret = ioctlsocket(socket, FIONBIO, &flag);
	if (ret)
		printf("set nonblock error: (errno: %d)\n", WSAGetLastError());
	return ret;
}


//��ʼ��socket����
int server_transfer_Init(SOCKET*connfd, SOCKET *listenfd, WSAData *wsaData)
{


	if (WSAStartup(MAKEWORD(2, 2), wsaData)) {  //�汾2
		printf("Fail to initialize windows socket!\n");
		return -1;
	}
	//����һ���׽���
	if ((*listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: (errno: %d)\n", WSAGetLastError());
		return -1;
	}
	//�����׽���Ϊ������ģʽ
	if (set_non_Block(*listenfd)) {
		closesocket(*listenfd);
		return -1;
	}
	//��ʼ���׽���*/
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT_NUMBER);
	servaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	/*��������˿��ظ���*/
	if (setsockopt(*listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int)) == -1) {
		printf("set socket option error: (errno: %d)\n", WSAGetLastError());
		closesocket(*listenfd);
		return -1;
	}	
}
//waitfor connect
int WatiForConnect(SOCKET*connfd, SOCKET *listenfd, WSAData *wsaData) {
	/*�󶨶˿�*/
	if (bind(*listenfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: (errno: %d)\n", WSAGetLastError());
		closesocket(*listenfd);
		return -1;
	}

	/*�����˿�*/
	if (listen(*listenfd, MAX_LIS_NUM) == -1) {
		printf("listen socket error: (errno: %d)\n", WSAGetLastError());
		closesocket(*listenfd);
		return -1;
	}

#ifdef DEBUG
	printf("======waiting for client's request!======\n");
#endif

	//���ܶ˿�����
	clientaddr_len = sizeof(SOCKADDR_IN);

	//����select�ȴ�accept
	FD_ZERO(&read_fd);
	FD_SET(*listenfd, &read_fd);
	tt.tv_sec = 600;    //50�볬ʱ,�ȴ��ͻ�������
	tt.tv_usec = 1;    //1u��

	sel = select(*listenfd + 1, &read_fd, NULL, NULL, &tt);
	if (sel <= 0) {   //����ʧ��
		printf("select socket error: (errno: %d)\n", WSAGetLastError());
		return -1;
	}

	if ((*connfd = accept(*listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len)) == -1) {
		printf("accept socket error: (errno: %d)\n", WSAGetLastError());
		closesocket(*listenfd);
		return -1;
	}
#ifdef DEBUG
	char buffer[512];
	inet_ntop(AF_INET, &clientaddr.sin_addr, buffer, 512);
	printf("client IP:%s, port:%d, connected\n", buffer, ntohs(clientaddr.sin_port));
#endif
	flag_conncet = true;
	return 0;
}

//���ٷ���˴���socket
void server_transfer_Destroy(SOCKET *listenfd, SOCKET *connfd)
{
	/* �ȴ��ͻ��˹ر����� */
	char buffer[256];
	int ret;
	while (1)
	{
		ret = recv(*connfd, buffer, 256, 0);
		if (ret <= 0) {
			closesocket(*connfd);
			printf("Client close\n");
			break;
		}
		Sleep(1000);  //˯��1s�ȴ��ر�
	}
	closesocket(*listenfd);
	WSACleanup();
}
