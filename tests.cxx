#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <vector>

SCENARIO("Sound files are parsed successfully")
{
    GIVEN("sound files names")
    {
        const auto files = std::vector<std::string>{ "1-1.wav", "1-2.wav" };
    }
}
