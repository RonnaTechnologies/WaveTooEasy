#ifndef __PLAYER_H__
#define __PLAYER_H__


#include "WavPlayer.h"

#include <array>
#include <cstdint>


static constexpr auto max_players = 10;

enum class playerStatus : std::uint8_t
{
    playerStopped,
    playerPlaying,
    playerPaused,
    playerPausing,
    playerStopping,
};

class PlayersPool;

class Player
{
    friend class PlayersPool;

public:
    bool play(const char* filename, PlayMode mode = PlayModeNormal)
    {
        if (status == playerStatus::playerPausing || status == playerStatus::playerStopping)
        {
            wav.stop();
            status = playerStatus::playerStopped;
        }

        if (status == playerStatus::playerStopped || status == playerStatus::playerPaused)
        {
            wav.setVolume(base_volume);
        }

        if (wav.play(filename, mode))
        {
            status = playerStatus::playerPlaying;
            return true;
        }

        return false;
    }

    auto get_duration() noexcept
    {
        return wav.duration();
    }

    auto get_samples_left() noexcept
    {
        return wav.getSamplesLeft();
    }

    void stop(bool ramp_volume = false)
    {
        if (status == playerStatus::playerStopped)
        {
            return;
        }

        if (ramp_volume && status != playerStatus::playerPaused)
        {
            if (status == playerStatus::playerPlaying)
            {
                wav.setVolume(0);
            }

            status = playerStatus::playerStopping;
        }
        else
        {
            wav.stop();
            status = playerStatus::playerStopped;
        }
    }

    playerStatus getStatus()
    {
        switch (status)
        {
        case playerStatus::playerStopping:
        case playerStatus::playerStopped:
        default: return playerStatus::playerStopped;

        case playerStatus::playerPlaying: return playerStatus::playerPlaying;

        case playerStatus::playerPausing:
        case playerStatus::playerPaused: return playerStatus::playerPaused;
        }
    }

    void pause(bool ramp_volume = false)
    {
        if (status == playerStatus::playerPaused || status == playerStatus::playerStopped || status == playerStatus::playerStopping)
        {
            return;
        }

        if (ramp_volume)
        {
            if (status == playerStatus::playerPlaying)
            {
                wav.setVolume(0);
                status = playerStatus::playerPausing;
            }
        }
        else
        {
            wav.pause();
            status = playerStatus::playerPaused;
        }
    }

    void resume()
    {
        if (status == playerStatus::playerStopping || status == playerStatus::playerStopped || status == playerStatus::playerPlaying)
        {
            return;
        }

        if (status == playerStatus::playerPausing)
        {
            wav.pause();
        }

        wav.setVolume(base_volume);
        wav.resume();
        status = playerStatus::playerPlaying;
    }

    [[nodiscard]] float getVolume() const
    {
        return base_volume;
    }

    void setVolume(float volume)
    {
        base_volume = volume;
        wav.setVolume(volume);
    }

protected:
    Player() = default;
    void poll()
    {
        if (status == playerStatus::playerStopping && wav.getVolume() == 0)
        {
            wav.stop();
        }
        else if (status == playerStatus::playerPausing && wav.getVolume() == 0)
        {
            wav.pause();
            status = playerStatus::playerPaused;
        }

        if (wav.getStatus() == AudioSourceStopped)
        {
            status = playerStatus::playerStopped;
        }
    }

    playerStatus status{ playerStatus::playerStopped };
    bool busy{ false };
    float base_volume{ 1.0F };
    WavPlayer wav;
};

class PlayersPool
{
    /*
     * This class represents a list of players and 'synchronized'
     * here means that the list is accessed through the
     * acquire() and release() methods, used by the io_mode in
     * which there are 10 players/channels shared between 16 inputs.
     *
     * In serial_mode and latched_mode, the list is accessed by an
     * index using the get() function.
     */

private:
    PlayersPool() = default;

    std::array<Player, max_players> players{};

    bool initialized{ false };
    bool synchronized{ true };

public:
    void initialize(bool synchronized = true)
    {
        this->synchronized = synchronized;
        initialized = true;
    }

    static PlayersPool& getInstance()
    {
        static PlayersPool pool;
        return pool;
    }

    Player* acquire()
    {
        if (!synchronized || !initialized)
        {
            return nullptr;
        }

        for (uint8_t i = 0; i < max_players; i++)
        {
            if (!players[i].busy)
            {
                players[i].busy = true;
                return &players[i];
            }
        }

        return nullptr;
    }

    void release(Player* player) const
    {
        if (!synchronized || !initialized || (player == nullptr))
        {
            return;
        }

        // Ensure stopped state
        player->stop();
        player->busy = false;
    }

    Player* get(uint8_t num)
    {
        if (synchronized || !initialized)
        {
            return nullptr;
        }

        if (num > max_players)
        {
            return nullptr;
        }

        players[num].busy = true;
        return &players[num];
    }

    void poll()
    {
        if (!initialized)
        {
            return;
        }

        for (uint8_t i = 0; i < max_players; i++)
        {
            if (players[i].busy)
            {
                players[i].poll();
            }
        }
    }

    void stopAll(bool ramp_volume = false)
    {
        if (!initialized)
        {
            return;
        }

        for (uint8_t i = 0; i < max_players; i++)
        {
            if (players[i].busy)
                players[i].stop(ramp_volume);
        }
    }

    void pauseAll(bool ramp_volume = false)
    {
        if (!initialized)
        {
            return;
        }

        for (uint8_t i = 0; i < max_players; i++)
        {
            if (players[i].busy)
            {
                players[i].pause(ramp_volume);
            }
        }
    }

    void resumeAll()
    {
        if (!initialized)
        {
            return;
        }

        for (uint8_t i = 0; i < max_players; i++)
        {
            if (players[i].busy)
            {
                players[i].resume();
            }
        }
    }

    void releaseAll()
    {
        if (!initialized || !synchronized)
        {
            return;
        }

        for (uint8_t i = 0; i < max_players; i++)
        {
            release(&players[i]);
        }
    }

    bool playing()
    {
        AudioSourceStatus status;

        for (uint8_t i = 0; i < max_players; i++)
        {
            status = players[i].wav.getStatus();
            if (status == AudioSourcePlaying || status == AudioSourcePaused)
            {
                return true;
            }
        }

        return false;
    }

    static constexpr auto getMaxPlayers()
    {
        return max_players;
    }
};

#endif // __PLAYER_H__
