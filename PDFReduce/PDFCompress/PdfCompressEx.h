#ifndef __PDF_COMPRESS_H__
#define __PDF_COMPRESS_H__

#include <atlstr.h>
#include <string>
#include <list>
#include <functional>
#include "..\Msg\UtilMsg.h"
#include "..\Base\BaseCom.h"
#include "..\Thread\ThreadManager.h"
#include "..\Help\MyLock.h"

using namespace ATL;
using namespace std;
extern "C"
{
#include <mupdf/pdf.h>
#include <mupdf/fitz.h>
}

struct ST_PDFINFO_NODE
{
	INT32   nId;						//唯一标示  id递增
	CString strPdfInPath;				//输入PDF路径 
	CString strPassword;
	CString strPdfOutFolder;			//输出PDF文件夹
};

enum E_PDFCOMPRESS_STATE
{
	E_PDFCOMPRESS_STATE_FINISH  = 0,	//0 完成
	E_PDFCOMPRESS_STATE_PRASE   = 1,	//1 解析中
	E_PDFCOMPRESS_STATE_PERFECT = 2,	//2 完美PDF 不需要处理
	E_PDFCOMPRESS_STATE_FAIL    = 3,	//3 解析失败
};

struct ST_COMPRESS_RESULT
{
	CString					strPdfInPath;//唯一标示
	E_PDFCOMPRESS_STATE     eState;		//压缩状态
	INT32					nVal;		//进度值
	CString					strOutInfo; //输出信息
};


class CPdfCompressEx : public Util::Msg::CMsgBase, public Thread::IThreadCallback
{
public:
	CPdfCompressEx();
	~CPdfCompressEx();

	//添加任务队列
	void   AddTask(ST_PDFINFO_NODE TaskNode);
	//启动线程开始压缩PDF文件
	UINT32 StartCompress();

	//设置进度回调
	void  SetProcessCallback(std::function<void(INT32 nCode, CString strPdfInPath, INT32 nVal, CString strOutInfo)>Func_CallBack);
	//退出线程
	void ExistThread(bool bForce);

protected:
	//初始化操作
	BOOL DocumentInit(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));

	//获取PDF页
	UINT32 GetPdfPages(pdf_document* doc);
	//是否是图片
	BOOL IsImageType(pdf_obj* dict);
	//彩色空间是RGB
	BOOL IsRGBColorSpace(pdf_obj* dict);
	//是zlib压缩
	BOOL IsFlateDecode(pdf_obj* dict);

	//解析资源图片
	void PraseImageTypeObj(CString strPdfInPath, CString strPdfOutPath);
	//保存图片PNG
	BOOL SaveImageAsPng(ATL::CString strImagePath, INT32 nNum);
	//保存图片格式
	BOOL SavaImageAsJpg(INT32 nNum);
	//PDF中图片压缩格式是FlateDecode 与转换后jpg的图片大小比较
	BOOL IsWriteStream(INT32 nNum);
	//写pixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb);

	//数据回写
	BOOL WriteDataToStream(pdf_obj* dict, ATL::CString  strDestImagePath, INT32 nNum);


	//消息
	virtual void OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	// 获取类名称
	virtual const char* GetClass();
	//执行线程
	virtual void Run();
	//跳线程
	void JumpThreadSetProgress(CString strPdfInPath, E_PDFCOMPRESS_STATE eState, INT32 nVal, CString strOutInfo);

	IMPLEMENT_REFCOUNT(CPdfCompressEx);
private:
	BOOL                                  m_bRun;
	UINT32                                m_uThreadId;
	std::list<ST_PDFINFO_NODE>		      m_TaskList;
	CCritSec                              m_CritSec;
	fz_context_s* m_ctx; 
	pdf_document* m_doc; 



	std::function<void(INT32 nCode, CString strPdfInPath, INT32 nVal, CString strOutInfo)>m_Func_CallBack;
};

#endif