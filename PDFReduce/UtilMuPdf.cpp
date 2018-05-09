#include "UtilMuPdf.h"
#include <string>
#include "UtilImage.h"
#include <sstream>  
#include "zlib.h"
#include <io.h>
#include <fstream>

//��ʼ������
BOOL Util::MuPdf::Init(ATL::CString strPdfPath, ATL::CString strPassword)
{
	if (strPdfPath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	ATL::CW2A szPdfPath(strPdfPath.GetString(), CP_ACP);
	ATL::CW2A szPassword(strPassword.GetString(), CP_ACP);
    //���������� ����ȫ����Ϣ
 	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
 	if (!ctx)
 	{
		ATLASSERT(FALSE);
		return FALSE;
 	}
	//���ĵ�
	doc = pdf_open_document_no_run(ctx, szPdfPath.m_psz);
	if (!doc)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//PDF�Ƿ���Ҫ���뼰�����Ȩ
	if (pdf_needs_password(doc) && !pdf_authenticate_password(doc, szPassword.m_psz))
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	//Test
	PraseResImage("E:\\test\\convert");
	return TRUE;
}

//��ȡPDFҳ
UINT32 Util::MuPdf::GetPdfPages(pdf_document* doc)
{
	return pdf_count_pages(doc);
}

//�Ƿ���ͼƬ
BOOL Util::MuPdf::IsImage(pdf_obj* obj)
{
	pdf_obj *type = pdf_dict_gets(obj, "Subtype");
	return pdf_is_name(type) && !strcmp(pdf_to_name(type), "Image");
}

BOOL  Util::MuPdf::WritePixmap(fz_context *ctx, fz_pixmap *pix, std::string filepath, int rgb)
{
	BOOL bRet = FALSE;

	fz_pixmap *converted = NULL;
	if (!pix)
	{
		return bRet;
	}
	if (rgb && pix->colorspace && pix->colorspace != fz_device_rgb(ctx))
	{
		fz_irect bbox;
		converted = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_pixmap_bbox(ctx, pix, &bbox));
		fz_convert_pixmap(ctx, converted, pix);
		pix = converted;
	}
	if (pix->n <= 4)
	{
		fz_write_png(ctx, pix, (char*)filepath.c_str(), 0);
		bRet = TRUE;
	}
	else
	{
		ATLASSERT(FALSE);
	}
	fz_drop_pixmap(ctx, converted);
	return bRet;
}

BOOL Util::MuPdf::SaveImage(std::string szSavePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (szSavePath.empty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}
	fz_image   *image;
	fz_pixmap  *pix;
	pdf_obj    *ref;


	ref = pdf_new_indirect(doc, nNum, 0);

	//����ͼƬ���ļ�
	image = pdf_load_image(doc, ref);
	pix = fz_new_pixmap_from_image(ctx, image, 0, 0);
	fz_drop_image(ctx, image);

	//rgb mode
	bRet = WritePixmap(ctx, pix, szSavePath, 1);

	fz_drop_pixmap(ctx, pix);
	pdf_drop_obj(ref);

	return bRet;
}

void StrToHex(const char*pbSrc, int nLen)
{
	CString strTest;
	char h1, h2;
	BYTE s1, s2;
	int i;
	for (i = 0; i < nLen; i++)
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;
		strTest.AppendFormat(_T("%0xd"), s1 * 16 + s2);
	}
}

//������Դ��Ϣ
void Util::MuPdf::PraseResImage(std::string szSavePath)
{
	UINT32 uObjNums = pdf_count_objects(doc);
	for (UINT32 uIndex = 1; uIndex < uObjNums; uIndex++)
	{
		pdf_obj  *obj;
		obj = pdf_load_object(doc, uIndex, 0);
		if (IsImage(obj))
		{
#if 1
			std::stringstream os;
			os << szSavePath << "\\img-" << uIndex << ".png";
			if (SaveImage(os.str(), uIndex))
			{
				if (ImageConvert(szSavePath, uIndex))
					WriteDataToStream(obj,szSavePath, uIndex);
			}
#endif
		}
		pdf_drop_obj(obj);
	}
	std::stringstream os;
	os << szSavePath << "\\out.pdf";
	std::string strOutPath = os.str();
	pdf_write_document(doc, (char*)strOutPath.c_str(), nullptr);

}

BOOL Util::MuPdf::ImageConvert(std::string szSavePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (szSavePath.empty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}

	std::stringstream osin;
	osin << szSavePath << "\\img-" << nNum << ".png";

	std::stringstream osout;
	osout << szSavePath << "\\img-" << nNum << ".jpg";

	ATL::CA2W strSrcImagePath(osin.str().c_str(), CP_ACP);
	ATL::CA2W strImageOutPath(osout.str().c_str(), CP_ACP);

	bRet = Util::Image::ConvertType(strSrcImagePath, strImageOutPath);
	return bRet;
}





BOOL  Util::MuPdf::WriteDataToStream(pdf_obj* obj, std::string szSavePath, INT32 nNum)
{
	//��������Ϣ  Test

	//ͼƬ·��
	std::stringstream osout;
	osout << szSavePath << "\\img-" << nNum << ".jpg";
	
	std::ifstream fin(osout.str().c_str(), std::ios::binary);
	fin.seekg(0, std::ios::end);
	UINT32 uSize = fin.tellg();
	UINT8* szBuf = new (std::nothrow)UINT8[uSize];

	fin.seekg(0, std::ios::beg);
	fin.read((char*)szBuf, sizeof(char) * uSize);
	fin.close();

	UINT32 uDestLen = uSize;
	UINT8* buffer_dest = new (std::nothrow)UINT8[uSize];

	compress(buffer_dest, (uLong*)&uDestLen, szBuf, uSize);

	fz_buffer * stm_buf = fz_new_buffer(ctx, (int)uDestLen);
	memcpy(stm_buf->data, buffer_dest, uDestLen);
	stm_buf->len = uDestLen;


	//���³���
	pdf_obj *type = pdf_dict_gets(obj, "Length");
	if (pdf_is_indirect(type))
	{
		int num = pdf_to_num(type);
		int gen = pdf_to_gen(type);
		pdf_obj* lenobj = pdf_load_object(doc, num, gen);
		pdf_set_int(lenobj, uDestLen);
		pdf_update_object(doc, num, lenobj);
	}
	else if (pdf_is_int(obj))
	{
		int i = pdf_to_num(obj);
	}

	//pdf_dict_puts_drop(obj, "Length", pdf_new_int(doc, udstLength));
	//������
	pdf_update_stream(doc, nNum, stm_buf);

	delete[] buffer_dest;
	delete[]  szBuf;

	

	//pdf_is_name(type) && !strcmp(pdf_to_name(type), "Image");


	return FALSE;
}


#if 0

unsigned long udstLength = 1024, usrcLength = 1024;
unsigned char *buffer_src = (unsigned char*)malloc(1024);
unsigned char *buffer_dest = (unsigned char*)malloc(1024);
memset(buffer_src, 0, 1024);
memset(buffer_dest, 0, 1024);
unsigned char buffer[] = "123456789asdfghjklzxcvb ���ΰ���� ";
usrcLength = strlen((char*)buffer);
ScopedMem<char> annot_apc_dict(str::Format(ap_dictFilter, usrcLength));
memcpy(buffer_src, buffer, usrcLength);
//���� zlib �� compress ��ע����Ϣѹ���� FlateDecode ѹ��,udstLength ���봫��һ���Ƚϴ��������
//usrcLength:Դ���ݳ���
//buffer_src:Դ����
//UdstLength:Ŀ�����ݳ���
//buffer_dest:Ŀ������
compress(buffer_dest, &udstLength, buffer_src, usrcLength);
//����һ��<</Filter/FlateDecode>>���͵� pdf ����
pdf_obj * apc_obj = pdf_new_obj_from_str(doc, annot_apc_dict);
//����һ�� fz_buffer*���͵��ڴ�ռ䣬 ������������ѹ������ע��Ϣ
fz_buffer * apc_buf = fz_new_buffer(ctx, (int)udstLength);
memcpy(apc_buf->data, buffer_dest, udstLength);
apc_buf->len = udstLength;

pdf_dict_puts_drop(apc_obj, "Length", pdf_new_int(doc, apc_buf->len)); 	//ָ��<</Filter/FlateDecode>>������ Length �Ĵ�С
int num0 = pdf_create_object(doc); 	//����һ���ն��� 19 0 obj....endobj���� ���ض������
pdf_update_object(doc, num0, apc_obj); 	//��<</Filter/FlateDecode/Length 42>>д�� 19 0 obj �� value ����

pdf_update_stream(doc, num0, apc_buf); 	//����������д�� stream...endstream

pdf_dict_puts_drop(annot_obj, "Contents", pdf_new_indirect(doc, num0, 0));//������ 19 0 obj�� �루 17 0 obj�� ������ /Contents 19 0 R��

#endif

#if  0
std::string strpath = "c1111.txt";

std::ifstream fin(strpath.c_str(), std::ios::binary);
fin.seekg(0, std::ios::end);
int iSize = fin.tellg();
char* szBuf = new (std::nothrow) char[iSize];

fin.seekg(0, std::ios::beg);
fin.read(szBuf, sizeof(char) * iSize);
fin.close();
//CxMemFile memfile((BYTE*)szBuf, iSize);

pdf_xref_entry *entry = pdf_get_xref_entry(doc, uIndex);
// 			fz_compressed_buffer *f_com_buffer = pdf_load_compressed_stream(doc, uIndex, entry->gen);
// 
// 
fz_stream * pdf_stm = pdf_open_stream(doc, uIndex, entry->gen);
int count, n;

//StrToHex((const char*)pdf_stm->rp, strlen((char*)pdf_stm->rp));
// 
// 			std::string strbuf((const char*)f_com_buffer->buffer->data, f_com_buffer->buffer->len);
// 			
//	StrToHex((const char*)f_com_buffer->buffer->data, f_com_buffer->buffer->len);

#endif