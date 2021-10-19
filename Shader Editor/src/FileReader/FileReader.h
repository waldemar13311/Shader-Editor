#ifndef FILE_READER_H
#define FILE_READER_H

#include <vector>
#include <string>

using std::vector;
using std::string;

class FileReader
{
public:
	static vector<string> loadFromAsVector(const string &fileName);
	static string loadFromAsStr(const string &fileName);
};

#endif