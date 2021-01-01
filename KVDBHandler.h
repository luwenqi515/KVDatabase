#pragma once
#include<iostream>
#include<string>
#include<fstream>
#include<queue>
#include "Hash.h"
#include "LRUCache.h"
const int  max_capacity = 3;
struct KeyTime
{
	std::string key;
	int time;
	bool operator <(const KeyTime& x)const
	{
		return time > x.time;
	}
};
class KVDBHandler
{
private:
	std::string filename;
	Hash hash;
	std::priority_queue<KeyTime> q;
	LRUCache LRU;
public:
	KVDBHandler(const std::string& db_file,int &flag);
	~KVDBHandler();
	long getLength();//获取磁盘大小
	void getHash_q();
	friend int set(KVDBHandler* handler, const std::string& key, const std::string& value,int ktime);
	friend int get(KVDBHandler* handler, const std::string& key, std::string& value);
	friend int del(KVDBHandler* handler, const std::string& key);
	friend int purge(KVDBHandler* handler);
	friend int expires(KVDBHandler* handler, const std::string key, int n);
	friend void purge_Traversehash(KVDBHandler* handler, KVDBHandler* savehandler);
};


