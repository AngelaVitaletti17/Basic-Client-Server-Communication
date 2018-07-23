#include <iostream>
#include <cstdio>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctime>
#include <stdint.h>
#include <sys/time.h>
using namespace std;

int main(int argc, const char* argv[]){
  string ip = "127.0.0.1"; //Use loopback for server
  int port = 13000; //Fixed port to be used
  int sock = socket(AF_INET, SOCK_DGRAM, 0); //Open a socket
  struct sockaddr_in client, server;

  //For random number
  srand(time(NULL));

  //Set up Client Information
  int client_size = sizeof(client);
  client.sin_family = AF_INET;
  client.sin_port = htons(port);
  //inet_pton(AF_INET, INADDR_ANY, &client.sin_addr);

  //Set up Server Information
  int server_size = sizeof(server);
  const char* s_ip = ip.c_str();
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  inet_pton(AF_INET, s_ip, &server.sin_addr);

  //Bind the socket
  bind(sock, (const sockaddr *) &server, sizeof(server));

  //Output some stuff
  cout << "The server is ready to receive on port: " << port << endl;

  //Incoming Message
  int32_t inc_msg;

  //Response Ping
  struct message{
    int32_t msg_echo;
    suseconds_t timestamp;
  };

  struct timeval recv_t;

  //Wait for a message and respond (maybe)
  while (true){
    struct message m;
    int rand_rspnd = rand() % 10;

    //Did we receive a message?
    if (recvfrom(sock, &inc_msg, sizeof(inc_msg), 0, (sockaddr*) &client, (socklen_t*) &client_size) != -1)
      {
	gettimeofday(&recv_t, NULL);
	inc_msg = ntohl(inc_msg);
	//Should we respond?
	if (rand_rspnd < 4)
	  cout << "Message with sequence number " << inc_msg << " dropped" << endl;
	else{
	  cout << "Responding to ping request with the sequence number " << inc_msg << " received at " << recv_t.tv_sec + recv_t.tv_usec << endl;
	  m.msg_echo = htonl(inc_msg);
	  m.timestamp = recv_t.tv_usec;
	  sendto(sock, &m, sizeof(m), 0, (const sockaddr*) &client, sizeof(client));
	}
      }



  }

  return 0;
}
