#include "FileReader.h"

#include <fstream>

using std::ifstream;
using std::getline;

vector<string> FileReader::loadFromAsVector(const string &fileName)
{
	vector<string> result;

	ifstream fReader(fileName);

	string lineFromFile;
	while (getline(fReader, lineFromFile))
	{
		result.push_back(lineFromFile);
	}

	return result;
}

string FileReader::loadFromAsStr(const string & fileName)
{
	string oneStr;
	for (const auto & line : loadFromAsVector(fileName))
	{
		oneStr += line + "\n";
	}

	return oneStr;
}
