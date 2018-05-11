#pragma once
#include <atlstr.h>



namespace Util
{
	namespace MuPdf
	{
		//Æô¶¯PDFÑ¹Ëõ
		UINT32  StartPdfCompress(ATL::CString strPdfOutPath, ATL::CString strPdfPath, ATL::CString strPassword);

		//Í£Ö¹Ñ¹ËõPDF
		void  StopPDFCompress(UINT32 uThreadID);

		//ÍË³ö
		void  DoExit();
	}
}

