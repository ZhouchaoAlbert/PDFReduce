#ifndef __BASE_COM_H__
#define __BASE_COM_H__

#include "SmartRef.h"
#include "SmartPointer.h"

//
// 线程管理接口
//
namespace Thread{

	//
	// 回调
	//
	class IThreadCallback : public ISmartRef
	{
	public:
		//函数名称：GetClass
		//功   能：获取类名称
		virtual const char* GetClass() = 0;


		//函数名称：Run
		//功   能：运行线程任务
		virtual void Run() = 0;
	};

	//
	// 线程管理接口
	//
	class IThreadManager
	{
	public:
		virtual ~IThreadManager() {}
	public:

		// 函数名称：CreateThread
		// 功   能：创建线程
		// 参   数：spCallBack 线程回调, uiThreadID 成功时候返回线程id
		// 返回值：成功返回ture,否则返回false

		virtual bool CreateThread(CScopedRefPtr<IThreadCallback> spCallBack,
			unsigned int & uiThreadID) = 0;

		// 函数名称：ExistThread
		// 功   能：退出指定线程
		// 参   数：uiThreadID 线程id， bForced 是否强制退出线程
		virtual void ExistThread(unsigned int uiThreadID, bool bForced) = 0;


		//函数名称：GetCallbackByThreadID
		//功   能：根据线程ID拿线程回调对象
		//参   数：uiThreadID 线程id
		//返回值：返回回调对象指针
		virtual CScopedRefPtr<IThreadCallback> GetCallbackByThreadID(unsigned int uiThreadID) = 0;

		// 函数名称：ExistAll
		// 功   能：退出所有线程
		// 参   数：bForced 是否强制退出线程
		virtual void ExistAll(bool bForced) = 0;
	};
}
#endif
