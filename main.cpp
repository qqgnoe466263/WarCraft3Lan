#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

#pragma comment (lib,"ws2_32.lib")

// The IP header structure
//
typedef struct _iphdr
{
	unsigned char  h_len : 4;    // Length
	unsigned char  ver : 4;      // IP Version
	unsigned char  tos;        // Type of service
	unsigned short totlen;     // Total length of the packet
	unsigned short id;         // Unique identifier
	unsigned short offset;     // Fragment offset field
	unsigned char  ttl;        // Time to live
	unsigned char  proto;      // Protocol(TCP,UDP,ICMP,IGMP...)
	unsigned short checksum;   // IP checksum

	unsigned int srcIP;        // Source IP address
	unsigned int destIP;       // Destination IP address
}IpHeader, *LPIpHeader;

// The UDP header structure
//
typedef struct _udphdr
{
	unsigned short sport;       // Source Port    
	unsigned short dport;       // Destination Port   
	unsigned short Length;     // Length      
	unsigned short Checksum;    // Checksum    

}UdpHeader, *LPUdpHeader;

typedef struct _PSHeader
{
	unsigned long srcaddr;
	unsigned long destaddr;

	unsigned char  zero;
	unsigned char  protocol;
	unsigned short len;

}PSHeader;




USHORT checksum(USHORT *buffer, int size);

void main()
{
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	SOCKADDR_IN sock_addr;
	
	unsigned char sendBuf[256];
	unsigned char chksumBuf[256];
	unsigned char data[20] = { 0xf7 ,0x2f ,0x10 ,0x00 ,0x50 ,0x58 ,0x33 ,0x57 ,0x1a ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 };
	char srcIp[16];
	char destIp[16];
	int data_size = 16;
	int chksumLen = 0;
	int optval = 1;



	memset(sendBuf, 0, 256);
	memset(chksumBuf, 0, 256);

	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sock == SOCKET_ERROR) {
		printf("[!] sock error!\n");
		system("pause");
		exit(-1);
	} else {
		printf("[*] sock ok!\n");
	}


	
	int ret = setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&optval, sizeof(optval));
	if (ret == SOCKET_ERROR) {
		printf("Error setting opt: %d", WSAGetLastError());
		system("pause");
		exit(-1);
	} else {
		printf("[*] set opt ok!\n");
	}



	IpHeader *iphdr;
	UdpHeader *udphdr, udpHdr;
	int iUdpSize, error;
	PSHeader pseudo_header;


	iphdr = (IpHeader *)sendBuf;  // the ip header now points to the top of the sendBuf
	udphdr = (UdpHeader *)(sendBuf + sizeof(IpHeader));  // the udp header points to the part next to the ip header

	int j = 0;
	for (int i = sizeof(IpHeader) + sizeof(UdpHeader); i < sizeof(IpHeader) + sizeof(UdpHeader) + data_size - 1; i++) {
		sendBuf[i] = data[j];
		j++;
	}

	printf("[*] srcIp : ");
	scanf("%15s",&srcIp);
	printf("[*] destIp: ");
	scanf("%15s", &destIp);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(6112);
	sock_addr.sin_addr.s_addr = inet_addr(destIp);

	iphdr->ver = 4;
	iphdr->h_len = 5;
	iphdr->tos = 0;
	iphdr->totlen = sizeof(IpHeader) + sizeof(UdpHeader) + data_size;
	iphdr->id = 1;
	iphdr->offset = 0;
	iphdr->ttl = 255;
	iphdr->proto = IPPROTO_UDP;  //UDP
	iphdr->checksum = 0;

	iphdr->srcIP = inet_addr(srcIp); // your source id
	iphdr->destIP = sock_addr.sin_addr.s_addr;

	// Initalize the UDP header
	//
	iUdpSize = sizeof(UdpHeader) + data_size;

	udphdr->sport = htons(6112);
	udphdr->dport = htons(6112);
	udphdr->Length = htons(iUdpSize);
	udphdr->Checksum = 0;

	//calculate UDP CheckSum
	pseudo_header.destaddr = inet_addr(destIp);
	pseudo_header.srcaddr = inet_addr(srcIp);
	pseudo_header.zero = 0;
	pseudo_header.protocol = IPPROTO_UDP;
	pseudo_header.len = udphdr->Length;

	memcpy(chksumBuf, (PVOID)&pseudo_header, sizeof(pseudo_header));
	chksumLen += sizeof(pseudo_header);
	memcpy(chksumBuf + chksumLen, udphdr, sizeof(UdpHeader));
	chksumLen += sizeof(UdpHeader);
	memcpy(chksumBuf + chksumLen, data, data_size);
	chksumLen += data_size;

	udphdr->Checksum = checksum((unsigned short*)chksumBuf, chksumLen);
	iphdr->checksum = checksum((unsigned short *)iphdr, sizeof(IpHeader));



	error = sendto(sock, (char*)sendBuf, 28 + data_size, 0, (LPSOCKADDR)&sock_addr, sizeof(SOCKADDR_IN));

	if (error == SOCKET_ERROR)
		cout << WSAGetLastError() << endl;
	else
		cout << endl << "sent" << endl;

	system("pause");
	return;
}



USHORT checksum(USHORT *buffer, int size)
{
	unsigned long cksum = 0;

	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}

	if (size)
	{
		cksum += *(UCHAR*)buffer;
	}

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (USHORT)(~cksum);
}