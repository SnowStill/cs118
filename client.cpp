#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include "CRC.h"
#include <poll.h>

using namespace std;
int main(int argc, char* argv[]){
  struct hostent *server;
  if(argc < 4){
    cerr << "Error: Invalid number of arguments." << endl;
    return 1;
  }
  const char* host = argv[1];
  server = gethostbyname(host);
  if(server == NULL){
    cerr << "Error: Invalid hostname." << endl;
    return 1;
  }
  int port = atoi(argv[2]);
  if(port < 1 || port > 65536){
    cerr << "Error: Invalid port number." << endl;
    return 1;
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);     // short, network byte order
  bcopy((char *)&addr.sin_addr.s_addr,
	(char *)server->h_addr,
	server->h_length);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    cerr << "Error: Failed to connect to the server" << endl;
    return 1;
  }
  struct pollfd p[1];
  p[0].fd = sockfd;
  p[0].events = POLLOUT | POLLHUP | POLLERR;
  int dc=0;
  string path = string(argv[3]);
  char* input = new char[1024];
  uint8_t* input_crc = new uint8_t[1016];
  ifstream myfile (path.c_str());
  while(1){
    dc = poll(p,1,10000);
    if(dc > 0){
      if(p[0].revents & POLLOUT){
	myfile.read(input, 1016);
	memcpy(input_crc, input, strlen(input));
	CRC crc;
	uint64_t crc_key = htobe64(crc.get_crc_code(input_crc, strlen(input)));
	memcpy(input + strlen(input),&crc_key,8);
	if (send(sockfd, input, strlen(input), 0) == -1) {
	  cerr <<"Error: Failed to send,";
	  return 1;
	}
	if(myfile.eof()){
	  break;
	}
	memset(input,0,1024);
      }
    }
    else if (dc == 0){
      close(sockfd);
      cerr << "Error: Disconnected from server for ideling.";
      exit(1);
    }
  }
  myfile.close();
  close(sockfd);
  return 0;
}  
