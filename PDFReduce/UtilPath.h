#pragma once
#include <atlstr.h>
using namespace ATL;

namespace  Util
{
	namespace Path
	{
		//获取资源文件夹目录
		ATL::CString GetResFolder();

		//获取文档目录
		ATL::CString GetMyDocPath();

		// 获取数据根目录
		ATL::CString GetDataFolder(BOOL bCreateIfNotExist/* = FALSE*/);
		ATL::CString GetParentDirPath(ATL::CString strFilePath);
		BOOL ValidFilePath(ATL::CString strFilePath);
		BOOL ValidDirPath(CString strDirPath);

		//获取Image
		ATL::CString GetImageTempPath(BOOL bCreateIfNotExist = FALSE);
	}
}
