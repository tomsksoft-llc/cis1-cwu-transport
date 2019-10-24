#include "ccwu_tcp_client.h"

#include <cis1_proto_utils/cloexec.h>

#include "queued_ccwu_session.h"

namespace cis1
{

namespace cwu
{

using tcp = boost::asio::ip::tcp;

tcp_client::tcp_client(boost::asio::io_context& io_context)
    : socket_(io_context)
{}

void tcp_client::set_message_handler(
            std::function<void(
                    boost::asio::const_buffer,
                    std::shared_ptr<queue>)> msg_handler)
{
    msg_handler_ = msg_handler;
}

void tcp_client::connect(
        const tcp::endpoint& endpoint,
        boost::system::error_code& ec)
{
    socket_.connect(endpoint, ec);
    if(ec)
    {
        return;
    }

    cis1::proto_utils::set_cloexec(socket_, ec);

    session_ = queued_session::accept_handler(
            std::move(socket_),
            [&](auto&&... args)
            {
                if(msg_handler_)
                {
                    msg_handler_(std::forward<decltype(args)>(args)...);
                }
            });
}

void tcp_client::disconnect()
{
    boost::asio::post(
            session_->get_queue()->get_executor(),
            [&](){session_->disconnect();});
}

std::shared_ptr<queue> tcp_client::get_queue()
{
    if(session_)
    {
        return session_->get_queue();
    }

    return nullptr;
}

} // namespace cwu

} // namespace cis1
