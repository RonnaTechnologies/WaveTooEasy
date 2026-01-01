#pragma once


#include "variant.h"
#include "wiring.h"

#undef min
#undef max

#include "Player.h"
#include "PlayersManager.hpp"
#include "SerialMIDI.hpp"


#include <algorithm>
#include <array>
#include <charconv>
#include <string>
#include <string_view>


namespace serial
{
    constexpr auto baudrate = 115'200;
    constexpr auto timeout_us = uint32_t{ 10'000 };

} // namespace serial


namespace sound
{
    class Module
    {
    public:
        void init()
        {
            players.initialize(false);
            serial_midi.begin(serial::baudrate);
        }

        void poll()
        {
            players.poll();
            serial_midi.poll(serial::timeout_us);

            if (!serial_midi.is_note_on())
            {
                return;
            }

            const auto now = millis();
            const auto note = serial_midi.get_note();
            const auto velocity = serial_midi.get_velocity();

            const auto player_id = player_manager.insert_note(note, velocity, true);
            Player* player = players.get(player_id);


            const auto volume = std::clamp(static_cast<float>(velocity) / max_velocity_float, 0.0F, 1.0F);

            player->setVolume(volume * master_volume);

            std::array<char, 4U> note_str_buffer{};
            std::to_chars(note_str_buffer.begin(), note_str_buffer.end(), int{ note });

            const auto note_str = std::string_view{ note_str_buffer.data() };

            std::string file_name;
            file_name.reserve(note_str.size() + wav_extension.size());
            file_name.append(note_str).append(wav_extension);

            player->stop();
            player->play(file_name.c_str(), PlayModeNormal, note);

            player_manager.set_duration(player_id, player->get_duration());

            serial_midi.println(String{ player_id });
        }

        void set_volume(float volume) noexcept
        {
            master_volume = volume;
        }

        void serial_poll()
        {
            serial_midi.poll(serial::timeout_us);
        }

        template <typename T>
        void print(const T& value)
        {
            serial_midi.println(String{ value });
        }

    private:
        static constexpr std::string_view wav_extension = std::string_view{ ".wav\0" };
        static constexpr auto max_velocity_float = 127.F;

        PlayersPool players = PlayersPool::getInstance();
        midi::serial serial_midi;
        proc::PlayerManager<PlayersPool::getMaxPlayers()> player_manager{ &millis };
        float master_volume = 1.F;
    };

} // namespace sound