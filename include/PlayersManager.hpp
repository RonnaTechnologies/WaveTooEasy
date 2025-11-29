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

        auto find_free_slot() -> std::size_t
        {
            const auto now = get_time();

            // look for a free slot
            const auto slot_it =
            std::find_if(slots.cbegin(), slots.cend(), [&](const note_time& n_t)
                         { return static_cast<std::int32_t>(n_t.start_time + n_t.duration - now) <= 0; });

            if (slot_it != slots.cend())
            {
                return std::distance(slots.cbegin(), slot_it);
            }

            return static_cast<std::size_t>(-1);
        }

        auto insert_note(const std::uint8_t note, const std::uint8_t velocity, const bool force = false) -> int
        {
            const auto add_note = [&](const auto& index) -> std::size_t
            {
                slots[index] = note_time{ .note = note, .velocity = velocity, .start_time = get_time() };
                return index;
            };

            const auto index = find_free_slot();
            const auto success = index != static_cast<std::size_t>(-1);
            if (!success && !force)
            {
                return -1;
            }

            if (success)
            {
                return add_note(index);
            }

            if (!force)
            {
                return -1;
            }

            // force

            const auto same_note_it =
            std::find_if(slots.cbegin(), slots.cend(), [&](const note_time& n_t) { return n_t.note == note; });

            if (same_note_it != slots.cend())
            {
                const auto slot_index = std::distance(slots.cbegin(), same_note_it);
                return add_note(slot_index);
            }

            return add_note(0);
        }

        void set_duration(const std::size_t& index, const std::uint32_t duration) noexcept
        {
            slots[index].duration = duration;

            std::sort(slots.begin(), slots.end(),
                      [](const note_time& lhs, const note_time& rhs)
                      {
                          return (lhs.start_time + lhs.duration) * lhs.velocity < (rhs.start_time + rhs.duration) * rhs.velocity;
                      });
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
            std::uint32_t duration{};
        };
        std::array<note_time, N> slots{};
        time_getter_t get_time;
    };
} // namespace proc