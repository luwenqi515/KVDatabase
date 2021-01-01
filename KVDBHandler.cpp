#include "KVDBHandler.h"
#include"Log_Data.h"
#include<windows.h>
#include<map>
#include<queue>
#include<ctime>
#include<vector>

const long MAX_LENGTH = 1000000;
const int KVDB_OK = 0;                        //成功
const int KVDB_INVALID_AOF_PATH = 1;          //路径不存在
const int KVDB_INVALID_KEY = 2;               //key值无效
const int KVDB_INVALID_VALUE = 3;             //value值无效
const int KVDB_NO_SPACE_LEFT_ON_DEVICES = 4;  //磁盘空间不足
const int KVDB_NO_FIND_VALUE = 5;             //未找到value的值

struct Record
{
	std::string value;
	int ktime;
};

bool Freespace()//判断磁盘是否已满
{
	DWORD64 qwFreeBytes, qwFreeBytesToCaller, qwTotalBytes;
	bool bResult = GetDiskFreeSpaceEx(TEXT("E:"),
		(PULARGE_INTEGER)&qwFreeBytesToCaller,
		(PULARGE_INTEGER)&qwTotalBytes,
		(PULARGE_INTEGER)&qwFreeBytes);
	return bResult;
}

KVDBHandler::KVDBHandler(const std::string& db_file,int &flag)
{
	filename = db_file;

	Log_Data data;
	std::string text = filename;
	text += " OPEN ";

	std::fstream file;
	file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!file)
	{
		flag = 0;
		std::ofstream newfile(filename);
		newfile.close();
		text += "KVDB_INVALID_AOF_PATH";
		data.setLog_Data(LOG_TYPE_ERROR, text);
	}
	else
	{
		flag = 1;
		text += "KVDB_OK";
		data.setLog_Data(LOG_TYPE_INFO, text);
	}
	file.close();
	getHash_q();

}

KVDBHandler::~KVDBHandler()
{
	filename.clear();
}

long KVDBHandler::getLength()//获取文件大小
{
	std::ifstream in(filename);
	in.seekg(0, std::ios::end);
	std::streampos ps = in.tellg();
	in.close();
	return ps;
}

void KVDBHandler::getHash_q()
{
	std::fstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	int klen, vlen,ktime;
	char* str_key, * str_value;
	int offset = 0;
	std::vector<std::string> k;
	while (file.read((char*)&klen, sizeof(int)))
	{
		offset += sizeof(int);
		file.read((char*)&vlen, sizeof(int));
		str_key = new char[klen + 1];
		file.read(str_key, klen);
		str_key[klen] = 0;
		if (vlen >= 1)
		{
			file.seekg(vlen, std::ios::cur);
			file.read((char*)&ktime, sizeof(int));
			if (hash.Find(str_key))
			{
				hash.SetOffset(str_key, offset);
			}
			else
			{
				hash.AddItem(str_key, offset,ktime);
				k.push_back(str_key);
			}
			offset += sizeof(int);
			offset += klen;
			offset += vlen;
			offset += sizeof(int);
			delete[]str_key;
		}
		else
		{
			hash.RemoveItem(str_key);
			for (auto it = k.begin(); it != k.end(); it++)
			{
				if (*it == str_key)
				{
					k.erase(it);
					break;
				}
				else
					++it;
			}
		}
	}
	file.close();
	while (!q.empty())
	{
		q.pop();
	}
	for (auto it=k.begin ();it!=k.end() ;it++)
	{
		ktime=hash.GetKtime(*it);
		q.push({ *it,ktime });
	}
}

int set(KVDBHandler* handler, const std::string& key, const std::string& value,int ktime)
{
	Log_Data data;
	std::string text = handler->filename;
	text += " SET ";
	std::fstream file;
	file.open(handler->filename, std::ios::out | std::ios::binary | std::ios::app);
	if (!file)
	{
		text += "KVDB_INVALID_AOF_PATH";
		data.setLog_Data(LOG_TYPE_ERROR, text);
		return KVDB_INVALID_AOF_PATH;
	}
	else
	{
		int klen, vlen;
		klen = key.length();
		vlen = value.length();
		if (klen == 0)
		{
			file.close();
			text += "KVDB_INVALID_KEY";
			data.setLog_Data(LOG_TYPE_ERROR, text);
			return KVDB_INVALID_KEY;
		}
		else if (!Freespace())
		{
			file.close();
			text += " KVDB_NO_SPACE_LEFT_ON_DEVICES";
			data.setLog_Data(LOG_TYPE_ERROR, text);
			return KVDB_NO_SPACE_LEFT_ON_DEVICES;
		}
		else
		{
			//offset定位到vlen
			handler->LRU.put(key, value);
			file.seekg(0, std::ios::end);
			int offset = file.tellg();
			file.write((char*)&klen, sizeof(int));
			offset += sizeof(int);
			file.write((char*)&vlen, sizeof(int));
			file.write(key.c_str(), klen);
			file.write(value.c_str(), vlen);
			file.write((char*)&ktime, sizeof(int));
			file.close();
			if (handler->hash.Find(key))
			{
				handler->hash.SetOffset(key, offset);
			}
			else
			{
				handler->hash.AddItem(key, offset,ktime);
			}
			text += key;
			text += value;
			data.setLog_Data(LOG_TYPE_INFO, text);
			return KVDB_OK;
		}
	}
}

int get(KVDBHandler* handler, const std::string& key, std::string& value)
{
	Log_Data data;
	std::string text = handler->filename;
	text += " GET ";
	
	std::fstream file;
	file.open(handler->filename, std::ios::in | std::ios::binary);
	if (!file)
	{
		text += "KVDB_INVALID_AOF_PATH";
		data.setLog_Data(LOG_TYPE_ERROR, text);
		return KVDB_INVALID_AOF_PATH;
	}
	else
	{
		time_t current_time;
		time(&current_time);
		std::string _filename;
		int klen, time;
		while (!handler->q.empty() && handler->q.top().time!=0 && handler->q.top().time < current_time)
		{
			int ktime = handler->hash.GetKtime(handler->q.top().key);
			if (ktime == handler->q.top().time)
			{
				del(handler, handler->q.top().key);
			}
			handler->q.pop();
		}
		if (key.length() == 0)
		{
			file.close();
			text += "KVDB_INVALID_KEY";
			data.setLog_Data(LOG_TYPE_ERROR, text);
			return KVDB_INVALID_KEY;
		}
		else
		{
			value=handler->LRU.get(key);
			if (value.empty())
			{
				if (handler->hash.Find(key))
				{
					int offset = handler->hash.GetOffset(key);
					int vlen;
					file.seekg(offset, std::ios::beg);//定位到vlen
					file.read((char*)&vlen, sizeof(int));
					file.seekg(key.length() , std::ios::cur);//定位到value
					char* str_value;
					str_value = new char[vlen + 1];
					file.read(str_value, vlen);
					str_value[vlen] = 0;
					value = str_value;
					delete[]str_value;
					file.close();
					text += key;
					text += " ";
					text += value;
					text += " KVDB_OK";
					data.setLog_Data(LOG_TYPE_INFO, text);
					return KVDB_OK;
				}
				else
				{
					file.close();
					text += key;
					text += "  KVDB_NO_FIND_VALUE";
					data.setLog_Data(LOG_TYPE_INFO, text);
					return KVDB_NO_FIND_VALUE;
				}
			}
			else
			{
				//LRU中找到
				text += key;
				text += " ";
				text += value;
				text += " KVDB_OK";
				data.setLog_Data(LOG_TYPE_INFO, text);
				return KVDB_OK;
			}
		}
	}
}
int del(KVDBHandler* handler, const std::string& key)
{
	Log_Data data;
	std::string text = handler->filename;
	text += " DEL ";
	std::fstream file;
	file.open(handler->filename, std::ios::out | std::ios::binary | std::ios::app);
	if (!file)
	{
		text += "KVDB_INVALID_AOF_PATH";
		data.setLog_Data(LOG_TYPE_ERROR, text);
		return KVDB_INVALID_AOF_PATH;
	}
	else
	{
		int klen, vlen;
		klen = key.length();
		vlen = -1;
		if (klen == 0)
		{
			file.close();
			text += "KVDB_INVALID_KEY";
			data.setLog_Data(LOG_TYPE_ERROR, text);
			return KVDB_INVALID_KEY;
		}
		else
		{
			handler->LRU.del(key);
			file.write((char*)&klen, sizeof(int));
			file.write((char*)&vlen, sizeof(int));
			file.write(key.c_str(), klen);
			file.close();
			handler->hash.RemoveItem(key);
			text += key;
			text += " KVDB_OK";
			data.setLog_Data(LOG_TYPE_INFO, text);
			return KVDB_OK;
		}
	}
}
int purge(KVDBHandler* handler)
{
	Log_Data logdata;
	std::string text = handler->filename;
	text += " PURGE ";
	std::fstream file;
	file.open(handler->filename, std::ios::in | std::ios::binary);
	if (!file)
	{
		text += "KVDB_INVALID_AOF_PATH";
		logdata.setLog_Data(LOG_TYPE_ERROR, text);
		return KVDB_INVALID_AOF_PATH;
	}
	else
	{
		int flag;
		std::string savefilename = handler->filename + "save",text;
		KVDBHandler savehandler(savefilename,flag);
		purge_Traversehash(handler, &savehandler);
		purge_Traversehash(&savehandler, handler);
		handler->getHash_q();
		text += "KVDB_OK";
		logdata.setLog_Data(LOG_TYPE_INFO, text);
		return KVDB_OK;
	}
}
void purge_Traversehash(KVDBHandler *handler,KVDBHandler *savehandler)
{
	for (int i = 0; i < handler->hash.tableSize; i++)
	{
		item* p = handler->hash.HashTable[i];
		while (p != NULL)
		{
			if (p->key != "empty")
			{
				std::string value;
				get(handler, p->key, value);
				set(savehandler, p->key, value, p->ktime);
			}
			p = p->next;
		}
	}
	std::ofstream file_writer(handler->filename, std::ios_base::out);
	handler->hash.clear();
}


int expires(KVDBHandler* handler, const std::string key, int n)
{
	Log_Data data;
	std::string text = handler->filename;
	text += " EXPIRES ";

	std::fstream file;
	file.open(handler->filename, std::ios::out | std::ios::app);
	if (!file)
	{
		text += "KVDB_INVALID_AOF_PATH";
		data.setLog_Data(LOG_TYPE_ERROR, text);
		return KVDB_INVALID_AOF_PATH;
	}
	else
	{
		KeyTime k;
		k.key = key;
		time_t current_time;
		time(&current_time);
		std::string value;
		get(handler, key, value);
		int klen = key.length(),vlen=value.length ();
		k.time = n + current_time;

		handler->hash.SetKtime(k.key, k.time);
		set(handler, key, value, k.time);
		handler->q.push(k);
		file.close();

		text += key;
		text += " ";
		text += n;
		text += " KVDB_OK ";
		data.setLog_Data(LOG_TYPE_INFO, text);
		return KVDB_OK;
	}
}
