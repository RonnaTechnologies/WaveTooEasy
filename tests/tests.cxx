
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../include/FileSystem.hpp"
#include "../include/PlayersManager.hpp"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace
{

    auto millis() -> std::uint32_t
    {
        using namespace std::chrono;
        static auto t_start = high_resolution_clock::now(); // - milliseconds{ 1 };
        const auto now = high_resolution_clock::now();
        return duration_cast<milliseconds>(now - t_start).count();
    }

    void delay(const std::uint32_t delay_ms)
    {
        using namespace std::chrono;
        using namespace std::this_thread;
        sleep_for(milliseconds{ delay_ms });
    }

} // namespace

SCENARIO("The millis() and delay() functions work as expected")
{
    static constexpr auto ref_delay_ms = std::uint32_t{ 5 };
    GIVEN("calling millis() returns a small value")
    {
        const auto t_start = millis();
        REQUIRE(t_start < ref_delay_ms);

        AND_GIVEN("a short delay")
        {
            delay(ref_delay_ms);

            AND_GIVEN("another call to millis()")
            {
                const auto t_stop = millis();

                THEN("the elapsed time is greater or equals the delay")
                {
                    const auto delta = t_stop - t_start;
                    REQUIRE(delta == ref_delay_ms);
                }
            }
        }
    }
}


SCENARIO("Player index selection works as expected")
{
    static constexpr auto max_players = 10;

    GIVEN("a MIDI note and a velocity")
    {
        const auto snare_note = std::uint8_t{ 36 };

        AND_GIVEN("a player manager")
        {
            auto player_manager = proc::PlayerManager<max_players>{ &millis };

            WHEN("a note is added to a free slot")
            {
                const auto index = player_manager.insert_note(snare_note, 127);

                THEN("the note is given a slot")
                {
                    REQUIRE(index == 0);
                }
            }
        }
    }
}

SCENARIO("Add many notes to players manager")
{
    static constexpr auto max_players = 10;
    const auto snare_note = std::uint8_t{ 36 };

    const auto kick_note = std::uint8_t{ 38 };
    GIVEN("a player manager")
    {
        auto player_manager = proc::PlayerManager<max_players>{ &millis };
        WHEN("two notes are added")
        {
            player_manager.set_duration(player_manager.insert_note(snare_note, 127), 1000);
            player_manager.set_duration(player_manager.insert_note(kick_note, 127), 1000);

            const auto& slots = player_manager.get_slots();
            const auto nb_busy_slots = std::ranges::count_if(slots, [](const auto& slot) { return slot.duration > 0; });

            THEN("there are two busy slots")
            {
                REQUIRE(nb_busy_slots == 2);
            }
        }
    }
}

SCENARIO("Can add a new slot, when a slot has been freed")
{
    static constexpr auto max_players = 10;

    GIVEN("a player manager")
    {
        auto player_manager = proc::PlayerManager<max_players>{ &millis };
        WHEN("all slots are busy")
        {
            for (std::size_t i = 0; i < max_players; ++i)
            {
                player_manager.insert_note(36 + i, i + 1);
                delay(10);
            }

            AND_WHEN("enough time is spent to free a slot")
            {
                delay(10);
                THEN("a new slot can be inserted")
                {
                    REQUIRE(player_manager.insert_note(40, 127) == 0);
                }
            }
        }
    }
}

SCENARIO("Cannot add a new slots, when all slots are busy")
{
    static constexpr auto max_players = 10;

    GIVEN("a player manager")
    {
        auto player_manager = proc::PlayerManager<max_players>{ &millis };
        WHEN("all slots are busy")
        {
            for (std::size_t i = 0; i < max_players; ++i)
            {
                player_manager.set_duration(player_manager.insert_note(36 + i, i + 1), 1000);
            }

            THEN("a new slot cannot be added")
            {
                REQUIRE(player_manager.insert_note(40, 127) == -1);
            }
        }
    }
}

SCENARIO("Can force-add a new slot when all slots are busy")
{
    static constexpr auto max_players = 10;

    GIVEN("a player manager")
    {
        auto player_manager = proc::PlayerManager<max_players>{ &millis };

        AND_GIVEN("all slots are  busy")
        {
            for (std::size_t i = 0; i < max_players; ++i)
            {
                player_manager.set_duration(player_manager.insert_note(36 + i, 50 - i), 200 + 10 * i);
                delay(10);
            }

            WHEN("a new slot is added forcefully")
            {

                const auto index = player_manager.insert_note(40, 100, true);

                THEN("the slot is attributed")
                {
                    REQUIRE(index >= 0);
                }
            }
        }
    }
}


SCENARIO("Add to correct slot when all slots are busy")
{
    static constexpr auto max_players = 10;
    static constexpr auto target_note = std::uint8_t{ 40 };
    GIVEN("a player manager")
    {
        auto player_manager = proc::PlayerManager<max_players>{ &millis };
        AND_GIVEN("all slots are  busy")
        {
            for (std::size_t i = 0; i < max_players; ++i)
            {
                player_manager.set_duration(player_manager.insert_note(36 + i, 50 - i), 2000 + 20 * i);
                delay(10);
            }

            delay(10);

            const auto& slots = player_manager.get_slots();
            const auto* const target_note_it =
            std::ranges::find_if(slots, [&](const auto& slot) { return slot.note == target_note; });

            REQUIRE(target_note_it != slots.cend());

            const auto target_note_index = std::distance(slots.cbegin(), target_note_it);

            const auto index = player_manager.insert_note(target_note, 127, true);
            REQUIRE(target_note_index == index);
        }
    }
}

// SCENARIO("Sound files are parsed successfully")
// {
//     GIVEN("sound files names")
//     {
//         const auto dir = fs::FileSystem{ "../data" };
//         const auto list = dir.list_files();

//         std::unordered_map<std::uint8_t, std::unordered_map<std::uint8_t, std::string>> notes{};

//         for (const auto& file_name : list)
//         {
//             const auto slash_pos = static_cast<std::ptrdiff_t>(file_name.find_last_of('/'));
//             const auto name = std::string_view{ file_name.begin() + slash_pos + 1, file_name.end() - 4 };

//             const auto dash_pos = static_cast<std::ptrdiff_t>(name.find_first_of('-'));

//             const auto note_str = std::string_view{ name.begin(), name.begin() + dash_pos };
//             const auto index_str = std::string_view{ name.begin() + dash_pos + 1, name.end() };

//             std::uint8_t note{};
//             std::from_chars(note_str.data(), note_str.data() + note_str.size(), note);

//             std::uint8_t index{};
//             std::from_chars(index_str.data(), index_str.data() + index_str.size(), index);

//             notes[note][index] = file_name;
//         }

//         [[maybe_unused]] auto a = 1;
//     }
// }
