#pragma once

#undef min
#undef max

#include <memory>
#include <string>
#include <vector>


namespace fs
{
    class ConceptFs
    {
    public:
        ConceptFs() = default;
        ConceptFs(const ConceptFs&) = default;
        ConceptFs(ConceptFs&&) = delete;
        ConceptFs& operator=(const ConceptFs&) = default;
        ConceptFs& operator=(ConceptFs&&) = delete;
        virtual ~ConceptFs() = default;
        [[nodiscard]] virtual auto list_files() const -> std::vector<std::string> = 0;
    };

    class FileSystem
    {
    public:
        explicit FileSystem(const std::string&);
        [[nodiscard]] auto list_files() const -> std::vector<std::string>
        {
            return impl->list_files();
        }

    private:
        class std_fs;
        class usd_fs;

        std::unique_ptr<class ConceptFs> impl;
    };


} // namespace fs