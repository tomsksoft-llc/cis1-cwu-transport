#include "queued_ccwu_session.h"

namespace cis1
{

namespace cwu
{

std::shared_ptr<queued_session> queued_session::accept_handler(
        boost::asio::ip::tcp::socket&& socket,
        request_handler_t handler)
{
    auto session = std::make_shared<queued_session>(
            std::move(socket),
            handler);
    session->run();

    return session;
}

queued_session::queued_session(
        boost::asio::ip::tcp::socket&& socket,
        request_handler_t handler)
    : basic_session(std::move(socket))
    , handler_(handler)
    , queue_(*this)
{}

void queued_session::async_write(
        boost::asio::const_buffer msg,
        std::function<void()>&& cb,
        basic_session::message_type type)
{
    queue_.send(
            msg,
            std::move(cb),
            type);
}

void queued_session::disconnect()
{
    queue_.disconnect();
}

std::shared_ptr<queued_session>
queued_session::shared_from_this()
{
    return shared_from_base<queued_session>();
}

std::shared_ptr<queue> queued_session::get_queue()
{
    return std::shared_ptr<queue>(shared_from_this(), &queue_);;
}

void queued_session::on_write(
    boost::system::error_code ec,
    size_t bytes_transferred)
{
    if(ec)
    {
        return;
    }

    if(queue_.on_write())
    {
        do_read();
    }
}

void queued_session::handle_message()
{
    handler_(
            boost::asio::buffer(
                    message_.data() + sizeof(message_type),
                    message_.size() - sizeof(message_type)),
            get_queue());

    message_.clear();
}

} // namespace cwu

} // namespace cis1
