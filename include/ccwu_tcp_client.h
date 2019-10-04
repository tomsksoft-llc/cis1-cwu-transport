#pragma once

#include <vector>
#include <memory>

#include <boost/asio.hpp>

#include "ccwu_queue.h"

namespace cis1
{

namespace cwu
{

class tcp_client
{
public:
    tcp_client(boost::asio::io_context& io_context);

    void connect(
            const boost::asio::ip::tcp::endpoint& endpoint,
            boost::system::error_code& ec);

    void disconnect();

    void set_message_handler(
            std::function<void(
                    boost::asio::const_buffer,
                    std::shared_ptr<queue>)> msg_handler);

    std::shared_ptr<queue> get_queue();

private:
    boost::asio::ip::tcp::socket socket_;
    std::function<void(
            boost::asio::const_buffer,
            std::shared_ptr<queue>)> msg_handler_;
    std::shared_ptr<queued_session> session_ = nullptr;
};

} // namespace cwu

} // namespace cis1
