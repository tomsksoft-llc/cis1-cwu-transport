#pragma once

enum class cis_message_type : uint8_t
{
    ping = 0,
    pong = 1,
    regular = 2,
};
