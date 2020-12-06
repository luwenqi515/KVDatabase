#pragma once
#include<iostream>
#include<string>
#include<fstream>
#include<queue>
#include"Hash.h"
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
public:
	KVDBHandler(const std::string& db_file);
	~KVDBHandler();
	long getLength();//获取文件大小
	void getHash();
	void getQueue();
	friend int set(KVDBHandler* handler, const std::string& key, const std::string& value);
	friend int get(KVDBHandler* handler, const std::string& key, std::string& value);
	friend int del(KVDBHandler* handler, const std::string& key);
	friend int purge(KVDBHandler* handler);
	friend int expires(KVDBHandler* handler, const std::string key, int n);
};

