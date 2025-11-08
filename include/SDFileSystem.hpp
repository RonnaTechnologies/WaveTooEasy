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

    private:
        std::string root_dir;
    };


} // namespace fs