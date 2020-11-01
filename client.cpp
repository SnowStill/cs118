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

  string path = string(argv[3]);
  cout << path << endl;
}  
