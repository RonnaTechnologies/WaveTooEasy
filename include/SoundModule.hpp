#pragma once


#include "WString.h"
#include "wiring.h"
#include <cstdint>

#undef min
#undef max

#include "HiHatManager.hpp"
#include "Player.h"
#include "PlayersManager.hpp"
#include "SerialMIDI.hpp"


#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
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

            if (serial_midi.is_control_change())
            {
                if (const auto cc_value = static_cast<std::uint32_t>(serial_midi.get_cc_value()); cc_value > 0)
                {
                    hi_hat_manager.update_pedal_position(cc_value);
                    // std::array<char, 8U> cc_str{};
                    // const auto result = std::to_chars(cc_str.begin(), cc_str.end(), cc_value);

                    // if (result.ec != std::errc{})
                    // {
                    //     return;
                    // }
                    // const auto ccstr = std::string{ cc_str.data(), result.ptr };
                    // serial_midi.println(String{ ccstr.c_str() });
                }
                return;
            }

            if (serial_midi.is_note_on())
            {

                const auto now = millis();
                const auto note = serial_midi.get_note() == 26 ? hi_hat_manager.get_hihat_note() : serial_midi.get_note();
                const auto velocity = serial_midi.get_velocity();

                const auto velocity_layer =
                static_cast<std::uint32_t>(std::ceil(static_cast<float>(velocity) / velocity_layer_range));

                // {
                //     std::array<char, 8U> cc_str{};
                //     const auto result = std::to_chars(cc_str.begin(), cc_str.end(), velocity_layer);

                //     if (result.ec != std::errc{})
                //     {
                //         return;
                //     }
                //     const auto ccstr = std::string{ cc_str.data(), result.ptr };
                //     serial_midi.println(String{ ccstr.c_str() });
                // }

                const auto player_id = player_manager.insert_note(note, velocity, true);
                Player* player = players.get(player_id);


                const auto volume = std::clamp(static_cast<float>(velocity) / max_velocity_float, 0.0F, 1.0F);

                player->setVolume(volume * master_volume);

                std::array<char, 4U> note_str_buffer{};
                std::to_chars(note_str_buffer.begin(), note_str_buffer.end(), int{ note });

                const auto note_str = std::string_view{ note_str_buffer.data() };

                std::array<char, 4U> vel_layer_str_buf{};
                std::to_chars(vel_layer_str_buf.begin(), vel_layer_str_buf.end(), velocity_layer);

                const auto vel_layer_str = std::string_view{ vel_layer_str_buf.data() };

                std::string file_name;
                file_name.reserve(kit_folder.size() + 1 + note_str.size() + 1 + vel_layer_str.size() + wav_extension.size());
                file_name.append("/").append(kit_folder).append("/").append(note_str).append("-").append(vel_layer_str).append(wav_extension);

                // serial_midi.println(String{ file_name.c_str() });

                player->stop();
                player->play(file_name.c_str(), PlayModeNormal, note);

                player_manager.set_duration(player_id, player->get_duration());

                return;
            }
        }

        void set_volume(float volume) noexcept
        {
            master_volume = volume;
        }

        void set_kit(const std::string& name)
        {
            kit_folder = name;
        }

        void set_nb_velocity_layers(std::uint32_t nb_velocity_layers)
        {
            velocity_layer_range = 127.F / static_cast<float>(nb_velocity_layers);
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
        proc::HiHatManager hi_hat_manager;
        float master_volume = 1.F;
        float velocity_layer_range = 127.F / 1.F;
        std::string kit_folder;
    };

} // namespace sound