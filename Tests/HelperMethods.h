#pragma once

#include <string>
#include <fstream>
#include <functional>
#include <filesystem>
#include "HelperDefines.h"

// Forward
static void SetupResourcePlayground();
static bool CreateResourceFile(std::string _filename, uint32_t _amountToWrite = 100);
static bool UpdateResourceFile(std::string _filename, uint32_t _amountToWrite = 100);
static bool MustChangeToTrueUntilTimeout(std::function<bool()> _condition, long long _timeout);
static bool MustNotChangeToFalseUntilTimeout(std::function<bool()> _condition, long long _timeout);
static bool MustChangeToTrueAfterTimeout(std::function<bool()> _condition, long long _timeout);
static bool MustChangeToFalseAfterTimeout(std::function<bool()> _condition, long long _timeout);

static void SetupResourcePlayground()
{
    // Clear old data
    std::filesystem::remove_all(ResourceDirectory);
    std::filesystem::remove(ExternalFilePath);

    // Base data dir and other directories
    std::filesystem::create_directory(ResourceDirectory);
    std::filesystem::create_directory(ImagesDirectory);
    std::filesystem::create_directory(SoundsDirectory);
    std::filesystem::create_directory(ShadersDirectory);

    // Initial files
    CreateResourceFile(ImageFilePath);
    CreateResourceFile(ShaderFilePath);
    CreateResourceFile(DummyFilePath);
    CreateResourceFile(ExternalFilePath);
}

static bool CreateResourceFile(std::string _filename, uint32_t _amountToWrite)
{
    if (std::filesystem::path(_filename).has_parent_path())
    {
        std::filesystem::create_directory(std::filesystem::path(_filename).parent_path());
    }

    std::ofstream myfile(_filename);
    if (myfile.is_open())
    {
        for (int i = 0; i < static_cast<int>(_amountToWrite); i++)
        {
            myfile << std::to_string(_amountToWrite);
        }

        myfile.close();

        return true;
    }

    return false;
}

static bool UpdateResourceFile(std::string _filename, uint32_t _amountToWrite)
{
    std::ofstream myfile(_filename);
    if (myfile.is_open())
    {
        for (int i = 0; i < static_cast<int>(_amountToWrite); i++)
        {
            myfile << std::to_string(_amountToWrite);
        }

        myfile.close();

        return true;
    }

    return false;
}

static std::vector<uint8_t> LoadFileIntoRawData(std::filesystem::path _file_path)
{
    std::ifstream file(_file_path, std::ios::binary);
    if (!file.is_open())
    {
        return {};
    }

    std::vector<uint8_t> entire_file_data;
    entire_file_data.reserve(std::filesystem::file_size(_file_path));

    file.unsetf(std::ios::skipws);
    std::copy(std::istream_iterator<uint8_t>(file),
              std::istream_iterator<uint8_t>(),
              std::back_inserter(entire_file_data));

    file.close();

    return entire_file_data;
}
static bool MustChangeToTrueUntilTimeout(std::function<bool()> _condition, long long _timeout)
{
    clock_t initialTime = clock();
    clock_t currentTime = initialTime;
    while (true)
    {
        if (double(currentTime - initialTime) / CLOCKS_PER_SEC >= double(_timeout) / 1000)
        {
            return false;
        }

        if (_condition())
        {
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(3));

        currentTime = clock();
    }

    // Should never happen
    return false;
}

static bool MustNotChangeToFalseUntilTimeout(std::function<bool()> _condition, long long _timeout)
{
    clock_t initialTime = clock();
    clock_t currentTime = initialTime;
    while (true)
    {
        if (double(currentTime - initialTime) / CLOCKS_PER_SEC >= double(_timeout) / 1000)
        {
            return true;
        }

        if (!_condition())
        {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(3));

        currentTime = clock();
    }

    // Should never happen
    return true;
}

static bool MustChangeToTrueAfterTimeout(std::function<bool()> _condition, long long _timeout)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(_timeout));

    return _condition();
}

static bool MustChangeToFalseAfterTimeout(std::function<bool()> _condition, long long _timeout)
{
    return !MustChangeToTrueAfterTimeout(_condition, _timeout);
}