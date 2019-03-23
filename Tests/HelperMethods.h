#pragma once

#include <string>
#include <fstream>

bool CreateResourceFile(std::string _filename, uint32_t _amountToWrite = 100)
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