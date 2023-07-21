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

        auto write_length = socket.read_some(buffer(reply, reply->size())); // to avoid where server's connect() is executed before client's accept() for new tcp connections
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
            auto write_length = tcp_socket.write_some(buffer(reply, reply->size()));
        std::cout << "write" << std::endl;
        tcp_socket.shutdown(tcp::socket::shutdown_send);
        // down.clear();
        // // down="EOF";
        // //  auto write_length =tcp_socket.write_some(buffer(down, down.length()));
        reply->fill(0);
        write_length = tcp_socket.read_some(buffer(reply, reply->size()));
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
    auto write_length = tcp_socket.write_some(buffer(check, check.length()));
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