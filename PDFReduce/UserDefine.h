#ifndef __USER_DEFINE_H__
#define __USER_DEFINE_H__

#define  WM_MYCOMPRESS_UI_TASK WM_USER + 1001
#define  UM_GSWIN32_UI_TASK    WM_USER + 1002



struct ST_PARSE_RESULT
{
	INT32        nCode;  //0  ���,1������  2 ����PDF ����Ҫ���� 3����ʧ��
	INT32        nVal;   //����ֵ
	ATL::CString strOutInfo;
};



#endif