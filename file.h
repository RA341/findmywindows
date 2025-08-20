#ifndef FINDMYWINDOWS_FILE_H
#define FINDMYWINDOWS_FILE_H

#include <string>
#include <vector>

void write_strings_to_file(const std::string& filename, const std::vector<std::string>& strings);

std::vector<std::string> read_strings_from_file(const std::string& filename);

#endif //FINDMYWINDOWS_FILE_H
