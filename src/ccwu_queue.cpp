#include "ccwu_queue.h"

#include "queued_ccwu_session.h"

namespace cis1
{

namespace cwu
{

queue::queue(queued_session& self)
    : self_(self)
{
    static_assert(limit > 0, "queue limit must be positive");
}

queue::~queue()
{
    for(auto& [key, handler] : close_handlers_)
    {
        handler();
    }
}

void queue::send(
        boost::asio::const_buffer msg,
        std::function<void()> on_write,
        cis_message_type type)
{
    messages_.push_back({
            static_cast<uint8_t>(type),
            std::move(msg),
            std::move(on_write),
            false});

    if(messages_.size() == 1)
    {
        send();
    }
}

void queue::send_text(
        boost::asio::const_buffer msg,
        std::function<void()> on_write)
{
    send(msg, on_write, cis_message_type::regular);
}

bool queue::is_full()
{
    return messages_.size() >= limit;
}

boost::asio::executor queue::get_executor()
{
    return self_.strand_;
}

uint32_t queue::add_close_handler(
        std::function<void()> on_close)
{
    if(close_handlers_.empty())
    {
        close_handlers_.insert({0, on_close});

        return 0;
    }
    else
    {
        auto it = close_handlers_.end();
        --it;

        auto new_key = it->first - 1;
        close_handlers_.insert({new_key, on_close});

        return new_key;
    }
}

void queue::remove_close_handler(
        uint32_t id)
{
    close_handlers_.erase(id);
}

void queue::close()
{
    disconnect();
}

bool queue::on_write()
{
    BOOST_ASSERT(!messages_.empty());
    auto const was_full = is_full();
    if(messages_.front().on_write)
    {
        messages_.front().on_write();
    }
    messages_.erase(messages_.begin());

    if(!messages_.empty())
    {
        send();
    }

    return was_full;
}

void queue::disconnect()
{
    messages_.push_back({
            {},
            {},
            {},
            true});

    if(messages_.size() == 1)
    {
        send();
    }
}

void queue::send()
{
    if(messages_.front().disconnect)
    {
        self_.basic_session::disconnect();

        return;
    }

    current_message_size_ = htonl(
            (uint32_t)(
                    messages_.front().buffer.size()
                    + sizeof(message::message_type)));
    async_write(
            self_.socket_,
            boost::asio::const_buffer(
                    &current_message_size_,
                    sizeof(current_message_size_)),
            boost::asio::bind_executor(
                    self_.strand_,
                    std::bind(
                            &queue::write_type,
                            std::shared_ptr<queue>(
                                    self_.shared_from_this(),
                                    this),
                            std::placeholders::_1,
                            std::placeholders::_2)));
}

void queue::write_type(
        boost::system::error_code ec,
        size_t bytes_transferred)
{
    if(ec)
    {
        self_.on_write(ec, bytes_transferred);
    }
    else
    {
        async_write(
                self_.socket_,
                boost::asio::const_buffer(
                        &messages_.front().message_type,
                        sizeof(cis_message_type)),
                boost::asio::bind_executor(
                        self_.strand_,
                        std::bind(
                                &queue::write_body,
                                std::shared_ptr<queue>(
                                        self_.shared_from_this(),
                                        this),
                                std::placeholders::_1,
                                std::placeholders::_2)));
    }
}

void queue::write_body(
        boost::system::error_code ec,
        size_t bytes_transferred)
{
    if(ec)
    {
        self_.on_write(ec, bytes_transferred);
    }
    else
    {
        async_write(
                self_.socket_,
                messages_.front().buffer,
                boost::asio::bind_executor(
                        self_.strand_,
                        std::bind(
                                &queued_session::on_write,
                                self_.shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2)));
    }
}

} // namespace cwu

} // namespace cis1
