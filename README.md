#                  基于文件的k-v数据库

[完整代码](https://github.com/welch0515/KVDatabase)                                                                                                                                                                                                                                                                                    **报告人：鲁文奇     学号：2019151095**

## 1.需求介绍

### （1）k-v数据库简介

​        在非关系型数据库中，数据之间无关系，这样就非常容易扩展。无形之间也在架构的层面上带来了可扩展的能力。在面对大数据量时，由于非关系型数据库的无关系性，其具有非常高的读写性能，并且数据库的结构简单。本项目就是 一个基于文件的二进制K-V 数据的非关系型数据库，能实现将k-v数据写入（set）、根据key查找value(get)、根据key删除value(del)、清理多余k-v数据(purge)、设置key数据过期时间(expires)的操作。

### （2）API介绍

#### ·打开关闭数据库

​       在第一次打开数据库时，判断文件是否存在。若文件存在，则打开数据库；否则，创建新的数据库。之后再建立索引和保存过期时间的最小堆。在之后的set,del,purge,expire操作中会重新打开文件，操作完毕后再关闭。

```c++
KVDBHandler(const std::string& db_file,int &flag)//flag指示文件是否存在
{
	filename = db_file;
	std::fstream file;
	file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!file)
	{
		flag = 0;
		std::ofstream newfile(filename);
		newfile.close();
	}
	else
	{
		flag = 1;
	}
	file.close();
	getIndex_q();

}

~KVDBHandler()
{
	filename.clear();
}
```

​    

#### ·写入数据

​      用户输入key和value,再根据文件名打开文件，用write()函数依次将klen(key的长度)、vlen(value的长度)、key、value、ktime(0)追加写入文件末尾，并在索引中查找是否存在，若不存在就添加入索引，否则修改索引值，并更新在LRU链表中的位置。

```c++
int set(KVDBHandler* handler, const std::string& key, const std::string& value,int ktime)
{
	std::fstream file;
	file.open(handler->filename, std::ios::out | std::ios::binary | std::ios::app);
	if (!file)
	{
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
			return KVDB_INVALID_KEY;
		}
		else if (!Freespace())
		{
			file.close();
			return KVDB_NO_SPACE_LEFT_ON_DEVICES;
		}
		else
		{
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
			if (handler->index.Find(key))
			{
				handler->index.SetOffset(key, offset);
			}
			else
			{
				handler->index.AddItem(key, offset,ktime);
			}
			return KVDB_OK;
		}
	}
}
```

  

#### ·读取数据

​       用户输入要查找的key，再根据文件名打开文件。依据LRU缓存机制，直接在LRU记录的几个最近操作的k-v数据查找，若未查找到就再由索引查找。根据key在索引中查找偏移值，用seekg()函数直接移动到文件相应的位置读取，返回 Value，或返回空。

```c++
int get(KVDBHandler* handler, const std::string& key, std::string& value)
{
	std::fstream file;
	file.open(handler->filename, std::ios::in | std::ios::binary);
	if (!file)
	{
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
			int ktime = handler->index.GetKtime(handler->q.top().key);
			if (ktime == handler->q.top().time)
			{
				del(handler, handler->q.top().key);
			}
			handler->q.pop();
		}
		if (key.length() == 0)
		{
			file.close();
			return KVDB_INVALID_KEY;
		}
		else
		{
			value=handler->LRU.get(key);
			if (value.empty())
			{
				if (handler->index.Find(key))
				{
					int offset = handler->index.GetOffset(key);
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
					return KVDB_OK;
				}
				else
				{
					file.close();
					return KVDB_NO_FIND_VALUE;
				}
			}
			else
			{
				//LRU中找到
				return KVDB_OK;
			}
		}
	}
}
```

 

#### ·删除数据

​      用户输入要删除的key,再根据文件名打开文件。用write()函数在文件末尾写入klen（key的长度）、vlen=-1(value的长度)、key，再删除在索引和LRU中的值。

```c++
int del(KVDBHandler* handler, const std::string& key)
{
	std::fstream file;
	file.open(handler->filename, std::ios::out | std::ios::binary | std::ios::app);
	if (!file)
	{
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
			return KVDB_INVALID_KEY;
		}
		else
		{
			file.write((char*)&klen, sizeof(int));
			file.write((char*)&vlen, sizeof(int));
			file.write(key.c_str(), klen);
			file.close();
			handler->index.RemoveItem(key);
            handler->LRU.del(key);
			return KVDB_OK;
		}
	}
}
```



#### ·文件整理

​        当文件大小到达设定值之后，进行操作。根据已经建立的索引，在备份文件中遍历索引写入k-v数据，再将原文件清空，将备份文件内容写回原文件，最后清空备份文件。

```c++
int purge(KVDBHandler* handler)
{
	std::fstream file;
	file.open(handler->filename, std::ios::in | std::ios::binary);
	if (!file)
	{
		return KVDB_INVALID_AOF_PATH;
	}
	else
	{
		int flag;
		std::string savefilename = handler->filename + "save",text;
		KVDBHandler savehandler(savefilename,flag);
		purge_TraverseIndex(handler, &savehandler);
		purge_TraverseIndex(&savehandler, handler);
		handler->getIndex_q();
		return KVDB_OK;
	}
}
void purge_TraverseIndex(KVDBHandler *handler,KVDBHandler *savehandler)
{
	for (int i = 0; i < handler->index.tableSize; i++)
	{
		item* p = handler->index.HashTable[i];
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
	handler->index.clear();
}
```



#### ·过期删除

​      用户输入要设置的key及其生存周期（秒），由相对时间加上当前时间计算出绝对时间后，连同k-v数据写入文件，并将key连同过期时间加入最小堆，最后修改在索引中的值。在每次get操作前，一直比较最小堆顶的时间和当前时间，若已经过期就用del函数删除，直到堆顶的时间大于当前时间，再进行上述的get操作。

```c++
int expires(KVDBHandler* handler, const std::string key, int n)
{
	std::fstream file;
	file.open(handler->filename, std::ios::out | std::ios::app);
	if (!file)
	{
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

		handler->index.SetKtime(k.key, k.time);
		set(handler, key, value, k.time);
		handler->q.push(k);
		file.close();
		return KVDB_OK;
	}
}
```

get操作中将最小堆中的时间与索引中的时间、当前时间比较，删除过期数据：

```c++
while (!handler->q.empty() && handler->q.top().time!=0 && handler->q.top().time < current_time)
		{
			int ktime = handler->index.GetKtime(handler->q.top().key);
			if (ktime == handler->q.top().time)
			{
				del(handler, handler->q.top().key);
			}
			handler->q.pop();
		}
```



### （3）类对象介绍

- **k-v数据库**

  ```c++
  class KVDBHandler
  {
  private:
  	std::string filename;
  	Hash index;
  	std::priority_queue<KeyTime> q;
  	LRUCache LRU;
  public:
  	KVDBHandler(const std::string& db_file,int &flag);
  	~KVDBHandler();
  	long getLength();//获取文件大小
  	void getIndex_q();
  	friend int set(KVDBHandler* handler, const std::string& key, const std::string& value,int ktime);
  	friend int get(KVDBHandler* handler, const std::string& key, std::string& value);
  	friend int del(KVDBHandler* handler, const std::string& key);
  	friend int purge(KVDBHandler* handler);
  	friend int expires(KVDBHandler* handler, const std::string key, int n);
  	friend void purge_TraverseIndex(KVDBHandler* handler, KVDBHandler* savehandler);
  };
  ```

  

- **哈希表**

  ```C++
  struct item
  {
  	std::string key;
  	int offset;
  	int ktime;
  	item* next;
  	item() :key("empty"),offset(0), ktime(0),next(NULL) {}
  	item(std::string k, int o,int t) : key(k), offset(o), ktime(t),next(NULL) {}
  };
  class Hash
  {
  public:
  	static const int tableSize = 11;
  	item* HashTable[tableSize];
  	Hash();
  	~Hash();
  	void clear();
  	int hashFunction(std::string key);
  	void AddItem(std::string key, int offset,int ktime);
  	int Find(std::string key);
  	int GetOffset(std::string key);
  	int GetKtime(std::string key);
  	void SetKtime(std::string key, int ktime);
  	void SetOffset(std::string key, int offset);
  	void RemoveItem(std::string key);
  };
  
  
  ```

  

- **LRU缓存**

  ```c++
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
  	int max_cnt=3;
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
  
  
  ```

  ### ·程序流程图

  ![流程图](https://github.com/welch0515/KVDatabase/blob/main/%E6%B5%81%E7%A8%8B%E5%9B%BE.png)

  

  ## 2.设计

  ### （1）基于文件存储

  ​      由于内存空间有限，无法大量存储数据，而且当程序运行过程中断时，存储在内存中的数据可能会丢失。而传统磁盘的顺序读、写性能远超过随机读、写，也不需要管理⽂件“空洞”。因此选择将数据存储到文件中，打开数据库时遍历文件在内存建立索引，写入或删除数据时追写到文件末尾，读取数据时根据索引值在文件中读取数据。

  

  ### （2）索引

  ​     由于读取数据需要遍历整个文件效率较低，因此增加⼀个索引来保存当前数据库中每⼀个 Key在文件中的位置。遍历文件中的 K-V Record，在索引中保存读取的 Key 及 Record 的位置。

  - 读取(get)操作时只需要访问索引，若 Key 在索引中，则从索引指向的文件中对应的 K-V Record 中读 取数据；若索引中 Key 不存在，则直接返回结果 
  - 写⼊(set)操作中，先将 K-V Record 写⼊文件中，再修改 Index ， 若 Key 不存在，则将它添加到索引中；若 Key 之前已存在于索引中，则修改索引指 向的位置
  - 删除(del)操作中，先将表示删除操作的特殊的 KV Record 写⼊ 文件 中，再将索引中的 Key 删除
  - 清理文件(purge)操作后，需要重新执行建立索引操作

  以下为建立索引操作：

  ```c++
  void getIndex_q()
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
  			if (index.Find(str_key))
  			{
  				index.SetOffset(str_key, offset);
  			}
  			else
  			{
  				index.AddItem(str_key, offset,ktime);
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
  			index.RemoveItem(str_key);
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
  }
  ```

  ### （3）LRU缓存机制

  ​       由于数据库中存在一些读取率较高的数据，磁盘操作⽐内存要慢，因此添加LRU缓存机制，短暂存储短期内操作的数据。通过哈希表结合双向链表来实现，哈希表中保存key值和对应的节点地址，这样可以快速找到节点，避免遍历链表。当进行更新操作时，将相应节点放置链表尾结点的前一个。

  

  ### （4）purge操作

  ​        当 set/get/del 操作反复执行后，⽂件体积会越来越⼤，但其中有效数据可能很少。因此增加 purge() 操作，整理文件： 合并多次写入的 Key，只保留最终有效的⼀项 。合并过程中，移除被删除的 Key 。设置⼀个⽂件⼤⼩上限的阈值，当⽂件⼤⼩达到阈值时，⾃动触发 purge() 操作用于处理文件膨胀问题。

  

  ### （5）异常处理

  ​       程序运行过程中可能会出现一些问题，例如：文件路径不存在、数据类型不符、磁盘大小不足等。通过异常情况下返回特定的常量，向用户反馈出现的问题。

  ```c++
  const int KVDB_OK = 0;                        //成功
  const int KVDB_INVALID_AOF_PATH = 1;          //路径不存在
  const int KVDB_INVALID_KEY = 2;               //key值无效
  const int KVDB_INVALID_VALUE = 3;             //value值无效
  const int KVDB_NO_SPACE_LEFT_ON_DEVICES = 4;  //磁盘空间不足
  const int KVDB_NO_FIND_VALUE = 5;             //未找到value的值
  ```

  

  ## 3.调试

  ### （1）分支测试

  - **set/get/del分支测试**

    - 多次重复对一个key进行set和del操作，比较get到的value是否符合

    - 写入大量不同的k-v数据，随机读取一个

    - 读取数据库中不存在的key

      

  - **purge分支测试**

    - 文件大小达到阈值时是否会自动触发purge（）操作

    - 多次重复对一个key进行set和del操作，输出文件内容，比较是否只保留了有效数据

      

  - **expires分支测试**

    - 重复对一个key设置不同的过期时间，比较是否是最后一个过期时间生效
    - 对一个key设置过期时间后，将这个key对应的k-v数据删除，再重新写入这个key，观察当前的key是否会被过期删除

    

  - **LRU分支测试**

    - 更新操作是否有效

    - 是否能从中由key读取到value

    - 当达到最大缓存数时，是否会自动删除

      

  ###   (2)日志

  ​       在程序每进行一次set/get/del/purge/expires操作后打出日志，记录操作的时间和内容，当程序出现问题时可查看日志找到具体哪一步出现问题。具体日志类如下：

  ```c++
  #define _LOG_WRITE_STATE_ 1            /* 条件编译开关，1：写日志，0：不写日志 */
  #define MAX_LOGTEXT_LEN (2048)         /* 每行日志的最大长度*/
  #define LOG_TYPE_INFO    0             /* 日志类型: 信息类型*/
  #define LOG_TYPE_ERROR   1             /* 日志类型: 错误类型*/
  #define LOG_TYPE_WARNING 2             /* 日志类型: 警告类型*/
  #define LOG_TYPE_DEBUG   3             /* 日志类型: 调试类型*/
  #define LOG_TYPE_FATAL   4             /* 日志类型: 严重错误类型*/
  
  class Log_Data
  {
  private:
  	std::string strDate_Time;
  	int iType;               //日志类型
  	std::string strText;     
  public:
  	void setLog_Data(int itype, std::string text);
  };
  ```

  

  ## 4.代码优化

  ###   (1) get操作的优化

  经过以下3个阶段的优化：

  **Stage1：**直接将文件用read函数顺序遍历，若有匹配的key就读取value，否则用seekg（）函数跳过，直到最后文件读完。获取满⾜查询条件的最后⼀个 Key，并返回其 Value，或返回空。时间复杂度O（n），每次查找需要遍历整个文件。

  **Stage2：**依据哈希表建立索引，根据key在索引中查找偏移值，用seekg（）函数直接移动到文件相应的位置读取，返回其 Value，或返回空。

  **Stage3：**依据LRU缓存机制，直接在LRU记录的几个最近操作的k-v数据查找，若未查找到就再由索引查找。这样可进一步减小时间复杂度，但需要更大的内存空间。

  

  ### （2）purge操作的优化

  **Stage1：**将文件顺序遍历，把读取到的k-v数据写入map容器中，每次写入前进行判断：若key已存在，就修改对应的value值；若key不存在，就写入map容器中；若vlen =-1，就删除map容器中的k-v数据。再把原文件清空，将map容器中的k-v数据写入原文件。这样时间复杂度较高，也占用较大内存空间。

  **Stage2：**利用已经建立好的索引来清理文件。先遍历索引，将k-v数据写入备份文件并把原文件清空，再把备份文件内容写入原文件并清空备份文件。这样可减小时间复杂度和空间复杂度。

  

  ### （3）过期删除操作的优化

  **Stage1：**在每次设置key的过期时间时，在小顶堆中查找key是否存在，若已经存在就重新建立小顶堆，否则直接写入小顶堆中。因为重新建立小顶堆过程复杂，时间复杂度较大。

  **Stage2：**在k-v Record数据后加入key的过期时间，每次设置key的过期时间时直接入小顶堆，在get操作中每次取出小顶堆堆顶时间时，与索引中的时间比较，只有一致时才删除对应的k-v数据。

  

  ## 5.问题与解决

  - **如何读写文件？**

    ```c++
    fstream(头文件#include<fstream>)
    表示文件读取/写入流，对文件进行读和写操作，既可以将数据从存储设备读取到内存中，也可以将数据从内存写入存储设备中
    read(char *buffer,streamsize size);
    write(chra *buffer,streamsize size);
    ```

  

  - **write()函数无法将整个k-v数据对象一次直接写入**

    ```c++
    KvRecord a;
    Write(a,sizeof(a));
    因为这里的a中的字符串key和value只是一个地址值，不是key和value实际值。需要将k-v数据中的内容分别依次写入。
    ```

  

  - **如何清空文件？**

    ```c++
    文件写入流设置ios_base::out打开方式
    ofstream file(filename, std::ios_base::out);
    ```

    

  - **如何代码调试？**

    （1）在程序每进行一次操作后打出日志，记录操作的时间和内容，当程序出现问题时可查看日志找到具体哪一步出现问题。

    （2）将程序分成不同分支调试。因为当程序代码量过大时，总体结合起来调试效率较低。

    （3）设置异常返回值，通过返回的常量判断运行结果，当函数出现问题时返回相应的常项，这样可清晰反馈可能的错误操作。

    

  - **日志分级**

    日志分为以下5个等级：

    debug：所有详细信息，用于调试。

    info:一些关键跳转，证明软件正常运行的日志。

    warning:表明发生了一些意外，软件无法处理，但是依然能正常运行。

    error：由于一些严重问题，软件不能正常执行一些功能，但是依然能运行。

    critical/fatal：非常严重的错误，软件已经不能继续运行了。

  

  - **工程化编程**

    所有代码不应写在一个.cpp文件中，应按照类对象分割放置在不同的.h和.cpp文件中。

  

  - **GitHub的作用**

    GitHub是一个基于git的代码分享平台，可以在上面创建仓库存放代码，有助于项目的共同开发。

    

  - **过期操作遇到重复对同一key设置不同的过期时间如何只使后一个时间生效？**

    因为无法对小顶堆中的数据进行修改，所以在k-v数据对中添加key的过期时间，并记录到索引中。在每一次取出堆顶元素是与索 引中的过期时间进行比较，如果符合再进行删除操作，这样可以避免对同一key的重复设置过期时间，以及key被删除后过期时间仍然生效的问题。

    

  - **什么是小顶堆？如何实现小顶堆？**

    小顶堆要求根节点的关键字既小于或等于左子树的关键字值，又小于或等于右子树的关键字值，可用STL中的优先队列实现。STL 的优先队列定义在头文件和 （队列一样），用"priority_queue q"来声明，默认情况下为大顶堆，需要自定义优先级。

  

  - **如何提高代码复用性？**

    对重复率高的代码进行包装成函数，这样可以减小程序出错的可能。

  

  - **如何实现LRU缓存机制？**

    使用哈希表结合双向链表来实现。哈希表中保存key值和对应的节点地址，这样可以快速找到节点，避免遍历链表。当进行更新操作时，将相应节点放置链表尾结点的前一个。

    
