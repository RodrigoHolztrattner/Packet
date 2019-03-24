#pragma once

#include <string>
#include <fstream>
#include <functional>

static bool CreateResourceFile(std::string _filename, uint32_t _amountToWrite = 100)
{
    if (std::filesystem::path(_filename).has_parent_path())
    {
        std::filesystem::create_directory(std::filesystem::path(_filename).parent_path());
    }

    std::ofstream myfile(_filename);
    if (myfile.is_open())
    {
        for (int i = 0; i < _amountToWrite; i++)
        {
            myfile << std::to_string(_amountToWrite);
        }

        myfile.close();

        return true;
    }

    return false;
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