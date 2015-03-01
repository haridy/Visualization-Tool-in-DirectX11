//--------------------------------------------------------------------------------------
// File: tools.h
//
//--------------------------------------------------------------------------------------

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <vector>
#include <fstream>
#include <string>
#include <Windows.h>
#include "DXUT.h"

//Reads a file bytewise into a char-vector. In case of an error a message box
//is opened and an empty vector is returned.
inline std::vector<char> ReadFileRaw(const std::string& strFileName)
{
    std::ifstream file(strFileName, std::ios::in | std::ios::binary);

    if (file.fail()) {
        MessageBoxA(DXUTGetHWND(), ("Error opening " + strFileName).c_str(), 
                                    "ReadFileRaw Error", MB_OK | MB_ICONERROR);
        return std::vector<char>();
    }

    file.seekg(0, std::ios::end);
    size_t bytesize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> result(bytesize);
    file.read(result.data(), result.size());

    if (file.fail()) {
        MessageBoxA(DXUTGetHWND(), ("Error reading " + strFileName).c_str(), 
                                    "ReadFileRaw Error", MB_OK | MB_ICONERROR);
        return std::vector<char>();
    }

    file.close();

    return result;
}

#endif //__TOOLS_H__