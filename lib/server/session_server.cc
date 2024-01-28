#include "session_server.h"

session::session(tcp::socket socket)
    : socket(std::move(socket))
{
    std::cout << "new session connected " << std::endl;
}

session::~session()
{
    std::cout << "session closed " << std::endl;
    delete reply;
}

void session::start()
{
    typedef struct
    { // Little endian
        char number[5];
        char time[5];
        char port[5];
    } pkt_t;
    pkt_t packet;

    socket.read_some(buffer(&packet, sizeof(pkt_t)));
    
    time = atoi(packet.time);
    number_ = atoi(packet.number);
    port = 10000 + atoi(packet.port);
    std::cout << "time:" << time << std::endl;
    std::cout << "number:" << number_ << std::endl;
    std::cout << "port:" << port << std::endl;
    ip::tcp::endpoint remote_ep = socket.remote_endpoint();
    ip::address remote_ad = remote_ep.address();
    client_ip = remote_ad.to_string();
    std::cout << client_ip << std::endl;
    int num = 0;
    for (; num < number_; num++)
    {
        threadpool_down.emplace_back(&session::do_download, this, port);
    }

    for (auto &thread : threadpool_down)
    {
        if (thread.joinable()) // thread is not nullable so thread != nullptr -> compile error
            thread.join();
    }

    for (; num < number_ * 2; num++)
    {
        threadpool_up.emplace_back(&session::do_upload, this, port);
    }

    for (auto &thread : threadpool_up)
    {
        if (thread.joinable()) // thread is not nullable so thread != nullptr -> compile error
            thread.join();
    }

    //   std::thread thread1(&session::do_download,this,port+num);
    // thread1.join();
    do_endtoend(port);
}
void session::do_download(int port_thread)
{
    std::cout << "iam thread" << std::endl;
    std::cout << port_thread << std::endl;
    // io_service ioservice;
    tcp::socket tcp_socket(ioservice);
    tcp::resolver resolver(ioservice);
    tcp::resolver::query q{client_ip, std::to_string(port_thread)};
    try
    { //
        std::cout << "check " << std::endl;

        
        socket.read_some(buffer(reply, reply->size())); // to avoid where server's connect() is executed before client's accept() for new tcp connections
        sleep(2);

        connect(tcp_socket, resolver.resolve(q));
        std::cout << "download connected " << std::endl;

        reply->fill('r');

        steady_timer timer{ioservice, std::chrono::seconds{time}};
        timer.async_wait([this](const boost::system::error_code &ec)
                         { stop = 0; });
        // std::thread thread1{[&ioservice]()
        //                     { ioservice.run(); }};
        std::thread thread1{[this]()
                            { ioservice.run(); }};
        thread1.detach();
        std::cout << "stop:" << stop << std::endl;
        while (stop)
            tcp_socket.write_some(buffer(reply, reply->size()));
        std::cout << "write" << std::endl;
        tcp_socket.shutdown(tcp::socket::shutdown_send);
        // down.clear();
        // // down="EOF";
        // //  auto write_length =tcp_socket.write_some(buffer(down, down.length()));
        reply->fill(0);
        tcp_socket.read_some(buffer(reply, reply->size()));
        // std::cout << "EOF :" << write_length << std::endl;

        tcp_socket.close();
    }
    catch (std::exception const &e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}
void session::do_upload(int port_thread)
{
    int sum = 0;
    // io_service ioservice;
    tcp::socket tcp_socket(ioservice);
    tcp::resolver resolver(ioservice);
    tcp::resolver::query q{client_ip, std::to_string(port_thread)};

    try
    {

        connect(tcp_socket, resolver.resolve(q));
        std::cout << "upload connected " << std::endl;

        auto read_length = 1;
        while (read_length != 0)
        {
            read_length = tcp_socket.read_some(buffer(reply, reply->size()));
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
void session::do_endtoend(int port_thread)
{
    // io_service ioservice;
    try
    {
        std::cout << port_thread << std::endl;
        udp::socket udp_socket{ioservice};
        udp_socket.open(udp::v4());

        udp_socket.send_to(
            boost::asio::buffer("Hello world!"),
            udp::endpoint{boost::asio::ip::make_address(client_ip), (short unsigned int)(port_thread)});
        std::cout << "send udp" << std::endl;
        udp::endpoint client;
        int i = 0;
        while (i < 5)
        {
            udp_socket.receive_from(
                boost::asio::buffer(reply_delay, reply_delay.size()),
                client);
            udp_socket.send_to(
                boost::asio::buffer(reply_delay, reply_delay.size()), client);
            i++;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // std::cout.write(recv_buf.data(), len);
}