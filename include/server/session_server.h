#ifndef SESSION_H
#define SESSION_H
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>

namespace beast = boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef struct
{ // Little endian
    char number[5];
    char time[5];
    char port[5];
} pkt_t;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket);
    ~session();

    void start();

private:
    void do_download(int port_thread);

    void do_upload(int port_thread);

    void do_endtoend(int port_thread);
    
    io_service ioservice;
    tcp::socket socket;
    std::vector<std::thread> threadpool_down;
    std::vector<std::thread> threadpool_up;
    // std::string data;
    std::array<char, 20>* reply = new std::array<char, 20>;
    std::array<char, 1024> reply_delay;
    int time; // excution time to download or upload
    std::string client_ip;
    int number_; // the number of thread
    int port;
    int stop = 1;
};

#endif