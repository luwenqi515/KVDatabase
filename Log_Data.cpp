#pragma warning(disable:4996)
#include "Log_Data.h"
#include<ctime>
#include<fstream>
void Log_Data::setLog_Data(int itype, std::string text)
{
	time_t rawtime;
	time(&rawtime);
	char pblgtime[20];
	strftime(pblgtime, 20, "%Y-%m-%d %X", localtime(&rawtime));
	strDate_Time = pblgtime;
	iType = itype;
	strText = text;
	std::fstream file;
	if (iType == 0 && _LOG_WRITE_STATE_)
	{
		file.open("INFO.txt", std::ios::out | std::ios::app);
		file << strDate_Time << " " <<strText << "\n";
	}
	else if (iType == 1)
	{
		file.open("ERROR.txt", std::ios::out | std::ios::app);
		file << strDate_Time << " " << strText << "\n";
	}
}