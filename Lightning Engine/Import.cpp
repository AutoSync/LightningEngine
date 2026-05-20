#include "Import.h"

const char* Lightning::ReadFile(const char* path, const char* name)
{
    std::string source;
    std::ifstream readfile;
    // ensure ifstream objects can throw exceptions:
    readfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        readfile.open(path);
        std::stringstream filestringstream;
        // read file's buffer contents into streams
        filestringstream << readfile.rdbuf();
        // close file handlers
        readfile.close();
        // convert stream into string
        source = filestringstream.str();
    }
    catch (std::ifstream::failure& e)
    {
		std::cout << "Error to Read file: "<< name << "From the Path: "<< path << e.what() <<  std::endl;
    }

	return source.c_str();
}
