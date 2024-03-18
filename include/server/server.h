
#ifndef SEVER_H
#define SEVER_H

#include "session_server.h"

using namespace boost::asio;
using namespace boost::asio::ip;

class Server
{

public:
    Server(io_service &ioservice, short port);

private:
    void do_accept();

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    vector<thread> threadpool_;
};

#endif