#pragma once
#include <atlstr.h>
using namespace ATL;

namespace  Util
{
	namespace Path
	{
		//��ȡ��Դ�ļ���Ŀ¼
		ATL::CString GetResFolder();

		//��ȡ�ĵ�Ŀ¼
		ATL::CString GetMyDocPath();

		// ��ȡ���ݸ�Ŀ¼
		ATL::CString GetDataFolder(BOOL bCreateIfNotExist/* = FALSE*/);
		ATL::CString GetParentDirPath(ATL::CString strFilePath);
		BOOL ValidFilePath(ATL::CString strFilePath);
		BOOL ValidDirPath(CString strDirPath);

		//��ȡImage
		ATL::CString GetImageTempPath(BOOL bCreateIfNotExist = FALSE);
	}
}
