#ifndef __UTIL_PATH_H__
#define __UTIL_PATH_H__

#include <atlstr.h>
#include <vector>
using namespace ATL;

namespace  Util
{
	namespace Path
	{
		//获取资源文件夹目录
		ATL::CString GetResFolder();

		//获取程序运行目录
		ATL::CString GetExePath();

		//获取安装目录
		ATL::CString GetInstallFolder();

		//文件是否存在
		BOOL IsFileExist(const ATL::CString strFile);

		//获取文档目录
		ATL::CString GetMyDocPath();

		// 获取数据根目录
		ATL::CString GetDataFolder(BOOL bCreateIfNotExist/* = FALSE*/);
		ATL::CString GetParentDirPath(ATL::CString strFilePath);
		BOOL ValidFilePath(ATL::CString strFilePath);
		BOOL ValidDirPath(CString strDirPath);

		//获取Image
		ATL::CString GetImageTempPath(BOOL bCreateIfNotExist = FALSE);
		ATL::CString GetPDFTempPath(BOOL bCreateIfNotExist = FALSE);

		CString GetPDFOutPath(CString strPdfInPath, CString strPdfOutFolder);
		void StringSplit(CString strSource, CString strSplit, std::vector<CString>& vecSplit);
	}
}

#endif