#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread.hpp>
#include <string>
#include <ctime>
#include <list>
#include <iostream>
#include <boost/asio.hpp>
#include <array>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "session_server.h"

using namespace boost::asio;
using namespace boost::asio::ip;


class Server
{
public:
    Server(io_service &ioservice, short port)
        : acceptor(ioservice, tcp::endpoint(tcp::v4(), port)),
          socket(ioservice)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor.async_accept(socket,
                              [this](boost::system::error_code ec)
                              {
                                  if (!ec)
                                  {
                                      threadpool.emplace_back(&Session::Start, std::make_shared<Session>(std::move(socket)));
                                      threadpool.back().detach();

                                      //   std::make_shared<Session>(std::move(socket))->Start();
                                  }

                                  do_accept();
                              });
    }

    tcp::acceptor acceptor;
    tcp::socket socket;
    std::vector<std::thread> threadpool;
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
      printf("Usage : %s <port>\n", argv[0]);
      exit(1);
    }
    io_service ioservice;

    Server s(ioservice, std::atoi(argv[1]));
    ioservice.run();

    return 0;
}