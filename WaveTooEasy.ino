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

#include "ff.h"

#include "include/Led.h"
#include "include/SoundModule.hpp"


#undef min
#undef max

namespace
{
    // Constants
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

    sound::Module module;


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

    module.init();

    if (!disable_leds)
    {
        led2.setOn();
        led1.blink(t_1000ms, t_500ms);
    }

    Audio.begin(sample_rate);

    delay(t_1000ms);
    delay(t_1000ms);

    for (std::size_t i = 0; i < nb_sensors; ++i)
    {
        module.serial_poll();
    }

    DIR dir{};
    const auto res = f_opendir(&dir, "/");
    if (res != FR_OK)
    {
        module.print("Failed to open root folder.");
        return;
    }


    module.print("Root folder open.");

    FILINFO file_info;
    for (;;)
    {
        const auto result = f_readdir(&dir, &file_info);
        if (result != FR_OK || file_info.fname[0] == 0)
        {
            break;
        }

        const auto file_name = String{ &file_info.fname[0] };
        if (file_name.endsWith(".wav") != 0)
        {

            module.print(file_info.fname);
        }

        // if ((file_info.fattrib & AM_DIR) != 0)
        // {
        // }
    }

    f_closedir(&dir);
}


void loop()
{
    pollAudioActivityLED();
    module.poll();
}
