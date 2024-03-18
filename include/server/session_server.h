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

class Session
    : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket);
    ~Session();

    void Start();

private:
    void DoDownload(int port_thread);

    void DoUpload(int port_thread);

    void DoEndToEnd(int port_thread);

    io_service ioservice_;
    tcp::socket socket_;
    std::vector<std::thread> threadpool_down_;
    std::vector<std::thread> threadpool_up_;
    // std::string data;
    std::array<char, 20> *reply_ = new std::array<char, 20>;
    std::array<char, 1024> reply_delay_;
    int time_; // excution time to download or upload
    std::string client_ip_;
    int number_; // the number of thread
    int port_;
    int stop_ = 1;
};

#endif