#include "file.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void write_strings_to_file(const std::string& filename, const std::vector<std::string>& strings)
{
    if (std::ofstream output_file(filename); output_file.is_open())
    {
        for (const auto& str : strings)
        {
            output_file << str << std::endl;
        }
        output_file.close();
    }
    else
    {
        std::cerr << "Unable to open file for writing: " << filename << std::endl;
    }
}

std::vector<std::string> read_strings_from_file(const std::string& filename)
{
    std::ifstream file_stream(filename);

    if (!file_stream.is_open())
    {
        std::cout << "File not found: " << filename << ". Creating a default file." << std::endl;

        if (std::ofstream output_file(filename); !output_file)
        {
            std::cerr << "Error: Could not create the default file: " << filename << std::endl;
            return {};
        }

        // After creating, try opening it again for reading.
        file_stream.open(filename);
        if (!file_stream.is_open())
        {
            std::cerr << "Fatal Error: Failed to open the newly created config file." << std::endl;
            return {}; // Return empty
        }
    }

    std::vector<std::string> lines;
    std::string current_line;
    while (std::getline(file_stream, current_line))
    {
        if (!current_line.empty())
        {
            lines.push_back(current_line);
        }
    }
    return lines;
}
