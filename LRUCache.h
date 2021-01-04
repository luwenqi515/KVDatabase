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
	int max_cnt=8;
	int cnt;
	std::map<std::string, ListNode*> m;//记录数据和其链表地址
	ListNode* head;
	ListNode* tail;
public:
	LRUCache();
	void update(ListNode* p);
	std::string get(std::string key);
	void put(std::string key, std::string value);
	void del(std::string key);
};

