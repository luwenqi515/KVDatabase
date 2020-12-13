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
	int max_cnt=3;//最大缓存数量
	int cnt;//缓存计数
	std::map<std::string, ListNode*> m;//记录数据和其链表地址
	ListNode* head;//链表头
	ListNode* tail;//链表尾
public:
	LRUCache();
	void update(ListNode* p);
	std::string get(std::string key);
	void put(std::string key, std::string value);
	void del(std::string key);
};

