#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <deque>

#include <cis1_proto_utils/queue_interface.h>

#include "ccwu_message_type.h"

namespace cis1
{

namespace cwu
{

class queued_session;

class queue
    : public cis1::proto_utils::queue_interface
{
public:
    friend class queued_session;
    queue(queued_session& self);
    ~queue() = default;

    void send_text(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write) override;
    bool is_full() override;
    boost::asio::executor get_executor() override;
    void disconnect();

private:
    enum
    {
        limit = 64
    };

    struct message
    {
        uint8_t message_type;
        boost::asio::const_buffer buffer;
        std::function<void()> on_write;
        bool disconnect = false;
    };

    queued_session& self_;
    std::deque<message> messages_;
    uint32_t current_message_size_;

    bool on_write();
    void send();
    void write_type(
            boost::system::error_code ec,
            size_t bytes_transferred);

    void write_body(
            boost::system::error_code ec,
            size_t bytes_transferred);
    void send(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write,
            cis_message_type type);
};

} // namespace cwu

} // namespace cis1
