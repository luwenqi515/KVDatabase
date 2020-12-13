#pragma once
#include<string>
struct item
{
	std::string key;
	int offset;
	item* next;
	item() :key("empty"), offset(0), next(NULL) {}
	item(std::string k, int o) : key(k), offset(o), next(NULL) {}
};
class Hash
{
private:
	static const int tableSize = 11;
	item* HashTable[tableSize];
public:
	Hash();
	~Hash();
	int hashFunction(std::string key);
	void AddItem(std::string key, int offset);
	int Find(std::string key);
	int GetOffset(std::string key);
	void SetOffset(std::string key, int offset);
	void RemoveItem(std::string key);

};

