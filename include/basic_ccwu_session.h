#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>

#include "ccwu_message_type.h"

namespace cis1
{

namespace cwu
{

class basic_session
    : public std::enable_shared_from_this<basic_session>
{
public:
    using message_type = cis_message_type;

    virtual ~basic_session();

    void run();

    virtual void disconnect();

    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base()
    {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }

protected:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;
    message_type message_type_;
    uint32_t message_size_;
    std::vector<char> message_;
    char ping_state_ = 0;
    message_type regular_message = message_type::regular;

    basic_session(boost::asio::ip::tcp::socket&& socket);

    virtual void async_write(
            boost::asio::const_buffer msg,
            std::function<void()>&& cb,
            basic_session::message_type type) = 0;

    virtual void handle_message() = 0;

    void do_read();

private:
    void on_timer(const boost::system::error_code& ec);

    void handle_size(
            const boost::system::error_code& ec,
            size_t bytes_transferred);

    void on_message_read(
            const boost::system::error_code& ec,
            size_t bytes_transferred);

    void on_ping_message();

    void on_pong_message();

    void activity();
};

} // namespace cwu

} // namespace cis1
