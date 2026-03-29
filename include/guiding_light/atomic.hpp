#ifndef ATOMIC_H
#define ATOMIC_H

#include <atomic>

template <typename T>
class atomic_ptr : public std::atomic <T*> {
public:
	using std::atomic <T*>::atomic;
	using std::atomic <T*>::operator++;
	using std::atomic <T*>::operator+=;
	using std::atomic <T*>::operator--;
	using std::atomic <T*>::operator-=;
	using std::atomic <T*>::operator=;

	T* operator ->() const {
		return this->load();
	}
};

#endif
