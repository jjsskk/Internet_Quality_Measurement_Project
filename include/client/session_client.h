#ifndef SESSION_H
#define SESSION_H
#include <iostream>
#include <array>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include "globalVariable.h"
#include <thread>

using namespace std;

namespace beast = boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

void Fail(beast::error_code ec, char const *what);

class Session
    : public enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, int port, string time, string number);

    ~Session();
    void Start();

private:
    void ProgressbarDown();

    void ProgressbarUp();

    void DoDownload(beast::error_code ec, tcp::socket tcp_socket);

    void DoUpload(beast::error_code ec, tcp::socket tcp_socket);

    void DoEndToEnd(udp::socket &udp_socket);

    int GetCurrentUsec();

    io_service ioservice_;
    vector<tcp::acceptor> acceptor_pool_;
    vector<thread> threadpool_;
 
    tcp::socket socket_;
    array<char, 20> reply_;
    array<char, 1024> reply_delay_;
    int time_;    // excution time to download or upload
    int number_; // the number of thread
    int port_;
    int stop_ = 1;
};

#endif