#include "ccwu_tcp_server.h"

#include "queued_ccwu_session.h"

namespace cis1
{

namespace cwu
{

using tcp = boost::asio::ip::tcp;

tcp_server::tcp_server(boost::asio::io_context& io_context)
    : acceptor_(io_context)
    , socket_(io_context)
{}

void tcp_server::set_session_acceptor(
            std::function<std::function<void(
                    boost::asio::const_buffer,
                    std::shared_ptr<queue>)>()> acceptor)
{
    session_acceptor_ = acceptor;
}

void tcp_server::listen(const tcp::endpoint& endpoint)
{
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    start_accept();
}

void tcp_server::start_accept()
{
    acceptor_.async_accept(
            socket_,
            boost::bind(
                    &tcp_server::handle_accept,
                    this,
                    boost::asio::placeholders::error));
}

void tcp_server::handle_accept(const boost::system::error_code& error)
{
    if(!error)
    {
        queued_session::accept_handler(
                    std::move(socket_),
                    session_acceptor_());
    }

    start_accept();
}

} // namespace cwu

} // namespace cis1
