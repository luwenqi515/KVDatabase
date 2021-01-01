#include "Hash.h"
#include"KVDBHandler.h"
Hash::Hash()
{
	for (int i = 0; i < tableSize; i++)
	{
		HashTable[i] = new item();
	}
}
Hash::~Hash()
{
	for (int i = 0; i < tableSize; i++)
	{
		delete HashTable[i];
	}
}
void Hash::clear()
{
	for (int i = 0; i < tableSize; i++)
	{
		HashTable[i] = new item();
	}

}
int Hash::hashFunction(std::string key)
{
	int sum = 0;
	int index;
	for (int i = 0; i < key.size(); i++)
	{
		sum += static_cast<int>(key[i]);
	}
	index = sum % tableSize;
	return index;
}
void Hash::AddItem(std::string key, int offset,int ktime)
{
	int index = hashFunction(key);
	if (HashTable[index]->key == "empty")
	{
		HashTable[index]->key = key;
		HashTable[index]->offset = offset;
		HashTable[index]->ktime = ktime;
	}
	else
	{
		item* p = HashTable[index];
		item* n = new item(key, offset,ktime);
		while (p->next != NULL)
		{
			p = p->next;
		}
		p->next = n;
	}
}
int Hash::Find(std::string key)
{
	int index = hashFunction(key);
	bool FindKey = false;
	item* p = HashTable[index];
	while (p != NULL)
	{
		if (p->key == key)
		{
			FindKey = true;
			break;
		}
		p = p->next;
	}
	return FindKey;
}
int Hash::GetOffset(std::string key)
{
	int index = hashFunction(key);
	item* p = HashTable[index];
	while (p != NULL)
	{
		if (p->key == key)
		{
			return p->offset;
			break;
		}
		p = p->next;
	}
}
int Hash::GetKtime(std::string key)
{
	int index = hashFunction(key);
	item* p = HashTable[index];
	while (p != NULL)
	{
		if (p->key == key)
		{
			return p->ktime;
			break;
		}
		p = p->next;
	}

}
void Hash::SetKtime(std::string key, int ktime)
{
	int index = hashFunction(key);
	item* p = HashTable[index];
	while (p != NULL)
	{
		if (p->key == key)
		{
			p->ktime=ktime;
			break;
		}
		p = p->next;
	}

}
void Hash::SetOffset(std::string key, int offset)
{
	int index = hashFunction(key);
	item* p = HashTable[index];
	while (p != NULL)
	{
		if (p->key == key)
		{
			p->offset = offset;
			break;
		}
		p = p->next;
	}
}
void Hash::RemoveItem(std::string key)
{
	int index = hashFunction(key);

	item* delPtr;
	item* p1;
	item* p2;
	if (HashTable[index]->key == key && HashTable[index]->next == NULL)
	{
		HashTable[index]->key = "empty";
		HashTable[index]->offset = 0;
		HashTable[index]->ktime = 0;
	}
	else if (HashTable[index]->key == key&&HashTable[index]->next!=NULL)
	{
		delPtr = HashTable[index];
		HashTable[index] = HashTable[index]->next;
		delete delPtr;
	}
	else
	{
		p1 = HashTable[index]->next;
		p2 = HashTable[index];
		while (p1 != NULL && p1->key != key)
		{
			p2 = p1;
			p1 = p1->next;
		}
		delPtr = p1;
		p2->next = p1->next;
		delete delPtr;
	}
}
