#include "LRUCache.h"
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
	//将p与前后连接断开
	p->pre->next = p->next;
	p->next->pre = p->pre;
	//将p插入尾节点
	p->pre = tail->pre;
	p->next = tail;
	tail->pre->next = p;
	tail->pre = p;
}

std::string LRUCache::get(std::string key)
{//获取值
	std::map<std::string, ListNode*>::iterator it = m.find(key);
	if (it == m.end())
		return "";
	ListNode* p = it->second;
	//提取p的value后更新p
	update(p);
	return p->value;
}

void LRUCache::put(std::string key, std::string value)
{
	if (max_cnt <= 0)
		return;
	std::map<std::string, ListNode*>::iterator it = m.find(key);//查找key值是否存在
	//先延长链表再判断，如果超出，则删除节点
	if (it == m.end())
	{//如果不存在，则放在双向链表头部，即链表尾
		ListNode* p = new ListNode(key, value);//初始化key和value
		m[key] = p;//建立新的map
		//在尾部插入新节点
		p->pre = tail->pre;
		tail->pre->next = p;
		tail->pre = p;
		p->next = tail;
		cnt++;//计数+1
		if (cnt > max_cnt)
		{//如果计数大于了缓存最大值
			//删除头结点
			ListNode* pDel = head->next;
			head->next = pDel->next;
			pDel->next->pre = head;
			//在链表中删除后，需要在map中也删除掉
			std::map<std::string, ListNode*>::iterator itDel = m.find(pDel->key);
			m.erase(itDel);
			//delete pDel;
			cnt--;
		}
	}
	else
	{//如果存在
		ListNode* p = it->second;//因为map的second存储的是key对应的链表地址，所以将其赋给p
		p->value = value;//计算p内存块中的value值
		update(p);//更新p
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
}