#pragma once

#include <variant.h>

#include <array>
#include <cstdint>

namespace midi
{
    class serial
    {
    public:
        void begin(std::uint32_t baudrate)
        {
            uart->setTimeout(0);
            uart->begin(baudrate, UARTClass::Mode_8N1);
        }

        void poll(std::uint32_t timeout_us)
        {
            data_buffer = { 0, 0, 0 };
            for (std::uint32_t i = 0, start = micros(); static_cast<int32_t>(micros() - start - timeout_us) < 0;)
            {
                const auto input = uart->read();
                if (input > 0)
                {
                    data_buffer[i++] = static_cast<std::uint8_t>(input);
                    if (i == 3)
                    {
                        break;
                    }
                }
            }
        }

        [[nodiscard]] auto is_note_on() const noexcept -> bool
        {
            return note_on == (data_buffer.front() & upper_nibble_mask);
        }

        [[nodiscard]] auto is_control_change() const noexcept -> bool
        {
            return control_change == (data_buffer.front() & upper_nibble_mask);
        }

        [[nodiscard]] auto get_note() const noexcept -> std::uint8_t
        {
            return data_buffer[1];
        }

        [[nodiscard]] auto get_cc_number() const noexcept -> std::uint8_t
        {
            return data_buffer[1];
        }

        [[nodiscard]] auto get_velocity() const noexcept -> std::uint8_t
        {
            return data_buffer.back();
        }

        [[nodiscard]] auto get_cc_value() const noexcept -> std::uint8_t
        {
            return data_buffer.back();
        }

        void println(const String& str)
        {
            uart->println(str);
        }

    private:
        static constexpr auto upper_nibble_mask = static_cast<std::uint8_t>(0xF0);
        static constexpr auto note_on = static_cast<std::uint8_t>(0x90);
        static constexpr auto control_change = static_cast<std::uint8_t>(0xB0);

        UARTClass* uart = &Serial;
        std::array<std::uint8_t, 3U> data_buffer = { 0, 0, 0 };
    };
} // namespace midi