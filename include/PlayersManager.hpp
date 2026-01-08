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

        auto find_free_slot(std::uint32_t now) -> std::size_t
        {

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
            const auto now = get_time();

            if (const auto index = find_free_slot(now); index != static_cast<std::size_t>(-1))
            {
                slots[index] = note_time{ note, velocity, now, 0 };
                return static_cast<int>(index);
            }

            if (!force)
            {
                return -1;
            }

            const auto get_priority = [now](const note_time& n_t) -> float
            {
                const auto start_impulse = std::uint32_t{ n_t.velocity } * std::uint32_t{ n_t.duration };
                const auto elapsed_impulse = (now - n_t.start_time) * std::uint32_t{ n_t.velocity };
                return static_cast<float>(start_impulse) - static_cast<float>(elapsed_impulse);
            };

            const auto is_same_note = [note](const note_time& n_t) { return n_t.note == note; };

            const auto same_note_it = std::find_if(slots.begin(), slots.end(), is_same_note);

            if (same_note_it != slots.end())
            {
                const auto lowest_same = std::min_element(slots.begin(), slots.end(),
                                                          [note, &get_priority](const note_time& lhs, const note_time& rhs)
                                                          {
                                                              const auto lhs_is_same = (lhs.note == note);
                                                              const auto rhs_is_same = (rhs.note == note);

                                                              if (lhs_is_same && rhs_is_same)
                                                              {
                                                                  return get_priority(lhs) < get_priority(rhs);
                                                              }
                                                              return lhs_is_same && !rhs_is_same;
                                                          });

                const auto steal_index = std::distance(slots.begin(), lowest_same);
                slots[steal_index] = note_time{ note, velocity, now, 0 };
                return static_cast<int>(steal_index);
            }

            const auto steal_it =
            std::min_element(slots.begin(), slots.end(), [&get_priority](const note_time& lhs, const note_time& rhs)
                             { return get_priority(lhs) < get_priority(rhs); });

            const auto steal_index = std::distance(slots.begin(), steal_it);
            slots[steal_index] = note_time{ note, velocity, now, 0 };
            return static_cast<int>(steal_index);
        }

        void set_duration(const std::size_t& index, const std::uint32_t duration) noexcept
        {
            slots[index].duration = duration;

            // std::sort(slots.begin(), slots.end(),
            //           [](const note_time& lhs, const note_time& rhs) { return lhs.velocity > rhs.velocity; });
        }

        [[nodiscard]] auto& get_slots() const noexcept
        {
            return slots;
        }

        struct note_time
        {
            std::uint8_t note{};
            std::uint8_t velocity{};
            std::uint32_t start_time{};
            std::uint32_t duration{};
        };

    private:
        std::array<note_time, N> slots{};
        time_getter_t get_time;
    };
} // namespace proc