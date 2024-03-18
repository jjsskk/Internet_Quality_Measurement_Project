
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/time.h>
#include "session_client.h"


void RunClient(string host, short const port, string number, string time)
{
   try
   {
      io_service ioservice;
      tcp::socket tcp_socket(ioservice);
      tcp::resolver resolver(ioservice);
      tcp::resolver::query q{host, to_string(port)};
      srand(std::time(NULL));
      // cout << "rand : " << rand() << endl;
      int port = rand() % 400 + 1;

      cout << port << endl;
      connect(tcp_socket, resolver.resolve(q));
      typedef struct
      { // Little endian
         char number[5];
         char time[5];
         char port[5];
      } pkt_t;
      pkt_t packet;
      strcpy(packet.number, number.c_str());
      strcpy(packet.time, time.c_str());
      strcpy(packet.port, to_string(port).c_str());

      tcp_socket.write_some(buffer(&packet, sizeof(pkt_t)));
      vector<thread> threadpool;

      make_shared<Session>(move(tcp_socket), port + 10000, time, number)->Start();
      // threadpool.emplace_back(thread(&Session::Start, make_shared<Session>(move(tcp_socket), port + 20000, time,number)));
      // threadpool.back().join();
   }
   catch (exception const &e)
   {
      cerr << "exception: " << e.what() << endl;
   }
}

int main(int argc, char *argv[])
{

   if (argc != 5)
   {
      printf("Usage : %s <IP> <port> <the number of Session> <measure time>\n", argv[0]);
      exit(1);
   }
   RunClient(argv[1], atoi(argv[2]), argv[3], argv[4]);
}
