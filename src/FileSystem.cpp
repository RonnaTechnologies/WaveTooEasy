#include "../include/FileSystem.hpp"

#ifdef TESTS_ENABLED

#include "../include/StdFileSystem.hpp"

fs::FileSystem::FileSystem(const std::string& dir) : impl{ std::make_unique<FileSystem::std_fs>(dir) }
{
}

#else

#include "../include/SDFileSystem.hpp"

fs::FileSystem::FileSystem(const std::string& dir) : impl{ std::make_unique<FileSystem::usd_fs>(dir) }
{
}

#endif
