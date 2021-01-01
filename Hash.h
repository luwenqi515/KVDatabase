#pragma once
#include<string>

struct item
{
	std::string key;
	int offset;
	int ktime;
	item* next;
	item() :key("empty"),offset(0), ktime(0),next(NULL) {}
	item(std::string k, int o,int t) : key(k), offset(o), ktime(t),next(NULL) {}
};
class Hash
{
public:
	static const int tableSize = 11;
	item* HashTable[tableSize];
	Hash();
	~Hash();
	void clear();
	int hashFunction(std::string key);
	void AddItem(std::string key, int offset,int ktime);
	int Find(std::string key);
	int GetOffset(std::string key);
	int GetKtime(std::string key);
	void SetKtime(std::string key, int ktime);
	void SetOffset(std::string key, int offset);
	void RemoveItem(std::string key);
};

