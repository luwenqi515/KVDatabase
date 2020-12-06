#pragma once
#include<string>

#define _LOG_WRITE_STATE_ 1            /* 条件编译开关，1：写日志，0：不写日志 */
#define MAX_LOGTEXT_LEN (2048)         /* 每行日志的最大长度*/
#define LOG_TYPE_INFO    0             /* 日志类型: 信息类型*/
#define LOG_TYPE_ERROR   1             /* 日志类型: 错误类型*/



class Log_Data
{
private:
	std::string strDate_Time;
	int iType;               //日志类型
	std::string strText;     
public:
	void setLog_Data(int itype, std::string text);
};
