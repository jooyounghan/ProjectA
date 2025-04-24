#include "ISerializable.h"

using namespace std;

void SerializeHelper::SerializeString(ofstream& ofs, const string& str)
{
	size_t length = str.size();
	ofs.write(reinterpret_cast<const char*>(&length), sizeof(length));
	ofs.write(str.c_str(), length);
}

std::string SerializeHelper::DeserializeString(ifstream& ifs)
{
	size_t length;
	ifs.read(reinterpret_cast<char*>(&length), sizeof(length));
	std::string str(length, '\0');
	ifs.read(&str[0], length);
	return str;
}
