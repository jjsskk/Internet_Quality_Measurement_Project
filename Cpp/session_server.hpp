

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket(std::move(socket))
    {
        std::cout << "new session connected " << std::endl;
    }

    ~session()
    {
        std::cout << "session closed " << std::endl;
        delete reply;
    }

    void start()
    {
        typedef struct
        { // Little endian
            char number[5];
            char time[5];
            char port[5];
        } pkt_t;
        pkt_t packet;
        auto read_length = socket.read_some(buffer(&packet, sizeof(pkt_t)));
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

        for (auto& thread : threadpool_down)
        {
            if (thread.joinable()) // thread is not nullable so thread != nullptr -> compile error
                thread.join();
        }


        for (; num < number_ * 2; num++)
        {
            threadpool_up.emplace_back(&session::do_upload, this, port);
        }

        for (auto& thread : threadpool_up)
        {
            if (thread.joinable())// thread is not nullable so thread != nullptr -> compile error
                thread.join();
        }

        //   std::thread thread1(&session::do_download,this,port+num);
        // thread1.join();
        do_endtoend(port);
    }

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