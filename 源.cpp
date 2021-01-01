#include<iostream>
#include<string>
#include"KVDBHandler.h"

const long MAX_LENGTH = 1000000;

int main()
{
	std::string file;
	std::cout << "������Ҫ�򿪵����ݿ⣺";
	std::cin >> file;
	int flag;
	KVDBHandler handler((const std::string)file,flag);
	std::cout << "���ڴ����ݿ�..." << std::endl;
	if(flag==0)
		std::cout << "���ݿⲻ����,�Ѵ����µ����ݿ⣡" << std::endl;
	else
		std::cout << "���ݿ�򿪳ɹ���" << std::endl;
	int command;
	while (true)
	{
		std::cout << "�������������:\n";
		std::cout << "             [1]д��" << std::endl;
		std::cout << "             [2]��ȡ" << std::endl;
		std::cout << "             [3]ɾ��" << std::endl;
		std::cout << "             [4]��ʱɾ��" << std::endl;
		std::cout << "             [5]�˳�" << std::endl;
		std::cin >> command;
		getchar();
		if (command == 1)
		{
			std::string key, value;
			std::cout << "������key��value��" << std::endl;
			getline(std::cin, key);
			getline(std::cin, value);
			flag = set(&handler, key, value,0);
			if (flag == 0)
				std::cout << "д��ɹ���" << std::endl;
			else if (flag == 1)
			{
				std::cout << "·�������ڣ�" << std::endl;
			}
			else if (flag == 2)
			{
				std::cout << "KEY��Ч�������²�����" << std::endl;
			}
			else if (flag == 4)
			{
				std::cout << "�����������޷�д�룡" << std::endl;
			}
			system("pause");
			system("cls");
		}
		else if (command == 2)
		{
			std::string key, value;
			std::cout << "������key:" << std::endl;
			getline(std::cin, key);
			int flag;
			flag = get(&handler, key, value);
			if (flag == 0)
			{
				std::cout << "��ȡ�ɹ���" << std::endl;
				std::cout << key << "��Ӧ��valueΪ��" << value << std::endl;
			}
			else if (flag == 1)
			{
				std::cout << "·�������ڣ�" << std::endl;
			}
			else if (flag == 2)
			{
				std::cout << "KEY��Ч�������²�����" << std::endl;
			}
			else if (flag == 5)
			{
				std::cout << "δ��ȡ��key��ص�value��" << std::endl;
			}
			system("pause");
			system("cls");
		}
		else if (command == 3)
		{
			std::string key;
			std::cout << "��������Ҫɾ����key:" << std::endl;
			getline(std::cin, key);
			int flag;
			flag = del(&handler, key);
			if (flag == 0)
			{
				std::cout << "ɾ���ɹ���" << std::endl;
			}
			else if (flag == 1)
			{
				std::cout << "·�������ڣ�" << std::endl;
			}
			else if (flag == 2)
			{
				std::cout << "KEY��Ч�������²�����" << std::endl;

			}
			system("pause");
			system("cls");
		}
		else if (command == 4)
		{
			int time;
			std::string key;
			std::cout << "��������Ҫ����ɾ����key:" << std::endl;
			getline(std::cin, key);
			std::cout << "������key�Ĺ���ʱ��:" << std::endl;
			std::cin >> time;
			int flag;
			flag = expires(&handler, key,time);
			if (flag == 0)
			{
				std::cout << "���óɹ���" << std::endl;
			}
			else if (flag == 1)
			{
				std::cout << "·�������ڣ�" << std::endl;
			}
			system("pause");
			system("cls");
		}
		else if (command == 5)
		{
			//exit(0);
			purge(&handler);
		}
		else
		{
			std::cout << "       Command Error�����������룡" << std::endl;
			system("pause");
			system("cls");
		}
		if (handler.getLength() > MAX_LENGTH)
		{
			flag = purge(&handler);
			if (flag == 0)
			{
				std::cout << "�ļ�������������ϣ�" << std::endl;
				handler.getHash_q();
			}
			else if (flag == 1)
			{
				std::cout << "�ļ�����·���������޷�����" << std::endl;
			}
		}
	}
	return 0;
}