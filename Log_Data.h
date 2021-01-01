#pragma once
#include<string>

#define _LOG_WRITE_STATE_ 1            /* �������뿪�أ�1��д��־��0����д��־ */
#define MAX_LOGTEXT_LEN (2048)         /* ÿ����־����󳤶�*/
#define LOG_TYPE_INFO    0             /* ��־����: ��Ϣ����*/
#define LOG_TYPE_ERROR   1             /* ��־����: ��������*/



class Log_Data
{
private:
	std::string strDate_Time;
	int iType;               //��־����
	std::string strText;     
public:
	void setLog_Data(int itype, std::string text);
};