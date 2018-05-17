#ifndef __USER_DEFINE_H__
#define __USER_DEFINE_H__

#define  WM_MYCOMPRESS_UI_TASK WM_USER + 1001
#define  UM_GSWIN32_UI_TASK    WM_USER + 1002



struct ST_PARSE_RESULT
{
	INT32        nCode;  //0  完成,1解析中  2 完美PDF 不需要处理 3解析失败
	INT32        nVal;   //进度值
	ATL::CString strOutInfo;
};



#endif