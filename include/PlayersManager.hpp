#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace proc
{
    template <std::size_t N>
    class PlayerManager
    {
    public:
        using time_getter_t = std::uint32_t (*)();

        explicit PlayerManager(time_getter_t time_getter) : get_time{ time_getter }
        {
        }

        auto compute_index_for(const std::uint8_t note) -> std::size_t
        {
            const auto now = get_time();


            // look for a free slot
            const auto slot_it =
            std::find_if(slots.cbegin(), slots.cend(), [&](const note_time& n_t)
                         { return static_cast<std::int32_t>(n_t.start_time + n_t.length - now) < 0; });

            if (slot_it != slots.cend())
            {
                return std::distance(slots.cbegin(), slot_it);
            }

            return static_cast<std::size_t>(-1);
        }

        auto insert_note(const std::uint8_t note, const std::uint8_t velocity, const std::uint32_t duration) -> bool
        {
            const auto index = compute_index_for(note);
            if (index == static_cast<std::size_t>(-1))
            {
                return false;
            }

            slots[index] = note_time{ .note = note, .velocity = velocity, .start_time = get_time(), .length = duration };

            std::sort(slots.begin(), slots.end(), [](const note_time& lhs, const note_time& rhs)
                      { return (lhs.start_time + lhs.length) < (rhs.start_time + rhs.length); });

            return true;
        }

        [[nodiscard]] auto& get_slots() const noexcept
        {
            return slots;
        }

    private:
        struct note_time
        {
            std::uint8_t note{};
            std::uint8_t velocity{};
            std::uint32_t start_time{};
            std::uint32_t length{};
        };
        std::array<note_time, N> slots{};
        time_getter_t get_time;
    };
} // namespace proc