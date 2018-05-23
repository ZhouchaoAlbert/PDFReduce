#ifndef __BASE_COM_H__
#define __BASE_COM_H__

#include "SmartRef.h"
#include "SmartPointer.h"

//
// �̹߳���ӿ�
//
namespace Thread{

	//
	// �ص�
	//
	class IThreadCallback : public ISmartRef
	{
	public:
		//�������ƣ�GetClass
		//��   �ܣ���ȡ������
		virtual const char* GetClass() = 0;


		//�������ƣ�Run
		//��   �ܣ������߳�����
		virtual void Run() = 0;
	};

	//
	// �̹߳���ӿ�
	//
	class IThreadManager
	{
	public:
		virtual ~IThreadManager() {}
	public:

		// �������ƣ�CreateThread
		// ��   �ܣ������߳�
		// ��   ����spCallBack �̻߳ص�, uiThreadID �ɹ�ʱ�򷵻��߳�id
		// ����ֵ���ɹ�����ture,���򷵻�false

		virtual bool CreateThread(CScopedRefPtr<IThreadCallback> spCallBack,
			unsigned int & uiThreadID) = 0;

		// �������ƣ�ExistThread
		// ��   �ܣ��˳�ָ���߳�
		// ��   ����uiThreadID �߳�id�� bForced �Ƿ�ǿ���˳��߳�
		virtual void ExistThread(unsigned int uiThreadID, bool bForced) = 0;


		//�������ƣ�GetCallbackByThreadID
		//��   �ܣ������߳�ID���̻߳ص�����
		//��   ����uiThreadID �߳�id
		//����ֵ�����ػص�����ָ��
		virtual CScopedRefPtr<IThreadCallback> GetCallbackByThreadID(unsigned int uiThreadID) = 0;

		// �������ƣ�ExistAll
		// ��   �ܣ��˳������߳�
		// ��   ����bForced �Ƿ�ǿ���˳��߳�
		virtual void ExistAll(bool bForced) = 0;
	};
}
#endif
