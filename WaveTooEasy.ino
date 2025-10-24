/***************************************************************************
 * Artekit Wavetooeasy
 * https://www.artekit.eu/products/devboards/wavetooeasy
 *
   Written by Ivan Meleca
 * Copyright (c) 2021 Artekit Labs
 * https://www.artekit.eu

### WaveTooEasy.ino

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

***************************************************************************/


#include <Arduino.h>

#include "Led.h"
#include "Player.h"

#undef min
#undef max

#include <array>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>


namespace
{
    constexpr auto wav_extension = std::string_view{ ".wav\0" };

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
            led1.blink(250, 125);
        }
        else if (!Audio.isPlaying() && playing)
        {
            playing = false;
            led1.stopBlink();
            led1.blink(1'000, 500);
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
    serial->begin(115'200, UARTClass::Mode_8N1);

    if (!disable_leds)
    {
        led2.setOn();
        led1.blink(1'000, 500);
    }

    Audio.begin(sample_rate);


    delay(500);
}


void loop()
{

    static int player_id = 0;

    pollAudioActivityLED();

    players.poll();


    const auto start = micros();
    constexpr auto serial_timeout_us = uint32_t{ 10000 };

    if (serial->available() > 0)
    {
        byte data_buffer[3U]{ 0, 0, 0 };
        size_t data_index = 0;
        for (auto start = micros(); static_cast<int32_t>(micros() - start - serial_timeout_us) < 0;)
        {
            const auto c = serial->read();

            if (data_index == 0 && c == 0x99)
            {
                data_buffer[data_index++] = static_cast<byte>(c);
                continue;
            }

            if (data_index != 0 && c > 0)
            {
                data_buffer[data_index++] = static_cast<byte>(c);
                if (data_index == 3)
                {
                    break;
                }
            }
        }


        const auto message = data_buffer[0];
        const auto note = data_buffer[1];
        const auto velocity = data_buffer[2];

        if (message == 0x99 && data_index == 3)
        {


            auto player = players.get(player_id);
            player_id = (player_id + 1) % MAX_PLAYERS;
            auto volume = static_cast<float>(velocity) / 127.F;
            volume = std::max(volume, 0.F);

            volume = std::min(volume, 1.0F);

            player->setVolume(volume);

            std::array<char, 4U> note_str_buffer{};
            std::to_chars(note_str_buffer.begin(), note_str_buffer.end(), int{ note });

            const auto note_str = std::string_view{ note_str_buffer.data() };

            std::string file_name;
            file_name.reserve(note_str.size() + wav_extension.size());
            file_name.append(note_str).append(wav_extension);

            player->play(file_name.c_str());
        }
    }
}
