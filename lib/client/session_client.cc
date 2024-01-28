#include "session_client.h"

void fail(beast::error_code ec, char const *what)
{
   std::cerr << what << ": " << ec.message() << "\n";
}

session::session(tcp::socket socket, int port, std::string time, std::string number)
    : socket(std::move(socket))
{
   this->port = port;
   this->time = atoi(time.c_str());
   this->number_ = atoi(number.c_str());
   std::cout << "new session connected " << std::endl;
   reply.fill('r');
}

session::~session()
{

   std::cout << "session closed " << std::endl;
}

void session::start()
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
         socket.write_some(buffer(reply, reply.size()));
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

void session::progressbar_down()
{
   const char bar = '=';          // 프로그레스바 문자
   const char blank = ' ';        // 비어있는 프로그레스바 문자
   const int LEN = 20;            // 프로그레스바 길이
   const int MAX = 100;           // 진행작업 최대값
   int count = 0;                 // 현재 진행된 작업
   int i;                         // 반복문 전용 변수
   float tick = (float)100 / LEN; // 몇 %마다 프로그레스바 추가할지 계산
   printf("%0.2f%% 마다 bar 1개 출력\n\n", tick);
   int bar_count; // 프로그레스바 갯수 저장 변수
   float percent; // 퍼센트 저장 변수
   while (count <= MAX)
   {
      printf("\r%d/%d [", count, MAX);    // 진행 상태 출력
                                          //   fflush(stdout);
      percent = (float)count / MAX * 100; // 퍼센트 계산
      bar_count = percent / tick;         // 프로그레스바 갯수 계산
      for (i = 0; i < LEN; i++)
      { // LEN길이의 프로그레스바 출력
         if (bar_count > i)
         { // 프로그레스바 길이보다 i가 작으면
            printf("%c", bar);
         }
         else
         { // i가 더 커지면
            printf("%c", blank);
         }
      }
      printf("] %0.2f%%", percent); // 퍼센트 출력
      double throughput_down = ((total_downloaddata * 8) / 1000000.0d) / time;
      if (count == MAX - 1)
      {
         while (delay_down != number_)
         {
         }
         std::cout << ": download throughput = " << throughput_down << "Mbps";
      }
      else
         std::cout << ": download throughput = " << throughput_down << "Mbps";
      fflush(stdout);
      count++;              // 카운트 1증가
      usleep(time * 10000); // spended time per 1/100
   }
   printf(" done!\n\n");
   // system("pause"); // 프로그램 종료 전 일시정지
}
void session::progressbar_up()
{
   const char bar = '=';          // 프로그레스바 문자
   const char blank = ' ';        // 비어있는 프로그레스바 문자
   const int LEN = 20;            // 프로그레스바 길이
   const int MAX = 100;           // 진행작업 최대값
   int count = 0;                 // 현재 진행된 작업
   int i;                         // 반복문 전용 변수
   float tick = (float)100 / LEN; // 몇 %마다 프로그레스바 추가할지 계산
   // printf("%0.2f%% 마다 bar 1개 출력\n\n", tick);
   int bar_count; // 프로그레스바 갯수 저장 변수
   float percent; // 퍼센트 저장 변수
   while (count <= MAX)
   {
      printf("\r%d/%d [", count, MAX);    // 진행 상태 출력
                                          //   fflush(stdout);
      percent = (float)count / MAX * 100; // 퍼센트 계산
      bar_count = percent / tick;         // 프로그레스바 갯수 계산
      for (i = 0; i < LEN; i++)
      { // LEN길이의 프로그레스바 출력
         if (bar_count > i)
         { // 프로그레스바 길이보다 i가 작으면
            printf("%c", bar);
         }
         else
         { // i가 더 커지면
            printf("%c", blank);
         }
      }
      printf("] %0.2f%% ", percent); // 퍼센트 출력
      double throughput_up = ((total_uploaddata * 8) / 1000000.0d) / time;

      if (count == MAX - 1)
      {
         while (delay_up != number_)
         {
         }
         std::cout << ": upload throughput =  " << throughput_up << "Mbps";
      }
      else
         std::cout << ": upload throughput =  " << throughput_up << "Mbps";
      fflush(stdout);
      count++;              // 카운트 1증가
      usleep(time * 10000); // // spended time per 1/100
   }
   printf(" done!\n\n");
   // system("pause"); // 프로그램 종료 전 일시정지
}

void session::do_download(beast::error_code ec, tcp::socket tcp_socket)
{
   if (ec)
   {
      fail(ec, "accept");
   }
   else
   {

      long sum = 0;

      try
      {

         // std::cout << "download connected " << std::endl;

         auto read_length = 1;
         try
         {
            while (read_length != 0)
            {
               read_length = tcp_socket.read_some(buffer(reply, reply.size()));
               // std::cout.write(reply.data(), read_length);
               sum += read_length;
               // std::cout.write(reply.data(), read_length);
            }
         }
         catch (std::exception const &e)
         {
            // std::cerr << "download terminated: " << e.what() << std::endl;
         }
      }
      catch (std::exception const &e)
      {
         std::cerr << "exception: " << e.what() << std::endl;
      }

      // std::cout << "down load data size : " << sum << std::endl;
      mtx_download.lock();
      total_downloaddata += sum;
      mtx_download.unlock();
      std::string check = "EOF";
      tcp_socket.write_some(buffer(check, check.length()));
      tcp_socket.close();
   }
   mtx_delay_down.lock();
   delay_down++;
   mtx_delay_down.unlock();
}
void session::do_upload(beast::error_code ec, tcp::socket tcp_socket)
{
   if (ec)
   {
      fail(ec, "accept");
   }
   else
   {

      long sum = 0;
      try
      {

         reply.fill('r');
         steady_timer timer{ioservice, std::chrono::seconds{time}};
         timer.async_wait([this](const boost::system::error_code &ec)
                          { stop = 0; });
         std::thread thread1{[this]()
                             { ioservice.run(); }};
         thread1.detach();
         // std::cout << "stop:" << stop << std::endl;
         while (stop)
         {
            auto write_length = tcp_socket.write_some(buffer(reply, reply.size()));
            sum += write_length;
         }
         // std::cout << "write" << std::endl;
         tcp_socket.shutdown(tcp::socket::shutdown_send);
         // down.clear();
         // // down="EOF";
         // //  auto write_length =tcp_socket.write_some(buffer(down, down.length()));
         reply.fill(0);
         tcp_socket.read_some(buffer(reply, reply.size()));
         // std::cout << "EOF :" << write_length << std::endl;

         tcp_socket.close();
         // std::cout << "upload data size : " << sum << std::endl;
         mtx_upload.lock();
         total_uploaddata += sum;
         mtx_upload.unlock();
         // std::cout << "upload terminated " << std::endl;
      }
      catch (std::exception const &e)
      {
         std::cerr << "exception: " << e.what() << std::endl;
      }
   }

   mtx_delay_up.lock();
   delay_up++;
   mtx_delay_up.unlock();
}
void session::do_endtoend(udp::socket &udp_socket)
{
   try
   {
      udp::endpoint server;
      reply.fill(0);
      udp_socket.receive_from(
          boost::asio::buffer(reply, reply.size()),
          server);
      // std::cout << server << ": " << reply.data() << '\n';


      reply_delay.fill('r');
      reply.fill(0);

      int sum_send = 0;
      int sum_receive = 0;
      int k = 0;
      int t1, t2;
      while (k < 5)
      {
         t1 = GetCurrentUsec();
         auto send_length = udp_socket.send_to(
             boost::asio::buffer(reply_delay, reply_delay.size()), server);
         auto receive_length = udp_socket.receive_from(
             boost::asio::buffer(reply_delay, reply_delay.size()),
             server);
         t2 = GetCurrentUsec();
         sum_send += send_length;
         sum_receive += receive_length;
         std::cout << k + 1 << " : end to end delay : " << (t2 - t1) << " usec." << '\n';
         k++;
      }

      //  std::cout  << "end to end delay : "<< (t2-t1) <<" usec." <<'\n';

      std::cout << "sum_send : " << sum_send << "bytes" << '\n';
      std::cout << "sum_receive : " << sum_receive << "bytes" << '\n';
      if (sum_send == sum_receive)
         printf(" no packet loss by UDP end to end delay\n");
      else
         printf(" there is packet loss by UDP end to end delay\n");
   }
   catch (std::exception &e)
   {
      std::cerr << e.what() << std::endl;
   }
}
int session::GetCurrentUsec()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return tv.tv_usec;
}