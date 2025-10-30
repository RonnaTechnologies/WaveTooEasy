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

#include "variant.h"
#include "wiring.h"

#include "Led.h"
#include "Player.h"

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
    constexpr auto note_on_message = 0x99;
    constexpr auto max_velocity_float = 127.F;
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

    // Serial
    UARTClass* serial = nullptr;

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
    serial = &Serial;
    serial->setTimeout(0);
    serial->begin(baudrate, UARTClass::Mode_8N1);

    if (!disable_leds)
    {
        led2.setOn();
        led1.blink(t_1000ms, t_500ms);
    }

    Audio.begin(sample_rate);

    delay(t_500ms);
}


void loop()
{
    static auto player_id = 0;
    static auto stop_time = std::uint32_t{};
    static constexpr auto channel_note =
    std::array<std::uint8_t, PlayersPool::getMaxPlayers()>{ 26, 36, 38, 41, 44, 47, 49, 50, 51, 57 };

    pollAudioActivityLED();

    players.poll();

    if (serial->available() > 0)
    {
        byte data_buffer[3U]{ 0, 0, 0 };
        size_t data_index = 0;
        for (auto start = micros(); static_cast<int32_t>(micros() - start - serial_timeout_us) < 0;)
        {
            const auto input = serial->read();

            if (data_index == 0 && input == note_on_message)
            {
                data_buffer[data_index++] = static_cast<byte>(input);
                continue;
            }

            if (data_index != 0 && input > 0)
            {
                data_buffer[data_index++] = static_cast<byte>(input);
                if (data_index == 3)
                {
                    break;
                }
            }
        }

        const auto message = data_buffer[0];
        const auto note = data_buffer[1];
        const auto velocity = data_buffer[2];

        if (message == note_on_message && data_index == 3)
        {
            auto* player = players.get(player_id);
            player_id = (player_id + 1) % PlayersPool::getMaxPlayers();

            auto volume = static_cast<float>(velocity) / max_velocity_float;
            volume = std::max(volume, 0.F);
            volume = std::min(volume, 1.0F);

            player->setVolume(volume);

            std::array<char, 4U> note_str_buffer{};
            std::to_chars(note_str_buffer.begin(), note_str_buffer.end(), int{ note });

            const auto note_str = std::string_view{ note_str_buffer.data() };

            std::string file_name;
            file_name.reserve(note_str.size() + wav_extension.size());
            file_name.append(note_str).append(wav_extension);

            player->stop(true);
            player->play(file_name.c_str());
            player_id = (player_id + 1) % PlayersPool::getMaxPlayers();
        }
    }
}
