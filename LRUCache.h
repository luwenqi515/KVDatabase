#pragma once
#include<string>
#include<map>
#include<algorithm>
struct ListNode
{
	std::string key;
	std::string value;
	ListNode* pre;
	ListNode* next;
	ListNode(std::string _key, std::string _value)
	{
		key = _key;
		value = _value;
		pre = NULL;
		next = NULL;
	}
};
class LRUCache
{
private:
	int max_cnt=3;//��󻺴�����
	int cnt;//�������
	std::map<std::string, ListNode*> m;//��¼���ݺ��������ַ
	ListNode* head;//����ͷ
	ListNode* tail;//����β
public:
	LRUCache();
	void update(ListNode* p);
	std::string get(std::string key);
	void put(std::string key, std::string value);
	void del(std::string key);
};

