#pragma once

#include <gladContainer.h>
#include <iostream>
#include <fstream>
#include <sstream>

struct FileImport {
    static std::string openFile(std::string path) {
        try 
        {
            std::ifstream file;
            file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            // open files
            file.open(path);

            std::stringstream fileStream;

            // read file's buffer contents into streams
            fileStream << file.rdbuf();

            // close file handlers
            file.close();

            // convert stream into string
            return fileStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "Successfully failed to load file " << path << "\n";
            return "FAIL";
        }
    };
};

class Shader {
public:
    GLuint ID;
    Shader(const std::string vertexPath, const std::string fragmentPath);

    void Activate();
    void Delete();

    void compileErrors(unsigned int shader, const char* type);
};

class Compute {
public:
    GLuint ID;
    Compute(const std::string path);

    void Activate();
    void Delete();
    void Dispatch(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ);
    void Wait();

    void compileErrors(unsigned int shader, const char* type);
};