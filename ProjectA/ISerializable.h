#pragma once
#include <fstream>
#include <vector>
#include <string>

class ISerializable
{
public:
	virtual void Serialize(std::ofstream& ofs) = 0;
	virtual void Deserialize(std::ifstream& ifs) = 0;
};

class SerializeHelper
{
public:
	template<typename T>
	static void SerializeElement(std::ofstream& ofs, const T& elem);
	template<typename T>
	static T DeserializeElement(std::ifstream& ifs);


public:
	template<typename T>
	static void SerializeVector(std::ofstream& ofs, const std::vector<T>& v);
	template<typename T>
	static std::vector<T> DeserializeVector(std::ifstream& ifs);

public:
	static void SerializeString(std::ofstream& ofs, const std::string& str);
	static std::string DeserializeString(std::ifstream& ifs);
};

template<typename T>
void SerializeHelper::SerializeElement(std::ofstream& ofs, const T& elem)
{
	ofs.write(reinterpret_cast<const char*>(&elem), sizeof(T));
}

template<typename T>
T SerializeHelper::DeserializeElement(std::ifstream& ifs)
{
	T elem;
	ifs.read(reinterpret_cast<char*>(&elem), sizeof(T));
	return elem;
}

template<typename T>
void SerializeHelper::SerializeVector(std::ofstream& ofs, const std::vector<T>& v)
{
	size_t length = v.size();
	ofs.write(reinterpret_cast<const char*>(&length), sizeof(length));
	ofs.write(reinterpret_cast<const char*>(v.data()), length * sizeof(T));
}

template<typename T>
static std::vector<T> SerializeHelper::DeserializeVector(std::ifstream& ifs)
{
	size_t length;
	ifs.read(reinterpret_cast<char*>(&length), sizeof(length));

	std::vector<T> v;
	v.resize(length);
	ifs.read(reinterpret_cast<char*>(v.data()), length * sizeof(T));
	return v;

}

