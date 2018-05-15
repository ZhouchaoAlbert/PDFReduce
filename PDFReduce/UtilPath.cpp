#include "UtilPath.h"
#include <ShlObj.h>
#include <ShellAPI.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <atlstr.h>

ATL::CString Util::Path::GetResFolder()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	ATL::CString strPath(szPath);
	INT32 iFind = strPath.ReverseFind(_T('\\'));
	if (iFind < 0)
	{
		return _T("");
	}
	strPath = strPath.Left(iFind);

	INT32 ix = strPath.ReverseFind(_T('\\'));
	if (ix != -1)
	{
		return strPath.Mid(0, ix) + _T("\\Resource");
	}
	return _T("");
}

ATL::CString Util::Path::GetExePath()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	ATL::CString strPath(szPath);
	INT32 iFind = strPath.ReverseFind(_T('\\'));
	if (iFind < 0)
	{
		ATLASSERT(FALSE);
		return _T("");
	}
	strPath = strPath.Left(iFind);
	return strPath;
}

ATL::CString Util::Path::GetInstallFolder()
{
	ATL::CString strPath = GetExePath();
	ATLASSERT(!strPath.IsEmpty());
	INT32 iFind = strPath.ReverseFind(_T('\\'));
	if (iFind < 0)
	{
		ATLASSERT(FALSE);
		return _T("");
	}
	strPath = strPath.Mid(0, iFind);
	return strPath;
}


BOOL Util::Path::IsFileExist(const ATL::CString strFile)
{
	DWORD dwAttrib = GetFileAttributes(strFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//获取我的文档目录
ATL::CString Util::Path::GetMyDocPath()
{
	HKEY	hKey = NULL;
	DWORD	dwType = REG_SZ;
	TCHAR	szPath[MAX_PATH * 2] = { 0 };
	DWORD	dwData = _countof(szPath) - 1;
	TCHAR	szDocuments[MAX_PATH * 2] = { 0 };

	::SHGetSpecialFolderPath(NULL, szDocuments, CSIDL_PERSONAL, FALSE);
	if (_tcsstr(szDocuments, _T("systemprofile")) == NULL)
		return szDocuments;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), 0, KEY_READ, &hKey))
		return szDocuments;

	if (ERROR_SUCCESS == RegQueryValueEx(hKey, _T("Personal"), NULL, &dwType, (LPBYTE)szPath, &dwData))
		_tcscpy_s(szDocuments, dwData, szPath);

	RegCloseKey(hKey);
	return szDocuments;
}

// 获取数据根目录
ATL::CString Util::Path::GetDataFolder(BOOL bCreateIfNotExist/* = FALSE*/)
{
	TCHAR szDocuments[MAX_PATH] = { 0 };
	ATL::CString strDocuments = GetMyDocPath();
	ATL::CString strDir;
	strDir.Format(_T("%s\\YueShuPDFTool"), strDocuments);

	if (FALSE != bCreateIfNotExist)
	{
		ValidDirPath(strDir);
	}

	return strDir;
}


ATL::CString Util::Path::GetParentDirPath(ATL::CString strFilePath)
{
	int ix = strFilePath.ReverseFind(_T('\\'));
	if (-1 != ix)
	{
		return strFilePath.Mid(0, ix);
	}
	return _T("");
}

BOOL Util::Path::ValidFilePath(CString strFilePath)
{
	ATL::CString strDirPath = GetParentDirPath(strFilePath);

	return ValidDirPath(strDirPath);
}


BOOL Util::Path::ValidDirPath(ATL::CString strDirPath)
{
	ATL::CString strParentPath = GetParentDirPath(strDirPath);

	if (strParentPath.IsEmpty())
	{
		return FALSE;
	}
	if (!PathFileExists(strParentPath))
	{
		BOOL bResult = ValidDirPath(strParentPath);
		if (FALSE == bResult)
		{
			return FALSE;
		}
	}

	if (!PathFileExists(strDirPath))
	{
		BOOL bResult = CreateDirectory(strDirPath, NULL);
		if (FALSE == bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}

ATL::CString Util::Path::GetImageTempPath(BOOL bCreateIfNotExist/* = FALSE*/)
{
	ATL::CString strDir = GetDataFolder(TRUE) + _T("\\TempImage");
	if (bCreateIfNotExist != FALSE)
	{
		if (!ValidDirPath(strDir))
		{
			ATLASSERT(FALSE);
			return _T("");
		}
	}
	return strDir;
}

ATL::CString Util::Path::GetPDFTempPath(BOOL bCreateIfNotExist/* = FALSE*/)
{
	ATL::CString strDir = GetDataFolder(TRUE) + _T("\\TempPDF");
	if (bCreateIfNotExist != FALSE)
	{
		if (!ValidDirPath(strDir))
		{
			ATLASSERT(FALSE);
			return _T("");
		}
	}
	return strDir;
}

void Util::Path::StringSplit(CString strSource, CString strSplit, std::vector<CString>& vecSplit)
{
	vecSplit.clear();
	if (strSource.IsEmpty())
		return;
	if (strSplit.IsEmpty())
		vecSplit.push_back(strSource);
	else
	{
		CString strT = L"";
		int nIndex = 0;
		while (!strSource.IsEmpty())
		{
			nIndex = strSource.Find(strSplit);
			if (nIndex >= 0)
			{
				strT = strSource.Left(nIndex);
				strSource.Delete(0, strT.GetLength() + strSplit.GetLength());
			}
			else
			{
				strT = strSource;
				strSource = L"";
			}
			vecSplit.push_back(strT);
		}
	}
}
