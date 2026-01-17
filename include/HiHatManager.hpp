#pragma once

#include <cstdint>

namespace proc
{
    class HiHatManager
    {
    public:
        void update_pedal_position(std::uint8_t cc_value) noexcept
        {
            previous_position = pedal_position;
            pedal_position = cc_value;
        }

        [[nodiscard]] auto should_choke_hihat() const noexcept -> bool
        {
            return previous_position > 50 && pedal_position < 30;
        }

        [[nodiscard]] auto get_pedal_position() const noexcept
        {
            return pedal_position;
        }

        [[nodiscard]] auto get_hihat_note() const noexcept -> std::uint8_t
        {
            if (pedal_position < 30)
            {
                return open_hihat_note;
            }

            if (pedal_position > 90)
            {
                return closed_hihat_note;
            }

            return half_open_hihat_note;
        }

    private:
        static constexpr auto closed_hihat_note = std::uint8_t{ 42 };
        static constexpr auto half_open_hihat_note = std::uint8_t{ 48 };
        static constexpr auto open_hihat_note = std::uint8_t{ 46 };
        static constexpr auto pedal_hihat_note = std::uint8_t{ 44 };

        static constexpr auto hihat_cc = std::uint8_t{ 4 };

        std::uint8_t pedal_position = 0;
        std::uint8_t previous_position = 0;
    };
} // namespace proc