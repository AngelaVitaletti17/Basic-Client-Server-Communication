#include <iostream>
#include <cstdio>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctime>
#include <sys/time.h>
#include <stdint.h>
#include <iomanip>
using namespace std;

int main (int argc, const  char* argv[]){
  string server_ip, msg, new_msg;
  struct sockaddr_in client, server;
  int client_size = sizeof(client);
  int server_size = sizeof(server);
  int server_port;
  double avg_RTT, avg_OTT, min_RTT, min_OTT, max_RTT, max_OTT = 0;
  double p_lost = 0;
  double OTT_T[10] = {0};
  double RTT_T[10] = {0};

  //Set up Client Information
  client.sin_family = AF_INET;

  //Open a socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  //Take in parameters
  if (argc > 0)
    {
      server_ip = argv[1];
    }
  if (argc > 1)
    server_port = atoi(argv[2]);

  //Set up Server Information
  const char* s = server_ip.c_str();
  server.sin_family = AF_INET;
  server.sin_port = htons(server_port);
  inet_pton(AF_INET, s, &server.sin_addr);

  //Output some information
  cout << "Pinging " << server_ip << ", " << server_port << endl;

  struct message {
    int32_t seq_num;
    suseconds_t res_time;
  };
  //For assembling OTT and RTT arrays
  int arr_index = 0;

  for (int i = 1; i <= 10; i++){
    int32_t  m;
    struct message r;
    struct timeval timeout, send_t, recv_t, result;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    m = htonl(i);

    gettimeofday(&send_t, NULL);
    sendto(sock, &m, sizeof(m), 0, (const sockaddr* ) &server, sizeof(server));

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0
	&& recvfrom(sock, &r, sizeof(r), 0, (sockaddr*) &server, (socklen_t * )&server_size) < 0)
      {
	cout << "Ping message number " << i << " timed out" <<  endl;
	p_lost++;
      }
    else{
      recvfrom(sock, &r, sizeof(r), 0, (sockaddr*) &server, (socklen_t * )&server_size);
      gettimeofday(&recv_t, NULL);
      timersub(&send_t,&recv_t, &result);
      double RTT = result.tv_usec;
      double  OTT = r.res_time - send_t.tv_usec;
      OTT_T[arr_index] = OTT; RTT_T[arr_index] = RTT;
      arr_index++;
      avg_OTT += OTT;
      avg_RTT += RTT;
      RTT = RTT / 1000000;
      OTT = OTT / 1000000;
      cout << "Ping message number " << ntohl(r.seq_num) << " RTT (OTT): " << fixed << setprecision(8) <<  RTT  << " (" << fixed <<  setprecision(8) << OTT << ") secs" << endl;
    }

  }
  double per_lost = p_lost / 10;
  cout << "Packets received: " << fixed << setprecision(0) << 10 - p_lost << "\nPackets lost: " << p_lost << "\nPercentage lost: " << per_lost * 100  << "%" << endl;
  //Max OTT
  for (int i = 0; OTT_T[i] != 0; i++){
    if (i != 0 && OTT_T[i] > OTT_T[i - 1])
      max_OTT = OTT_T[i];
    else if (i == 0)
      max_OTT = OTT_T[0];
  }
  //Max RTT
  for (int j = 0; RTT_T[j] != 0; j++){
    if (j != 0 && RTT_T[j] > RTT_T[j - 1])
      max_RTT = RTT_T[j];
    else if (j == 0)
      max_RTT = RTT_T[0];
  }

  //Min OTT
  for (int i = 0; OTT_T[i] != 0; i++){
    if (i != 0 && OTT_T[i] < OTT_T[i - 1])
      min_OTT = OTT_T[i];
    else if (i == 0)
      min_OTT = OTT_T[0];
  }
  //Min RTT
  for (int j = 0; RTT_T[j] != 0; j++){
    if (j != 0 && RTT_T[j] < RTT_T[j - 1])
      min_RTT = RTT_T[j];
    else if (j == 0)
      min_RTT = RTT_T[0];
  }

  
  cout << "Max RTT (OTT): " << fixed << setprecision(8) << max_RTT / 1000000 << " (" << max_OTT / 1000000 << ") " << "\nMin RTT (OTT): " << min_RTT /1000000 <<  " (" << min_OTT / 1000000 << ") " << "\nAverage RTT (OTT): "
       << setprecision(8) << avg_RTT / 10000000 << " (" << setprecision(8) <<  avg_OTT / 10000000 << ") " << endl;
  return(0);
}
