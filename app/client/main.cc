
#include <string>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/time.h>
#include "session_client.h"





void run_client(std::string host, short const port, std::string number, std::string time)
{
   try
   {
      io_service ioservice;
      tcp::socket tcp_socket(ioservice);
      tcp::resolver resolver(ioservice);
      tcp::resolver::query q{host, std::to_string(port)};
      std::srand(std::time(NULL));
      // std::cout << "rand : " << rand() << std::endl;
      int port = rand() % 400 + 1;

      std::cout << port << std::endl;
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
      strcpy(packet.port, std::to_string(port).c_str());

      tcp_socket.write_some(buffer(&packet, sizeof(pkt_t)));
      std::vector<std::thread> threadpool;

      std::make_shared<session>(std::move(tcp_socket), port + 10000, time, number)->start();
      // threadpool.emplace_back(std::thread(&session::start, std::make_shared<session>(std::move(tcp_socket), port + 20000, time,number)));
      // threadpool.back().join();
   }
   catch (std::exception const &e)
   {
      std::cerr << "exception: " << e.what() << std::endl;
   }
}

int main(int argc, char *argv[])
{

   if (argc != 5)
   {
      printf("Usage : %s <IP> <port> <the number of session> <measure time>\n", argv[0]);
      exit(1);
   }
   run_client(argv[1], atoi(argv[2]), argv[3], argv[4]);
}
