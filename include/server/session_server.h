#ifndef SESSION_H
#define SESSION_H
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>

using namespace std;
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
    : public enable_shared_from_this<Session>
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
    vector<thread> threadpool_down_;
    vector<thread> threadpool_up_;
    // string data;
    array<char, 20> *reply_ = new array<char, 20>;
    array<char, 1024> reply_delay_;
    int time_; // excution time to download or upload
    string client_ip_;
    int number_; // the number of thread
    int port_;
    int stop_ = 1;
};

#endif