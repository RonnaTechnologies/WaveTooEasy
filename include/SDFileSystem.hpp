#pragma once

#include "FileSystem.hpp"

#include "ff.h"

namespace fs
{
    class FileSystem::usd_fs : public ConceptFs
    {
    public:
        explicit usd_fs(std::string dir) : root_dir{ std::move(dir) }
        {
        }

        [[nodiscard]] std::vector<std::string> list_files() const override
        {

            DIR dir{};
            const auto res = f_opendir(&dir, root_dir.c_str());
            if (res != FR_OK)
            {
                return {};
            }

            auto list = std::vector<std::string>{};

            for (;;)
            {
                FILINFO file_info;
                const auto result = f_readdir(&dir, &file_info);

                if (result != FR_OK || file_info.fname[0] == 0)
                {
                    break;
                }

                list.emplace_back(&file_info.fname[0]);
            }

            f_closedir(&dir);

            return list;
        }

        [[nodiscard]] bool file_exists(const std::string& filename) const override
        {
            FILINFO info{};
            return f_stat(filename.c_str(), &info) == FR_OK;
        }

        [[nodiscard]] bool write_file(const std::string& filename, const std::vector<std::uint8_t>& data) const override
        {
            FIL file{};
            if (f_open(&file, filename.c_str(), FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
            {
                return false;
            }

            UINT bytes_written = 0;
            const auto result = f_write(&file, data.data(), static_cast<UINT>(data.size()), &bytes_written);
            f_close(&file);

            return result == FR_OK && bytes_written == data.size();
        }

        [[nodiscard]] ReadResult read_file(const std::string& filename) const override
        {
            FIL file{};
            if (f_open(&file, filename.c_str(), FA_READ) != FR_OK)
            {
                return {};
            }

            const auto size = f_size(&file);
            auto data = std::vector<std::uint8_t>(size);

            UINT bytes_read = 0;
            const auto result = f_read(&file, data.data(), static_cast<UINT>(size), &bytes_read);
            f_close(&file);

            if (result != FR_OK || bytes_read != size)
            {
                return {};
            }

            return ReadResult{ .ok = true, .data = std::move(data) };
        }

    private:
        std::string root_dir;
    };


} // namespace fs