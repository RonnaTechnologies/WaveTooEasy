#pragma once

#undef min
#undef max

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace fs
{
    struct ReadResult
    {
        bool ok{ false };
        std::vector<std::uint8_t> data;
    };

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
        [[nodiscard]] virtual auto file_exists(const std::string& filename) const -> bool = 0;
        [[nodiscard]] virtual auto write_file(const std::string& filename, const std::vector<std::uint8_t>& data) const
        -> bool = 0;
        [[nodiscard]] virtual auto read_file(const std::string& filename) const -> ReadResult = 0;
    };

    class FileSystem
    {
    public:
        explicit FileSystem(const std::string&);
        [[nodiscard]] auto list_files() const -> std::vector<std::string>
        {
            return impl->list_files();
        }

        [[nodiscard]] auto file_exists(const std::string& filename) const -> bool
        {
            return impl->file_exists(filename);
        }

        [[nodiscard]] auto write_file(const std::string& filename, const std::vector<std::uint8_t>& data) const -> bool
        {
            return impl->write_file(filename, data);
        }

        [[nodiscard]] auto read_file(const std::string& filename) const -> ReadResult
        {
            return impl->read_file(filename);
        }

    private:
        class std_fs;
        class usd_fs;

        std::unique_ptr<class ConceptFs> impl;
    };


} // namespace fs