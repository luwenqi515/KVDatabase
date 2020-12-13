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
	//��p��ǰ�����ӶϿ�
	p->pre->next = p->next;
	p->next->pre = p->pre;
	//��p����β�ڵ�
	p->pre = tail->pre;
	p->next = tail;
	tail->pre->next = p;
	tail->pre = p;
}

std::string LRUCache::get(std::string key)
{//��ȡֵ
	std::map<std::string, ListNode*>::iterator it = m.find(key);
	if (it == m.end())
		return "";
	ListNode* p = it->second;
	//��ȡp��value�����p
	update(p);
	return p->value;
}

void LRUCache::put(std::string key, std::string value)
{
	if (max_cnt <= 0)
		return;
	std::map<std::string, ListNode*>::iterator it = m.find(key);//����keyֵ�Ƿ����
	//���ӳ��������жϣ������������ɾ���ڵ�
	if (it == m.end())
	{//��������ڣ������˫������ͷ����������β
		ListNode* p = new ListNode(key, value);//��ʼ��key��value
		m[key] = p;//�����µ�map
		//��β�������½ڵ�
		p->pre = tail->pre;
		tail->pre->next = p;
		tail->pre = p;
		p->next = tail;
		cnt++;//����+1
		if (cnt > max_cnt)
		{//������������˻������ֵ
			//ɾ��ͷ���
			ListNode* pDel = head->next;
			head->next = pDel->next;
			pDel->next->pre = head;
			//��������ɾ������Ҫ��map��Ҳɾ����
			std::map<std::string, ListNode*>::iterator itDel = m.find(pDel->key);
			m.erase(itDel);
			//delete pDel;
			cnt--;
		}
	}
	else
	{//�������
		ListNode* p = it->second;//��Ϊmap��second�洢����key��Ӧ�������ַ�����Խ��丳��p
		p->value = value;//����p�ڴ���е�valueֵ
		update(p);//����p
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