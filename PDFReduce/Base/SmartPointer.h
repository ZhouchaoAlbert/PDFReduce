#ifndef __SMART_POINTER_H__
#define __SMART_POINTER_H__

/*
智能指针
*/
template <class T>
class CScopedRefPtr {
public:
	typedef T element_type;

	CScopedRefPtr() : ptr_(NULL) {
	}

	CScopedRefPtr(T* p) : ptr_(p) {
		if (ptr_)
			ptr_->AddRef();
	}

	CScopedRefPtr(const CScopedRefPtr<T>& r) : ptr_(r.ptr_) {
		if (ptr_)
			ptr_->AddRef();
	}

	template <typename U>
	CScopedRefPtr(const CScopedRefPtr<U>& r) : ptr_(r.get()) {
		if (ptr_)
			ptr_->AddRef();
	}

	~CScopedRefPtr() {
		if (ptr_)
			ptr_->Release();
	}

	T* get() const { 
		return ptr_; 
	}

	operator T*() const { 
		return ptr_; 
	}

	T** operator&(){ 
		return &ptr_; 
	}

	T* operator->() const {
		return ptr_;
	}

	CScopedRefPtr<T>& operator=(T* p) {
		if (p)
			p->AddRef();
		T* old_ptr = ptr_;
		ptr_ = p;
		if (old_ptr)
			old_ptr->Release();
		return *this;
	}

	CScopedRefPtr<T>& operator=(const CScopedRefPtr<T>& r) {
		return *this = r.ptr_;
	}

	template <typename U>
	CScopedRefPtr<T>& operator=(const CScopedRefPtr<U>& r) {
		return *this = r.get();
	}

	void attach(T* p) {
		T* old_ptr = ptr_;
		ptr_ = p;
		if (old_ptr)
			old_ptr->Release();
	}

	void swap(T** pp) {
		T* p = ptr_;
		ptr_ = *pp;
		*pp = p;
	}

	void swap(CScopedRefPtr<T>& r) {
		swap(&r.ptr_);
	}
protected:
	T* ptr_;
};

#endif
