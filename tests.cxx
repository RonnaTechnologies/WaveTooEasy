#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "include/FileSystem.hpp"

#include <string_view>


SCENARIO("Sound files are parsed successfully")
{
    GIVEN("sound files names")
    {
        const auto dir = fs::FileSystem{ "../data" };
        const auto list = dir.list_files();

        for (const auto& file_name : list)
        {
            const auto slash_pos = static_cast<std::ptrdiff_t>(file_name.find_last_of('/'));
            const auto name = std::string_view{ file_name.begin() + slash_pos + 1, file_name.end() - 4 };

            const auto dash_pos = static_cast<std::ptrdiff_t>(name.find_first_of('-'));

            const auto note_str = std::string_view{ name.begin(), name.begin() + dash_pos };
            const auto index_str = std::string_view{ name.begin() + dash_pos + 1, name.end() };

            [[maybe_unused]] auto a = 1;
        }
    }
}
