
#include "../include/FileSystem.hpp"
#include <vector>


#ifdef TESTS_ENABLED
#include <filesystem>
#else
#include "ff.h"
#endif


namespace fs
{

#ifdef TESTS_ENABLED

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


    // Constructor is a factory

    FileSystem::FileSystem(const std::string& dir) : impl{ std::make_unique<FileSystem::std_fs>(dir) }
    {
    }

#else

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


    // Constructor is a factory

    FileSystem::FileSystem(const std::string& dir) : impl{ std::make_unique<FileSystem::usd_fs>(dir) }
    {
    }
#endif

} // namespace fs