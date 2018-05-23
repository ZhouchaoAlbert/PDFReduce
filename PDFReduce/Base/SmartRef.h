//
// 描述：智能指针基类
// 注意：构造函数必须初始化m_ulRef为1
//

#ifndef __SMART_REF_H__
#define __SMART_REF_H__

class ISmartRef
{
public:
	virtual unsigned long AddRef() = 0;

	virtual unsigned long Release()  = 0;	
protected:
	virtual ~ISmartRef() {}
};

#define IMPLEMENT_REFCOUNT(ClassName)				\
  public:                                           \
    unsigned long AddRef()   {						\
      return ++m_ulRef;								\
    }                                               \
    unsigned long Release()   {						\
      if (0 == --m_ulRef){							\
			delete this;							\
			return 0;								\
		}											\
		return m_ulRef;								\
    }												\
  private:                                          \
    unsigned long	m_ulRef {1};

#endif