#include "UtilMuPdf.h"
#include <string>
#include "Singleton.h"
#include "PdfCompress.h"


UINT32  Util::MuPdf::StartPdfCompress(ATL::CString strPdfOutPath, ATL::CString strPdfPath, ATL::CString strPassword)
{
	return Singleton<CPdfCompress>::Instance().StartThread(strPdfOutPath, strPdfPath, strPassword);
}


