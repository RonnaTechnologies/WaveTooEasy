#pragma once
#include "wiring.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include <cstdint>

namespace IO
{
    class Button
    {
    public:
        enum class State : std::uint8_t
        {
            pressed,
            released,
            unknown
        };

        explicit Button(std::uint8_t pin_nb) : pin_nb{ pin_nb }
        {
            pinMode(pin_nb, INPUT);
        }

        void poll(std::uint32_t now)
        {
            // While inside the debounce window that follows the last confirmed
            // transition, ignore the pin entirely: any wobble here is treated as
            // contact bounce, not a real change.
            if (state != State::unknown && static_cast<std::int32_t>(now - state_change_time - mask_time) < 0)
            {
                return;
            }

            const auto current_state = digitalRead(pin_nb) == HIGH ? State::pressed : State::released;
            if (state == State::unknown || current_state != state)
            {
                state_change_time = now;
                state = current_state;

                if (state == State::pressed)
                {
                    press_reported = false;
                }
            }
        }

        void poll()
        {
            poll(micros());
        }

        [[nodiscard]] auto debounced() const noexcept -> bool
        {
            return static_cast<std::int32_t>(micros() - state_change_time - mask_time) >= 0;
        }

        [[nodiscard]] auto get_state() const noexcept -> State
        {
            return state;
        }

        [[nodiscard]] auto pressed() noexcept -> bool
        {
            if (state == State::pressed && !press_reported)
            {
                press_reported = true;
                return true;
            }

            return false;
        }

    private:
        std::uint32_t state_change_time{};
        std::uint32_t mask_time = 20'000;
        std::uint8_t pin_nb;
        State state = State::unknown;
        bool press_reported = false;
    };
} // namespace IO