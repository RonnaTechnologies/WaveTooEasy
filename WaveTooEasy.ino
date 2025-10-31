/***************************************************************************
 * Artekit Wavetooeasy
 * https://www.artekit.eu/products/devboards/wavetooeasy
 *
 * Written by Ivan Meleca
 * Copyright (c) 2021 Artekit Labs
 * https://www.artekit.eu
 *
 * Modified by Jeremy Oden
 * Copyright (c) 2025 Ronna Technologies
 * https://ronnatech.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 ***************************************************************************/


#include "WString.h"
#include "variant.h"
#include "wiring.h"

#include "Led.h"
#include "Player.h"
#include "SerialMIDI.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>


#undef min
#undef max


namespace
{
    // Constants
    constexpr auto wav_extension = std::string_view{ ".wav\0" };
    constexpr auto baudrate = 115'200;
    constexpr auto serial_timeout_us = uint32_t{ 10'000 };
    constexpr auto max_velocity_float = 127.F;
    constexpr auto nb_sensors = 10U;
    constexpr auto t_125ms = 125;
    constexpr auto t_250ms = 250;
    constexpr auto t_500ms = 500;
    constexpr auto t_1000ms = 1'000;

    // Configuration
    const std::uint32_t sample_rate = 44'100;
    const bool disable_leds = false;

    // LEDs
    BoardLed led1(LED1);
    BoardLed led2(LED2);

    // Players
    PlayersPool players = PlayersPool::getInstance();

    midi::serial serial_midi;

    void pollAudioActivityLED()
    {
        static bool playing = false;

        if (disable_leds)
        {
            return;
        }

        if (Audio.isPlaying() && !playing)
        {
            playing = true;
            led1.stopBlink();
            led1.blink(t_250ms, t_125ms);
        }
        else if (!Audio.isPlaying() && playing)
        {
            playing = false;
            led1.stopBlink();
            led1.blink(t_1000ms, t_500ms);
        }
    }
} // namespace


void setup()
{
    // Configure LEDs
    led1.initialize();
    led2.initialize();

    players.initialize(false);

    if (!disable_leds)
    {
        led2.setOn();
        led1.blink(t_1000ms, t_500ms);
    }

    Audio.begin(sample_rate);
    serial_midi.begin(baudrate);

    delay(t_1000ms);
    delay(t_1000ms);
    for (std::size_t i = 0; i < nb_sensors; ++i)
    {
        serial_midi.poll(serial_timeout_us);
    }
}


void loop()
{
    static constexpr auto player_note = std::array<std::uint8_t, PlayersPool::getMaxPlayers()>{};

    pollAudioActivityLED();
    players.poll();
    serial_midi.poll(serial_timeout_us);

    if (serial_midi.is_note_on())
    {
        const auto now = millis();
        const auto note = serial_midi.get_note();
        const auto velocity = serial_midi.get_velocity();

        Player* player = nullptr;
        Player* backup_player = nullptr;
        for (std::size_t i = 0; i < PlayersPool::getMaxPlayers(); ++i)
        {
            auto* selected_player = players.get(i);
            if (static_cast<std::int32_t>(selected_player->get_stop_time() - now) <= 0)
            {
                player = players.get(i);
            }
            if (backup_player->get_stop_time() > selected_player->get_stop_time())
            {
                backup_player = selected_player;
            }
        }

        if (player == nullptr)
        {
            // If no player has been found, find the oldest with same sound
            for (std::size_t i = 0; i < PlayersPool::getMaxPlayers(); ++i)
            {
                auto* selected_player = players.get(i);
                if (selected_player->get_sound_id() == note)
                {
                    if (player == nullptr)
                    {
                        player = selected_player;
                    }
                    else
                    {
                        if (player->get_stop_time() > selected_player->get_stop_time())
                        {
                            backup_player = selected_player;
                        }
                    }
                }
            }

            if (player == nullptr)
            {
                player = backup_player;
            }
        }


        const auto volume = std::clamp(static_cast<float>(velocity) / max_velocity_float, 0.0F, 1.0F);

        player->setVolume(volume);

        std::array<char, 4U> note_str_buffer{};
        std::to_chars(note_str_buffer.begin(), note_str_buffer.end(), int{ note });

        const auto note_str = std::string_view{ note_str_buffer.data() };

        std::string file_name;
        file_name.reserve(note_str.size() + wav_extension.size());
        file_name.append(note_str).append(wav_extension);

        player->stop();
        player->play(file_name.c_str(), PlayModeNormal, note);
        serial_midi.println(String{ static_cast<int>(player->get_sound_id()) });
    }
}
