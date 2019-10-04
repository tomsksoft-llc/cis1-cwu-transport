#pragma once

#include "basic_ccwu_session.h"
#include "ccwu_queue.h"

namespace cis1
{

namespace cwu
{

class queued_session
    : public basic_session
{
public:
    using request_handler_t = std::function<void(
            boost::asio::const_buffer,
            std::shared_ptr<queue>)>;

    static std::shared_ptr<queued_session> accept_handler(
            boost::asio::ip::tcp::socket&& socket,
            request_handler_t handler);

    explicit queued_session(
            boost::asio::ip::tcp::socket&& socket,
            request_handler_t handler);

    std::shared_ptr<queued_session> shared_from_this();

    std::shared_ptr<queue> get_queue();

    void on_write(
            boost::system::error_code ec,
            size_t bytes_transferred);

    void disconnect() override;

protected:
    virtual void async_write(
            boost::asio::const_buffer msg,
            std::function<void()>&& cb,
            basic_session::message_type type) override;

    virtual void handle_message() override;

private:
    request_handler_t handler_;
    queue queue_;
    friend class queue;
};

} // namespace cwu

} // namespace cis1
