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

template <typename T>
class movable_atomic : public std::atomic <T> {
public:
	using std::atomic <T>::atomic;
	using std::atomic <T>::operator=;

	movable_atomic(movable_atomic&& other) noexcept {
		this->store(std::move(other));
	}

	operator T() const {
		return this->load();
	}
};

#endif
