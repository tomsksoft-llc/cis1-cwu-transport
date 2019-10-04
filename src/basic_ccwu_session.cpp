#include "basic_ccwu_session.h"

namespace cis1
{

namespace cwu
{

const uint32_t max_message_size = 1024;
const auto keep_alive_duration = std::chrono::seconds(5);

basic_session::~basic_session() = default;

void basic_session::run()
{
    on_timer({});

    activity();

    do_read();
}

void basic_session::disconnect()
{
    boost::system::error_code ec;
    timer_.expires_at((std::chrono::steady_clock::time_point::min)());
    timer_.cancel();
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close();
}

basic_session::basic_session(
        boost::asio::ip::tcp::socket&& socket)
    : socket_(std::move(socket))
    , strand_(socket_.get_executor())
    , timer_(
            socket_.get_executor().context(),
            (std::chrono::steady_clock::time_point::max)())
{}

void basic_session::do_read()
{
    boost::asio::async_read(
            socket_,
            boost::asio::buffer(&message_size_, sizeof(message_size_)),
            boost::asio::bind_executor(
                    strand_,
                    boost::bind(
                            &basic_session::handle_size,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
}

void basic_session::on_timer(
        const boost::system::error_code& ec)
{
    if(ec && ec != boost::asio::error::operation_aborted)
    {
        return;
    }

    if(timer_.expiry() <= std::chrono::steady_clock::now())
    {
        if(socket_.is_open() && ping_state_ == 0)
        {
            ping_state_ = 1;
            timer_.expires_after(keep_alive_duration);

            this->async_write({}, {}, message_type::ping);
        }
        else
        {
            disconnect();

            return;
        }
    }

    timer_.async_wait(
            boost::asio::bind_executor(
                    strand_,
                    std::bind(
                            &basic_session::on_timer,
                            shared_from_this(),
                            std::placeholders::_1)));
}

void basic_session::handle_size(
        const boost::system::error_code& ec,
        size_t /*bytes_transferred*/)
{
    if(!ec)
    {
        message_size_ = ntohl(message_size_);

        if(message_size_ > 0 && message_size_ <= max_message_size)
        {
            message_.resize(message_size_);
            boost::asio::async_read(
                    socket_,
                    boost::asio::buffer(message_, message_size_),
                    boost::asio::bind_executor(
                            strand_,
                            boost::bind(
                                    &basic_session::on_message_read,
                                    shared_from_this(),
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred)));
        }
        else
        {
            disconnect();
        }
    }
    else
    {
        disconnect();
    }
}

void basic_session::on_message_read(
        const boost::system::error_code& ec,
        size_t /*bytes_transferred*/)
{
    if(!ec)
    {
        activity();

        std::memcpy(&message_type_, message_.data(), sizeof(message_type_));

        switch(message_type_)
        {
            case message_type::ping:
            {
                on_ping_message();
                break;
            }
            case message_type::pong:
            {
                on_pong_message();
                break;
            }
            case message_type::regular:
            {
                this->handle_message();

                do_read();
                break;
            }
            default:
            {
                disconnect();
            }
        }
    }
}

void basic_session::on_ping_message()
{
    activity();

    this->async_write({}, {}, message_type::pong);

    do_read();
}

void basic_session::on_pong_message()
{
    if(ping_state_ == 1)
    {
        ping_state_ = 0;
    }
    else
    {
        disconnect();

        return;
    }

    activity();

    do_read();
}

void basic_session::activity()
{
    timer_.expires_after(keep_alive_duration);
}

} // namespace cwu

} // namespace cis1
