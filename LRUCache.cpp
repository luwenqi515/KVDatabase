#include "LRUCache.h"
#include<iostream>
LRUCache::LRUCache()
{
	head = new ListNode("", "");
	tail = new ListNode("", "");
	head->next = tail;
	tail->pre = head;
	cnt = 0;
}
void LRUCache::update(ListNode* p)
{
	if (p->next == tail)
		return;
	p->pre->next = p->next;
	p->next->pre = p->pre;

	p->pre = tail->pre;
	p->next = tail;
	tail->pre->next = p;
	tail->pre = p;
}

std::string LRUCache::get(std::string key)
{
	std::map<std::string, ListNode*>::iterator it = m.find(key);
	if (it == m.end())
		return "";
	ListNode* p = it->second;
	update(p);
	return p->value;
}

void LRUCache::put(std::string key, std::string value)
{
	if (max_cnt <= 0)
		return;
	std::map<std::string, ListNode*>::iterator it = m.find(key);
	if (it == m.end())
	{
		ListNode* p = new ListNode(key, value);
		m[key] = p;
		p->pre = tail->pre;
		tail->pre->next = p;
		tail->pre = p;
		p->next = tail;
		cnt++;
		if (cnt > max_cnt)
		{
			ListNode* pDel = head->next;
			head->next = pDel->next;
			pDel->next->pre = head;
			std::map<std::string, ListNode*>::iterator itDel = m.find(pDel->key);
			m.erase(itDel);
			cnt--;
		}
	}
	else
	{
		ListNode* p = it->second;
		p->value = value;
		update(p);
	}
}
void LRUCache::del(std::string key)
{
	std::map<std::string, ListNode*>::iterator it = m.find(key);
	if (it != m.end())
	{
		ListNode* p = it->second;
		p->pre->next = p->next;
		p->next->pre = p->pre;
	}
	m.erase(key);
}