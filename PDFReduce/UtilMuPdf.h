#pragma once
#include <atlstr.h>



namespace Util
{
	namespace MuPdf
	{
		//����PDFѹ��
		UINT32  StartPdfCompress(ATL::CString strPdfOutPath, ATL::CString strPdfPath, ATL::CString strPassword);

		//ֹͣѹ��PDF
		void  StopPDFCompress(UINT32 uThreadID);

		//�˳�
		void  DoExit();
	}
}

