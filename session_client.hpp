class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket, int port, std::string time, std::string number)
        : socket(std::move(socket))
    {
        this->port = port;
        this->time = atoi(time.c_str());
        this->number_ = atoi(number.c_str());
        std::cout << "new session connected " << std::endl;
        reply.fill('r');
    }

    ~session()
    {

        std::cout << "session closed " << std::endl;
    }

    void start()
    {
        udp::socket udp_socket(ioservice,
                               udp::endpoint{boost::asio::ip::udp::v4(), (short unsigned int)(port)});

        int pool_num = 0;
        acceptorpool.emplace_back(ioservice, tcp::endpoint(tcp::v4(), port));
        for (; pool_num < number_ * 2; pool_num++)
        {
            // socketpool.emplace_back(tcp_socket(ioservice));
            if (pool_num < number_)
            {
                acceptorpool.back().async_accept(
                    ioservice,
                    beast::bind_front_handler(
                        &session::do_download,
                        shared_from_this()));

                threadpool.emplace_back(
                    [this]
                    {
                        ioservice.run();
                    });

                threadpool.back().detach();
            }
            else
            {
                acceptorpool.back().async_accept(
                    ioservice,
                    beast::bind_front_handler(
                        &session::do_upload,
                        shared_from_this()));
                auto write_length = socket.write_some(buffer(reply, reply.size()));
                // to avoid where server's connect() in do_download() is executed before client's accept() in do_download() for new tcp connections
            }
        }

        sleep(2);
        progressbar_down();

        progressbar_up();

        double throughput_down = ((total_downloaddata * 8) / 1000000.0d) / time;
        double throughput_up = ((total_uploaddata * 8) / 1000000.0d) / time;
        
        std::cout << "total data that all threads download : " << total_downloaddata << "bytes" << std::endl;
        std::cout << "total data that all threads upload : " << total_uploaddata << "bytes" << std::endl;
        std::cout << "Download throughput applied to all threads : " << throughput_down << "Mbps" << std::endl;
        std::cout << "Upload throughput applied to all threads : " << throughput_up << "Mbps" << std::endl;
        std::cout << "\nDelay measurement using UDP" << std::endl;

        do_endtoend(udp_socket);

    }

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