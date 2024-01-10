
#include "UDP.hpp"


uint32_t dotted2binary(const char p_acIP[])
{
 uint32_t l_ulAdd;
 uint8_t l_ucByte;
 char *l_pcToken;
 char l_acIP[32];

 strncpy(l_acIP, p_acIP, 32);
 if ((l_pcToken = strtok(l_acIP,".")) == NULL)
   return 0x00000000;
 l_ucByte = (uint8_t)atoi(l_pcToken);
 l_ulAdd = (uint32_t)l_ucByte * 16777216;
 if ((l_pcToken = strtok(NULL,".")) == NULL)
   return 0x00000000;
 l_ucByte = (uint8_t)atoi(l_pcToken);
 l_ulAdd += (uint32_t)l_ucByte * 65536;
 if ((l_pcToken = strtok(NULL,".")) == NULL)
   return 0x00000000;
 l_ucByte = (uint8_t)atoi(l_pcToken);
 l_ulAdd += (uint32_t)l_ucByte * 256;
 if ((l_pcToken = strtok(NULL,".")) == NULL)
   return 0x00000000;
 l_ucByte = (uint8_t)atoi(l_pcToken);
 l_ulAdd += (uint32_t)l_ucByte * 1;
 return l_ulAdd;
}




UDP::UDP(uint16_t p_usPort)
 {
  struct sockaddr_in l_localAddress; 
  WSADATA l_wsaData;
  unsigned long l_ulArg = 1;

  if (WSAStartup(0x0202, &l_wsaData) != 0)
	{
	 m_bValidSocket = false;
   	 return;
    }
  if ((m_sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
   {
	m_bValidSocket = false;
   	return;
   }
  memset((char*)&l_localAddress, 0 , sizeof(l_localAddress));
  l_localAddress.sin_family = AF_INET;
  l_localAddress.sin_port = htons(p_usPort);
  l_localAddress.sin_addr.s_addr = 0;	// localhost
  // set local port number
  if (bind(m_sd, (struct sockaddr*)&l_localAddress, sizeof(l_localAddress)) == SOCKET_ERROR)
    {
	 closesocket(m_sd);
     //m_sd = -1;
	 m_bValidSocket = false;
   	 return;
    }
   // make a non-blocking socket
  if (ioctlsocket(m_sd, FIONBIO, &l_ulArg) == SOCKET_ERROR) 
    {
	 closesocket(m_sd);
     //m_sd = -1;
	 m_bValidSocket = false;
   	 return;
    }
 
  m_bValidSocket = true;
  return;  
 }
 

 UDP::~UDP (void)
 {
  if (m_bValidSocket)
    closesocket(m_sd);
  WSACleanup();
  return;
 }
 
 
 int UDP::send(uint32_t p_ulRemoteHost, uint16_t p_usRemotePort, uint8_t p_aucBuffer[], int p_iNbytes)
 {
  struct sockaddr_in l_remoteAddress;                       

  if (!m_bValidSocket)
  	return -1;
  
  memset((char*)&l_remoteAddress, 0 , sizeof(l_remoteAddress));
  l_remoteAddress.sin_family = AF_INET;
  l_remoteAddress.sin_port = htons(p_usRemotePort);
  l_remoteAddress.sin_addr.s_addr = htonl(p_ulRemoteHost);
 
  if (sendto(m_sd, (char*)p_aucBuffer, p_iNbytes, 0, (struct sockaddr*)&l_remoteAddress, sizeof(l_remoteAddress)) == SOCKET_ERROR)
  	{
     return -1;
	}
  return 0;
 }

         
 int UDP::receive(uint32_t *p_pulRemoteHost, uint16_t *p_pusRemotePort, uint8_t *p_aucBuffer, int p_iDim)
 {
  struct sockaddr_in l_remoteAddress;
  int l_iRemoteAddressSize = sizeof(l_remoteAddress);
  int l_iNbytes = 0;

   if (!m_bValidSocket)
  	 return -1;
  
  if ((l_iNbytes = recvfrom(m_sd, (char*)p_aucBuffer, p_iDim, 0, (struct sockaddr*)&l_remoteAddress, &l_iRemoteAddressSize)) == SOCKET_ERROR)
    {
     if (WSAGetLastError() == WSAEWOULDBLOCK)
   	   return 0;
	 return -1;
	}
	
  *p_pulRemoteHost=ntohl(l_remoteAddress.sin_addr.s_addr);
  *p_pusRemotePort=ntohs(l_remoteAddress.sin_port);
	
  return (l_iNbytes);
 }


  int UDP::wait(unsigned long p_ulTimeout)
  {
   int l_iN;
   struct timeval l_timeout;
   int error;
   fd_set l_socketSet;
  
   if (!m_bValidSocket)
  	 return -1;

   FD_ZERO(&l_socketSet);
   FD_SET(m_sd, &l_socketSet);
   l_timeout.tv_sec = p_ulTimeout/1000;
   l_timeout.tv_usec = (p_ulTimeout%1000)*1000;
   
   l_iN = select(0, &l_socketSet, NULL, NULL, &l_timeout);
   if (l_iN == SOCKET_ERROR)
	 {
	  error = WSAGetLastError();
	  return -1;
     }
   if (l_iN == 0)
	 return 0;
   if (FD_ISSET(m_sd, &l_socketSet))
	 return 1;
   else
	   return 0;
  }
