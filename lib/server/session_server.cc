#include "session_server.h"

Session::Session(tcp::socket socket)
    : socket_(std::move(socket))
{
    std::cout << "new Session connected " << std::endl;
}

Session::~Session()
{
    std::cout << "Session closed " << std::endl;
    delete reply_;
}

void Session::Start()
{
    typedef struct
    { // Little endian
        char number[5];
        char time[5];
        char port[5];
    } pkt_t;
    pkt_t packet;

    socket_.read_some(buffer(&packet, sizeof(pkt_t)));

    time_ = atoi(packet.time);
    number_ = atoi(packet.number);
    port_ = 10000 + atoi(packet.port);
    std::cout << "time:" << time_ << std::endl;
    std::cout << "number:" << number_ << std::endl;
    std::cout << "port:" << port_ << std::endl;
    ip::tcp::endpoint remote_ep = socket_.remote_endpoint();
    ip::address remote_ad = remote_ep.address();
    client_ip_ = remote_ad.to_string();
    std::cout << client_ip_ << std::endl;
    int num = 0;
    for (; num < number_; num++)
    {
        threadpool_down_.emplace_back(&Session::DoDownload, this, port_);
    }

    for (auto &thread : threadpool_down_)
    {
        if (thread.joinable()) // thread is not nullable so thread != nullptr -> compile error
            thread.join();
    }

    for (; num < number_ * 2; num++)
    {
        threadpool_up_.emplace_back(&Session::DoUpload, this, port_);
    }

    for (auto &thread : threadpool_up_)
    {
        if (thread.joinable()) // thread is not nullable so thread != nullptr -> compile error
            thread.join();
    }

    DoEndToEnd(port_);
}
void Session::DoDownload(int port_thread)
{
    std::cout << "iam thread" << std::endl;
    std::cout << port_thread << std::endl;
    
    tcp::socket tcp_socket(ioservice_);
    tcp::resolver resolver(ioservice_);
    tcp::resolver::query q{client_ip_, std::to_string(port_thread)};
    try
    {
        std::cout << "check " << std::endl;

        socket_.read_some(buffer(reply_, reply_->size())); // to avoid where server's connect() is executed before client's accept() for new tcp connections
        sleep(2);

        connect(tcp_socket, resolver.resolve(q));
        std::cout << "download connected " << std::endl;

        reply_->fill('r');

        steady_timer timer{ioservice_, std::chrono::seconds{time_}};
        timer.async_wait([this](const boost::system::error_code &ec)
                         { stop_ = 0; });
   
        std::thread thread1{[this]()
                            { ioservice_.run(); }};
        thread1.detach();
        std::cout << "stop:" << stop_ << std::endl;
        while (stop_)
            tcp_socket.write_some(buffer(reply_, reply_->size()));
        std::cout << "write" << std::endl;
        tcp_socket.shutdown(tcp::socket::shutdown_send);
        reply_->fill(0);
        tcp_socket.read_some(buffer(reply_, reply_->size()));

        tcp_socket.close();
    }
    catch (std::exception const &e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}
void Session::DoUpload(int port_thread)
{
    int sum = 0;
    tcp::socket tcp_socket(ioservice_);
    tcp::resolver resolver(ioservice_);
    tcp::resolver::query q{client_ip_, std::to_string(port_thread)};

    try
    {

        connect(tcp_socket, resolver.resolve(q));
        std::cout << "upload connected " << std::endl;

        auto read_length = 1;
        while (read_length != 0)
        {
            read_length = tcp_socket.read_some(buffer(reply_, reply_->size()));
            sum += read_length;
        }
    }
    catch (std::exception const &e)
    {
        std::cerr << "upload terminated: " << e.what() << std::endl;
    }

    std::cout << "upload data size : " << sum << std::endl;
    std::string check = "EOF";
    tcp_socket.write_some(buffer(check, check.length()));
    tcp_socket.close();
}
void Session::DoEndToEnd(int port_thread)
{
    try
    {
        std::cout << port_thread << std::endl;
        udp::socket udp_socket{ioservice_};
        udp_socket.open(udp::v4());

        udp_socket.send_to(
            boost::asio::buffer("Hello world!"),
            udp::endpoint{boost::asio::ip::make_address(client_ip_), (short unsigned int)(port_thread)});
        std::cout << "send udp" << std::endl;
        udp::endpoint client;
        int i = 0;
        while (i < 5)
        {
            udp_socket.receive_from(
                boost::asio::buffer(reply_delay_, reply_delay_.size()),
                client);
            udp_socket.send_to(
                boost::asio::buffer(reply_delay_, reply_delay_.size()), client);
            i++;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // std::cout.write(recv_buf.data(), len);
}