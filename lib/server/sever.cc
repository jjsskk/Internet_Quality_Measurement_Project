#include "server.h"

Server::Server(io_service &ioservice, short port)
    : acceptor_(ioservice, tcp::endpoint(tcp::v4(), port)),
      socket_(ioservice)
{
    do_accept();
}

void Server::do_accept()
{
    acceptor_.async_accept(socket_,
                          [this](boost::system::error_code ec)
                          {
                              if (!ec)
                              {
                                  threadpool_.emplace_back(&Session::Start, make_shared<Session>(move(socket_)));
                                  threadpool_.back().detach();

                                  //   make_shared<Session>(move(socket_))->Start();
                              }

                              do_accept();
                          });
}