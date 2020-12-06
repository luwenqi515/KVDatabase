#include "KVDBHandler.h"
#include"Log_Data.h"
#include<windows.h>
#include<map>
#include<queue>
#include<ctime>
const long MAX_LENGTH = 1000000;
const int KVDB_OK = 0;                        //成功
const int KVDB_INVALID_AOF_PATH = 1;          //路径不存在
const int KVDB_INVALID_KEY = 2;               //key值无效
const int KVDB_INVALID_VALUE = 3;             //value值无效
const int KVDB_NO_SPACE_LEFT_ON_DEVICES = 4;  //磁盘空间不足
const int KVDB_NO_FIND_VALUE = 5;             //未找到value的值


bool Freespace()//判断磁盘是否已满
{
	DWORD64 qwFreeBytes, qwFreeBytesToCaller, qwTotalBytes;
	bool bResult = GetDiskFreeSpaceEx(TEXT("E:"),
		(PULARGE_INTEGER)&qwFreeBytesToCaller,
		(PULARGE_INTEGER)&qwTotalBytes,
		(PULARGE_INTEGER)&qwFreeBytes);
	return bResult;
}

KVDBHandler::KVDBHandler(const std::string& db_file)
{
	filename = db_file;
	std::cout << "正在打开数据库..." << std::endl;

	Log_Data data;
	std::string text =filename;
	text += " OPEN ";

	std::fstream file;
	file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!file)
	{
		std::cout << "数据库不存在,已创建新的数据库！" << std::endl;
		std::ofstream newfile(filename);
		newfile.close();
		text += "KVDB_INVALID_AOF_PATH";
		data.setLog_Data(LOG_TYPE_ERROR, text);
	}
	else
	{
		std::cout << "数据库打开成功！" << std::endl;
		text += "KVDB_OK";
		data.setLog_Data(LOG_TYPE_INFO, text);
	}
	file.close();
	getHash();
	getQueue();
	
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
void KVDBHandler::getHash()
{
	std::fstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	int klen, vlen;
	char* str_key, * str_value;
	int offset = 0;
	while (file.read((char*)&klen, sizeof(int)))
	{
		offset += sizeof(int);
		file.read((char*)&vlen, sizeof(int));
		offset += sizeof(int);
		str_key = new char[klen + 1];
		file.read(str_key, klen);
		offset += klen;
		str_key[klen] = 0;
		//std::cout <<"str_key:"<< str_key <<std::endl;

		if (vlen >= 1)
		{
			file.seekg(vlen, std::ios::cur);
			int flag = hash.Find(str_key);
			if (flag == true)
			{
				hash.SetOffset(str_key, offset);
			}
			else
			{
				hash.AddItem(str_key, offset);
			}
			offset += vlen;
			delete[]str_key;
		}
		else
		{
			hash.RemoveItem(str_key);
		}
	}
}
void KVDBHandler::getQueue()
{
	std::string _filename;
	_filename = filename;
	_filename += "_time";
	std::fstream file;
	file.open(_filename, std::ios::out | std::ios::app);
	std::map<std::string, int> m;
	std::pair<std::map<std::string, int>::iterator, bool > Insert_Pair;
	int klen, time;
	char* str_key;
	while (file.read((char*)&klen, sizeof(int)))
	{
		str_key = new char[klen + 1];
		file.read(str_key, klen);
		str_key[klen] = 0;
		file.read((char*)&time, sizeof(int));
		Insert_Pair = m.insert(std::pair<std::string, int>(str_key, time));
		if (Insert_Pair.second == false)
		{
			m[str_key] = time;
		}
		delete[]str_key;
	}
	while (!q.empty())
	{
		q.pop();
	}
	for (auto it = m.begin(); it != m.end(); it++)
	{
		q.push({ it->first ,it->second });
	}
	m.clear();

}
int set(KVDBHandler* handler, const std::string& key, const std::string& value)
{
	Log_Data data;
	std::string text = handler->filename ;
	text+=" SET ";
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
			file.seekg(0, std::ios::end);
			int offset = file.tellg();
			file.write((char*)&klen, sizeof(int));
			offset += sizeof(int);
			file.write((char*)&vlen, sizeof(int));
			offset += sizeof(int);
			file.write(key.c_str(), klen);
			offset += klen;
			file.write(value.c_str(), vlen);
			file.close();
			if (handler->hash.Find(key))
			{
				handler->hash.SetOffset(key, offset);
			}
			else
			{
				handler->hash.AddItem(key, offset);
			}
			text += key;
			text += value;
			data.setLog_Data(LOG_TYPE_INFO,text);
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
		while (handler->q.top().time < current_time)
		{
			del(handler, handler->q.top().key);
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
			if (handler->hash.Find(key))
			{
				int offset = handler->hash.GetOffset(key);
				int vlen;
				file.seekg(offset - key.length() - sizeof(int), std::ios::beg);//定位到vlen
				file.read((char*)&vlen, sizeof(int));
				file.seekg(key.length(), std::ios::cur);//定位到value
				char* str_value;
				str_value = new char[vlen + 1];
				file.read(str_value, vlen);
				str_value[vlen] = 0;
				value = str_value;
				delete[]str_value;
				file.close();
				text += key;
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
			/*   stage1的get
			int klen, vlen;
			char* str_key, * str_value;
			while (file.read((char*)&klen, sizeof(int)))
			{
				file.read((char*)&vlen, sizeof(int));
				//std::cout << klen << " " << vlen << std::endl;
				str_key = new char[klen + 1];
				file.read(str_key, klen);
				str_key[klen] = 0;
				//std::cout <<"str_key:"<< str_key <<std::endl;
				if (vlen == -1)
				{
					if (str_key == key)
					{
						value.clear();
					}
				}
				else
				{
					if (str_key == key)
					{
						value.clear();
						str_value = new char[vlen + 1];
						file.read(str_value, vlen);
						str_value[vlen] = 0;
						//std::cout << str_value << std::endl;
						value = str_value;
						delete[]str_key;
						delete[]str_value;
					}
					else
					{
						file.seekg(vlen, std::ios::cur);
					}
				}
			}
			file.close();
			if (value.empty())
				return KVDB_NO_FIND_VALUE;
			else
				return KVDB_OK;
				*/
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
		std::map<std::string, std::string> data;
		std::pair<std::map<std::string, std::string>::iterator, bool > Insert_Pair;
		int klen, vlen;
		char* str_key, * str_value;
		while (file.read((char*)&klen, sizeof(int)))
		{
			file.read((char*)&vlen, sizeof(int));
			//std::cout << klen << " " << vlen << std::endl;
			str_key = new char[klen + 1];
			file.read(str_key, klen);
			str_key[klen] = 0;
			//std::cout <<"str_key:"<< str_key <<std::endl;

			if (vlen >= 1)
			{
				str_value = new char[vlen + 1];
				file.read(str_value, vlen);
				str_value[vlen] = 0;
				//std::cout << str_value << std::endl;
				Insert_Pair = data.insert(std::pair<std::string, std::string>(str_key, str_value));
				if (Insert_Pair.second == false)
				{
					data[str_key] = str_value;
				}
				delete[]str_key;
				delete[]str_value;
			}
			else
			{
				data.erase(str_key);
			}
		}
		std::string file_name = handler->filename;
		std::ofstream file_writer(file_name, std::ios_base::out);

		for (auto it = data.begin(); it != data.end(); it++)
		{
			set(handler, it->first, it->second);
			//std::cout << it->first << " " << it->second << std::endl;
		}
		data.clear();
		file.close();
		text += "KVDB_OK";
		logdata.setLog_Data(LOG_TYPE_INFO, text);
		return KVDB_OK;
	}
}
int findkey(std::priority_queue<KeyTime> q, std::string key)
{
	while (!q.empty())
	{
		if (key == q.top().key)
		{
			return true;
			break;
		}
		q.pop();
	}
	return false;
}

int expires(KVDBHandler* handler, const std::string key, int n)
{
	Log_Data data;
	std::string text = handler->filename;
	text += " EXPIRES ";

	std::string _filename;
	_filename = handler->filename;
	_filename += "_time";
	std::fstream file;//存放过期时间的文件
	file.open(_filename, std::ios::out | std::ios::app);
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
		k.time = n + current_time;
		file >> k.key >> k.time;
		if (findkey(handler->q, key))
		{
			handler->getQueue();
		}
		else
		{
			handler->q.push(k);
		}
		text += key;
		text += n;
		text += " KVDB_OK ";
		data.setLog_Data(LOG_TYPE_INFO, text);
		return KVDB_OK;
	}
}