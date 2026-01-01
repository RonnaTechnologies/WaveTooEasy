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

#include "include/FileSystem.hpp"
#include "include/Led.h"
#include "include/SoundModule.hpp"
#include "variant.h"
#include "wiring.h"
#include "wiring_analog.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include <cstdlib>


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
    // BoardLed led3(A11);

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

    int prev_value = 1;

} // namespace


void setup()
{
    // Configure LEDs
    led1.initialize();
    led2.initialize();

    // led3.initialize();

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


    fs::FileSystem sd_dir{ "/" };
    const auto files = sd_dir.list_files();

    for (const auto& file : files)
    {
        module.print(file.c_str());
    }

    pinMode(A10, INPUT_ANALOG);
    pinMode(CHANNEL3, INPUT);
    pinMode(CHANNEL4, INPUT);
    prev_value = 1;
}


void loop()
{
    const auto start = micros();
    const auto value = static_cast<int>(analogRead(A10));
    // digitalRead(CHANNEL3);
    // digitalRead(CHANNEL4);
    if (value != 0 && abs(prev_value - value) > 32)
    {
        const auto volume = static_cast<float>(value) / 4096.F;
        module.set_volume(volume);
        module.print(static_cast<int>(100.F * volume));
        prev_value = value;
    }

    // const auto value = digitalRead(CHANNEL3);
    // module.print(value);
    // delay(10);
    pollAudioActivityLED();
    module.poll();
}
