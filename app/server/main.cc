#include <boost/asio/io_service.hpp>
#include <iostream>
#include "session_server.h"
#include "server.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
      printf("Usage : %s <port>\n", argv[0]);
      exit(1);
    }
    io_service ioservice;

    Server s(ioservice, atoi(argv[1]));
    ioservice.run();

    return 0;
}