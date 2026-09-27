#pragma once

#include "FileSystem.hpp"

#include <filesystem>
#include <fstream>

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

        [[nodiscard]] bool file_exists(const std::string& filename) const override
        {
            return std::filesystem::exists(filename);
        }

        [[nodiscard]] bool write_file(const std::string& filename, const std::vector<std::uint8_t>& data) const override
        {
            std::ofstream out{ filename, std::ios::binary | std::ios::trunc };
            if (!out.is_open())
            {
                return false;
            }

            out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
            return out.good();
        }
        [[nodiscard]] ReadResult read_file(const std::string& filename) const override
        {
            std::ifstream in{ filename, std::ios::binary | std::ios::ate };
            if (!in.is_open())
            {
                return {};
            }

            const auto size = in.tellg();
            if (size < 0)
            {
                return {};
            }

            in.seekg(0, std::ios::beg);

            auto data = std::vector<std::uint8_t>(static_cast<std::size_t>(size));
            if (!in.read(reinterpret_cast<char*>(data.data()), size))
            {
                return {};
            }

            return ReadResult{ .ok = true, .data = std::move(data) };
        }

    private:
        std::string root_dir;
    };


} // namespace fs