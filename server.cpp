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
using namespace std;
ofstream myfile;
string path;
void sig_handler(int sig)
{
  exit(1);
}
void* socketThread(void* arg){
  int newSocket = *((int *)arg);
  char buf[1024] = {0};
  myfile.open (path.c_str(), ios::app);
  if(!myfile.is_open()){
    cerr <<"Error: the file couldnt be found." <<endl;
    exit(1);
}
  while (1) {
    memset(buf, '\0', sizeof(buf));
    int r = recv(newSocket, buf, 1024, 0) == -1;
    if (r == -1) {
      cerr << "Error: Failed to recieve files." << endl;
      exit(1);
    }
    if(buf == "")
      break;
    myfile << buf;
  }
  myfile.close();
  close(newSocket);
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
  pthread_t tid[10];
  int i = 0;
  while(i < 10){
    //accpet
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    cout << "Accept a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << endl;
    //make thread
    int result = pthread_create(&tid[i], NULL, socketThread,&clientSockfd);
    if (result < 0)
      cout << "Failed to create thread from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << endl;
    else
    i++;
  }
  for(int i = 0; i < 10; i++)
    {
      pthread_join(tid[i], NULL);
    }
  return 0;
}
