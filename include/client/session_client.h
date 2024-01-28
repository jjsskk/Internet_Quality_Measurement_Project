#ifndef SESSION_H
#define SESSION_H
#include <iostream>
#include <array>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include "globalVariable.h"
#include <thread>

namespace beast = boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

void fail(beast::error_code ec, char const *what);

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket, int port, std::string time, std::string number);

    ~session();
    void start();

private:
    void progressbar_down();

    void progressbar_up();

    void do_download(beast::error_code ec, tcp::socket tcp_socket);

    void do_upload(beast::error_code ec, tcp::socket tcp_socket);

    void do_endtoend(udp::socket &udp_socket);

    int GetCurrentUsec();

    io_service ioservice;
    std::vector<tcp::acceptor> acceptorpool;
    std::vector<std::thread> threadpool;
    // std::vector<std::thread> threadpool_down;
    // std::vector<std::thread> threadpool_up;
    tcp::socket socket;
    std::array<char, 20> reply;
    std::array<char, 1024> reply_delay;
    int time;    // excution time to download or upload
    int number_; // the number of thread
    int port;
    int stop = 1;
};

#endif