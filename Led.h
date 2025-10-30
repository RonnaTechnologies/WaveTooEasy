/***************************************************************************
 * Artekit Wavetooeasy
 * https://www.artekit.eu/products/devboards/wavetooeasy
 *
   Written by Ivan Meleca
 * Copyright (c) 2021 Artekit Labs
 * https://www.artekit.eu

### Led.h

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

#ifndef __LED_H__
#define __LED_H__


#include <ServiceTimer.h>

#include "wiring_constants.h"
#include "wiring_digital.h"

#include <cstdint>

class BoardLed : public STObject
{
public:
    BoardLed(const BoardLed&) = default;
    BoardLed(BoardLed&&) = delete;
    BoardLed& operator=(const BoardLed&) = default;
    BoardLed& operator=(BoardLed&&) = delete;

    virtual ~BoardLed() = default;

    explicit BoardLed(uint32_t led) : led(led)
    {
    }

    void initialize()
    {
        pinMode(led, OUTPUT);
        set(false);
    }
    void end() const
    {
        pinMode(led, INPUT);
    }

    void toggle()
    {
        isOn() ? setOff() : setOn();
    }

    void set(bool on)
    {
        value = on;
        digitalWrite(led, value ? LOW : HIGH);
    }
    void setOn()
    {
        set(true);
    }
    void setOff()
    {
        set(false);
    }
    [[nodiscard]] bool isOn() const
    {
        return value;
    }

    void blink(uint32_t time_ms, uint32_t time_on_ms)
    {
        blink_cycle_ticks = (1'000 / getFrequency()) * time_ms;
        blink_time_on = time_on_ms;
        blink_counter = 0;
        set(true);
        add();
    }

    void stopBlink()
    {
        remove();
        setOff();
    }

    void poll() override
    {
        blink_counter++;
        if (isOn() && blink_counter > blink_time_on)
        {
            setOff();
        }

        if (!isOn() && blink_counter > blink_cycle_ticks)
        {
            blink_counter = 0;
            setOn();
        }
    }

private:
    std::uint32_t led;
    bool value{ false };
    std::uint32_t blink_counter{ 0 };
    std::uint32_t blink_cycle_ticks{ 0 };
    std::uint32_t blink_time_on{ 0 };
};

#endif /* __LED_H__ */
