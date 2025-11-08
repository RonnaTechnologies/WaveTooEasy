#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "include/FileSystem.hpp"

SCENARIO("Sound files are parsed successfully")
{
    GIVEN("sound files names")
    {
        fs::FileSystem dir{ "../data" };
        [[maybe_unused]] const auto list = dir.list_files();
        [[maybe_unused]] auto a = 1;
    }
}
