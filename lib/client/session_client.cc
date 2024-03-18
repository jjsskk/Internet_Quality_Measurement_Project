#include "session_client.h"

void Fail(beast::error_code ec, char const *what)
{
   cerr << what << ": " << ec.message() << "\n";
}

Session::Session(tcp::socket socket, int port, string time, string number)
    : socket_(move(socket))
{
   port_ = port;
   time_ = atoi(time.c_str());
   number_ = atoi(number.c_str());
   cout << "new Session connected " << endl;
   reply_.fill('r');
}

Session::~Session()
{

   cout << "Session closed " << endl;
}

void Session::Start()
{
   udp::socket udp_socket(ioservice_,
                          udp::endpoint{boost::asio::ip::udp::v4(), (short unsigned int)(port_)});

   int pool_num = 0;
   acceptor_pool_.emplace_back(ioservice_, tcp::endpoint(tcp::v4(), port_));
   for (; pool_num < number_ * 2; pool_num++)
   {
      // socketpool.emplace_back(tcp_socket(ioservice_));
      if (pool_num < number_)
      {
         acceptor_pool_.back().async_accept(
             ioservice_,
             beast::bind_front_handler(
                 &Session::DoDownload,
                 shared_from_this()));

         threadpool_.emplace_back(
             [this]
             {
                ioservice_.run();
             });

         threadpool_.back().detach();
      }
      else
      {
         acceptor_pool_.back().async_accept(
             ioservice_,
             beast::bind_front_handler(
                 &Session::DoUpload,
                 shared_from_this()));
         socket_.write_some(buffer(reply_, reply_.size()));
         // to avoid where server's connect() in DoDownload() is executed before client's accept() in DoDownload() for new tcp connections
      }
   }

   sleep(2);
   ProgressbarDown();

   ProgressbarUp();

   double throughput_down = ((total_downloaddata * 8) / 1000000.0d) / time_;
   double throughput_up = ((total_uploaddata * 8) / 1000000.0d) / time_;

   cout << "total data that all threads download : " << total_downloaddata << "bytes" << endl;
   cout << "total data that all threads upload : " << total_uploaddata << "bytes" << endl;
   cout << "Download throughput applied to all threads : " << throughput_down << "Mbps" << endl;
   cout << "Upload throughput applied to all threads : " << throughput_up << "Mbps" << endl;
   cout << "\nDelay measurement using UDP" << endl;

   DoEndToEnd(udp_socket);
}

void Session::DoDownload(beast::error_code ec, tcp::socket tcp_socket)
{
   if (ec)
   {
      Fail(ec, "accept");
   }
   else
   {
      long sum = 0;
      try
      {

         // cout << "download connected " << endl;
         auto read_length = 1;
         try
         {
            while (read_length != 0)
            {
               read_length = tcp_socket.read_some(buffer(reply_, reply_.size()));
               sum += read_length;
            }
         }
         catch (exception const &e)
         {
            // cerr << "download terminated: " << e.what() << endl;
         }
      }
      catch (exception const &e)
      {
         cerr << "exception: " << e.what() << endl;
      }

      // cout << "down load data size : " << sum << endl;
      mtx_download.lock();
      total_downloaddata += sum;
      mtx_download.unlock();
      string check = "EOF";
      tcp_socket.write_some(buffer(check, check.length()));
      tcp_socket.close();
   }
   mtx_delay_down.lock();
   delay_down++;
   mtx_delay_down.unlock();
}
void Session::DoUpload(beast::error_code ec, tcp::socket tcp_socket)
{
   if (ec)
   {
      Fail(ec, "accept");
   }
   else
   {
      long sum = 0;
      try
      {

         reply_.fill('r');
         steady_timer timer{ioservice_, std::chrono::seconds{time_}};
         timer.async_wait([this](const boost::system::error_code &ec)
                          { stop_ = 0; });
         thread thread1{[this]()
                             { ioservice_.run(); }};
         thread1.detach();
         // cout << "stop_:" << stop_ << endl;
         while (stop_)
         {
            auto write_length = tcp_socket.write_some(buffer(reply_, reply_.size()));
            sum += write_length;
         }
         // cout << "write" << endl;
         tcp_socket.shutdown(tcp::socket::shutdown_send);
         reply_.fill(0);
         tcp_socket.read_some(buffer(reply_, reply_.size()));

         tcp_socket.close();
         mtx_upload.lock();
         total_uploaddata += sum;
         mtx_upload.unlock();
      }
      catch (exception const &e)
      {
         cerr << "exception: " << e.what() << endl;
      }
   }

   mtx_delay_up.lock();
   delay_up++;
   mtx_delay_up.unlock();
}
void Session::DoEndToEnd(udp::socket &udp_socket)
{
   try
   {
      udp::endpoint server;
      reply_.fill(0);
      udp_socket.receive_from(
          boost::asio::buffer(reply_, reply_.size()),
          server);

      reply_delay_.fill('r');
      reply_.fill(0);

      int sum_send = 0;
      int sum_receive = 0;
      int k = 0;
      int t1, t2;
      while (k < 5)
      {
         t1 = GetCurrentUsec();
         auto send_length = udp_socket.send_to(
             boost::asio::buffer(reply_delay_, reply_delay_.size()), server);
         auto receive_length = udp_socket.receive_from(
             boost::asio::buffer(reply_delay_, reply_delay_.size()),
             server);
         t2 = GetCurrentUsec();
         sum_send += send_length;
         sum_receive += receive_length;
         cout << k + 1 << " : end to end delay : " << (t2 - t1) << " usec." << '\n';
         k++;
      }

      //  cout  << "end to end delay : "<< (t2-t1) <<" usec." <<'\n';

      cout << "sum_send : " << sum_send << "bytes" << '\n';
      cout << "sum_receive : " << sum_receive << "bytes" << '\n';
      if (sum_send == sum_receive)
         printf(" no packet loss by UDP end to end delay\n");
      else
         printf(" there is packet loss by UDP end to end delay\n");
   }
   catch (exception &e)
   {
      cerr << e.what() << endl;
   }
}
int Session::GetCurrentUsec()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return tv.tv_usec;
}