#ifndef __UTIL_PATH_H__
#define __UTIL_PATH_H__

#include <atlstr.h>
#include <vector>
using namespace ATL;

namespace  Util
{
	namespace Path
	{
		//��ȡ��Դ�ļ���Ŀ¼
		ATL::CString GetResFolder();

		//��ȡ��������Ŀ¼
		ATL::CString GetExePath();

		//��ȡ��װĿ¼
		ATL::CString GetInstallFolder();

		//�ļ��Ƿ����
		BOOL IsFileExist(const ATL::CString strFile);

		//��ȡ�ĵ�Ŀ¼
		ATL::CString GetMyDocPath();

		// ��ȡ���ݸ�Ŀ¼
		ATL::CString GetDataFolder(BOOL bCreateIfNotExist/* = FALSE*/);
		ATL::CString GetParentDirPath(ATL::CString strFilePath);
		BOOL ValidFilePath(ATL::CString strFilePath);
		BOOL ValidDirPath(CString strDirPath);

		//��ȡImage
		ATL::CString GetImageTempPath(BOOL bCreateIfNotExist = FALSE);
		ATL::CString GetPDFTempPath(BOOL bCreateIfNotExist = FALSE);

		CString GetPDFOutPath(CString strPdfInPath, CString strPdfOutFolder);
		void StringSplit(CString strSource, CString strSplit, std::vector<CString>& vecSplit);
	}
}

#endif