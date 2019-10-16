#pragma once

#include <vector>
#include <memory>

#include <boost/asio.hpp>

#include "ccwu_queue.h"

namespace cis1
{

namespace cwu
{

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context);

    void listen(const boost::asio::ip::tcp::endpoint& endpoint);

    void set_session_acceptor(
            std::function<std::function<void(
                    boost::asio::const_buffer,
                    std::shared_ptr<queue>)>()> acceptor);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    std::function<std::function<void(
                    boost::asio::const_buffer,
                    std::shared_ptr<queue>)>()> session_acceptor_;

    void start_accept();

    void handle_accept(const boost::system::error_code& error);
};

} // namespace cwu

} // namespace cis1
