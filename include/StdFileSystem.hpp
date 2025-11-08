#pragma once

#include "FileSystem.hpp"

#include <filesystem>

namespace fs
{

    // std::filesystem implementation

    class FileSystem::std_fs : public ConceptFs
    {
    public:
        explicit std_fs(std::string dir) : root_dir{ std::move(dir) }
        {
        }

        [[nodiscard]] std::vector<std::string> list_files() const override
        {
            auto list = std::vector<std::string>{};
            for (const auto& file : std::filesystem::directory_iterator{ root_dir })
            {
                list.emplace_back(file.path().generic_string());
            }

            return list;
        }

    private:
        std::string root_dir;
    };


} // namespace fs