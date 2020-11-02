#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <signal.h>
#include <pthread.h>
#include <fstream>
#include "CRC.h"
#include <poll.h>
#include <chrono>

using namespace std;
string path;
string fullpath;
int count = 0;

void sig_handler(int sig)
{
  exit(1);
}
void* socketThread(void* arg){
  pthread_detach(pthread_self());
  count++;
  string a = to_string(count);
  int newSocket = *((int *)arg);
  char* buf = new char[1024];
  uint8_t* ubuf = new uint8_t[1024];
  fullpath = path + "/" + a + ".file";
  ofstream myfile;
  int s = 0;
  int first_dced=0;
  chrono::steady_clock sc;
  auto start = sc.now();
  myfile.open (fullpath.c_str(), ios::app);
  if(!myfile.is_open()){
    cerr <<"Error: the file couldnt be found." <<endl;
    exit(1);
  }
  while (1) {
    //memset(buf,0,1024);
    //memset(ubuf,0,1024);
    int r = recv(newSocket, buf, 1024, 0);
    if(r==0){
      break;
    }
    if(r==0){
      if (first_dced == 0){
	  start = sc.now();
	  first_dced = 1;
	}
      else{
	auto end = sc.now();
	auto time_span = static_cast<chrono::duration<double>>(end - start);   // measure time span between start & end
	s = s + time_span.count();
	if(s>= 10){
	  myfile << "Error: No mesasge has been recieved for 10s." << endl;
	  first_dced = 0;
	  s = 0;
	  break;
	}
      }
      }
    if (r == -1) {
      cerr << "Error: Failed to recieve files." << endl;
      exit(1);
    }
    CRC crc;
    uint64_t crc_code;
    memcpy(ubuf, buf,strlen(buf));
    crc_code = crc.get_crc_code(ubuf, 1024);
    if(crc_code != 0x0000000000000000){
      cerr << crc_code;
      cerr << "Error: File is corrputed while trnasmitting." << endl;
      //     exit(1);
    }
    char* new_buf = new char[1024];
    strncpy(new_buf, buf,strlen(buf)-8);
    myfile << new_buf;
    memset(new_buf,0,1024);
    memset(buf,0,1024);
    memset(ubuf,0,1024);
  }
  myfile.close();
  return 0;
}

int main(int argc, char* argv[]){
  if(argc < 3){
    cerr << "Error: Invalid number of arguments." << endl;
    return 1;
  }
  int port = atoi(argv[1]);
  if(port < 1 || port > 65536){
    cerr << "Error: Invalid port number." << endl;
    return 1;
  }
  path = string(argv[2]);

  signal(SIGINT, sig_handler);
  //create sock
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      cerr <<"ERROR: Failed to open socket";
    }
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    cerr << "Error: setsockopt" << endl;
    return 1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  //bind
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    cerr << "Error: Failed to bind the socket and server." << endl;
    return 1;
  }
  //listen
  if (listen(sockfd, 10) == -1) {
    cerr <<"Error: Failed to listen" << endl;
    return 1;
  }
  pthread_t tid;
  while(1){
    //accpet
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    cout << "Accept a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << endl;
    //make thread
    int result = pthread_create(&tid, NULL, socketThread,&clientSockfd);
    if (result < 0){
      cout << "Failed to create thread from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;
      exit(1);
    }
  }
  //for(int i = 0; i < 10; i++)
  // {
  //  pthread_join(tid[i], NULL);
  // }
  close(sockfd);
  return 0;
}
