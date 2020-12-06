#include<iostream>
#include<string>
#include"KVDBHandler.h"

const long MAX_LENGTH = 1000000;
const int KVDB_OK = 0;                        //成功
const int KVDB_INVALID_AOF_PATH = 1;          //路径不存在
const int KVDB_INVALID_KEY = 2;               //key值无效
const int KVDB_INVALID_VALUE = 3;             //value值无效
const int KVDB_NO_SPACE_LEFT_ON_DEVICES = 4;  //磁盘空间不足
const int KVDB_NO_FIND_VALUE = 5;             //未找到value的值

int main()
{
	std::string file;
	std::cout << "请输入要打开的数据库：";
	std::cin >> file;
	int flag;
	KVDBHandler handler((const std::string)file);
	int command;
	while (true)
	{
		std::cout << "请输入操作命令:\n";
		std::cout << "             [1]写入" << std::endl;
		std::cout << "             [2]读取" << std::endl;
		std::cout << "             [3]删除" << std::endl;
		std::cout << "             [4]定时删除" << std::endl;
		std::cout << "             [5]退出" << std::endl;
		std::cin >> command;
		getchar();
		if (command == 1)
		{
			std::string key, value;
			std::cout << "请输入key和value：" << std::endl;
			getline(std::cin, key);
			getline(std::cin, value);
			flag = set(&handler, key, value);
			if (flag == 0)
				std::cout << "写入成功！" << std::endl;
			else if (flag == 1)
			{
				std::cout << "路径不存在！" << std::endl;
			}
			else if (flag == 2)
			{
				std::cout << "KEY无效，请重新操作！" << std::endl;
			}
			else if (flag == 4)
			{
				std::cout << "磁盘已满，无法写入！" << std::endl;
			}
			system("pause");
			system("cls");
		}
		else if (command == 2)
		{
			std::string key, value;
			std::cout << "请输入key:" << std::endl;
			//std::cin >> key;
			getline(std::cin, key);
			//std::cout << key << std::endl;
			int flag;
			flag = get(&handler, key, value);
			if (flag == 0)
			{
				std::cout << "读取成功！" << std::endl;
				std::cout << key << "对应的value为：" << value << std::endl;
			}
			else if (flag == 1)
			{
				std::cout << "路径不存在！" << std::endl;
			}
			else if (flag == 2)
			{
				std::cout << "KEY无效，请重新操作！" << std::endl;
			}
			else if (flag == 5)
			{
				std::cout << "未读取到key相关的value！" << std::endl;
			}
			system("pause");
			system("cls");
		}
		else if (command == 3)
		{
			std::string key;
			std::cout << "请输入需要删除的key:" << std::endl;
			//std::cin >> key;
			getline(std::cin, key);
			int flag;
			flag = del(&handler, key);
			if (flag == 0)
			{
				std::cout << "删除成功！" << std::endl;
			}
			else if (flag == 1)
			{
				std::cout << "路径不存在！" << std::endl;
			}
			else if (flag == 2)
			{
				std::cout << "KEY无效，请重新操作！" << std::endl;

			}
			system("pause");
			system("cls");
		}
		else if (command == 4)
		{
			int time;
			std::string key;
			std::cout << "请输入需要定期删除的key:" << std::endl;
			//std::cin >> key;
			getline(std::cin, key);
			std::cout << "请输入key的过期时间:" << std::endl;
			std::cin >> time;
			int flag;
			flag = expires(&handler, key,time);
			if (flag == 0)
			{
				std::cout << "设置成功！" << std::endl;
			}
			else if (flag == 1)
			{
				std::cout << "路径不存在！" << std::endl;
			}
			system("pause");
			system("cls");
		}
		else if (command == 5)
		{
			exit(0);
		}
		else
		{
			std::cout << "       Command Error！请重新输入！" << std::endl;
			system("pause");
			system("cls");
		}
		if (handler.getLength() > MAX_LENGTH)
		{
			flag = purge(&handler);
			if (flag == 0)
			{
				std::cout << "文件过大，已整理完毕！" << std::endl;
				handler.getHash();
			}
			else if (flag == 1)
			{
				std::cout << "文件过大，路径不存在无法整理！" << std::endl;
			}
		}

	}
	return 0;
}