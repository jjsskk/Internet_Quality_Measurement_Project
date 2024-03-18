#include "session_server.h"

Session::Session(tcp::socket socket)
    : socket_(move(socket))
{
    cout << "new Session connected " << endl;
}

Session::~Session()
{
    cout << "Session closed " << endl;
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
    cout << "time:" << time_ << endl;
    cout << "number:" << number_ << endl;
    cout << "port:" << port_ << endl;
    ip::tcp::endpoint remote_ep = socket_.remote_endpoint();
    ip::address remote_ad = remote_ep.address();
    client_ip_ = remote_ad.to_string();
    cout << client_ip_ << endl;
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
    cout << "iam thread" << endl;
    cout << port_thread << endl;
    
    tcp::socket tcp_socket(ioservice_);
    tcp::resolver resolver(ioservice_);
    tcp::resolver::query q{client_ip_, to_string(port_thread)};
    try
    {
        cout << "check " << endl;

        socket_.read_some(buffer(reply_, reply_->size())); // to avoid where server's connect() is executed before client's accept() for new tcp connections
        sleep(2);

        connect(tcp_socket, resolver.resolve(q));
        cout << "download connected " << endl;

        reply_->fill('r');

        steady_timer timer{ioservice_, std::chrono::seconds{time_}};
        timer.async_wait([this](const boost::system::error_code &ec)
                         { stop_ = 0; });
   
        thread thread1{[this]()
                            { ioservice_.run(); }};
        thread1.detach();
        cout << "stop:" << stop_ << endl;
        while (stop_)
            tcp_socket.write_some(buffer(reply_, reply_->size()));
        cout << "write" << endl;
        tcp_socket.shutdown(tcp::socket::shutdown_send);
        reply_->fill(0);
        tcp_socket.read_some(buffer(reply_, reply_->size()));

        tcp_socket.close();
    }
    catch (exception const &e)
    {
        cerr << "exception: " << e.what() << endl;
    }
}
void Session::DoUpload(int port_thread)
{
    int sum = 0;
    tcp::socket tcp_socket(ioservice_);
    tcp::resolver resolver(ioservice_);
    tcp::resolver::query q{client_ip_, to_string(port_thread)};

    try
    {

        connect(tcp_socket, resolver.resolve(q));
        cout << "upload connected " << endl;

        auto read_length = 1;
        while (read_length != 0)
        {
            read_length = tcp_socket.read_some(buffer(reply_, reply_->size()));
            sum += read_length;
        }
    }
    catch (exception const &e)
    {
        cerr << "upload terminated: " << e.what() << endl;
    }

    cout << "upload data size : " << sum << endl;
    string check = "EOF";
    tcp_socket.write_some(buffer(check, check.length()));
    tcp_socket.close();
}
void Session::DoEndToEnd(int port_thread)
{
    try
    {
        cout << port_thread << endl;
        udp::socket udp_socket{ioservice_};
        udp_socket.open(udp::v4());

        udp_socket.send_to(
            boost::asio::buffer("Hello world!"),
            udp::endpoint{boost::asio::ip::make_address(client_ip_), (short unsigned int)(port_thread)});
        cout << "send udp" << endl;
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
    catch (exception &e)
    {
        cerr << e.what() << '\n';
    }

    // cout.write(recv_buf.data(), len);
}