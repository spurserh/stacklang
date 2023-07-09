#ifndef VECTOR_H
#define VECTOR_H

#include "types.h"

// STL
#include <initializer_list>
#include <assert.h>

namespace stacklang {

// Immutable vector
template<typename T>
class vector {
public:
	vector() {}
	vector(const vector& other) 
		: storage_(other.storage_),
		  len_(other.len_) {
	}
	vector(std::initializer_list<T> inits) {
		T* new_storage = new T[inits.size()];
		int64 i = 0;
		for(T init : inits) {
			new_storage[i++] = init;
		}
		len_ = inits.size();
		storage_ = new_storage;
	}

	T operator[](int64 index) const {
		assert(index >= 0);
		assert(index < len_);
		return storage_[index];
	}

	int64 len()const {
		return len_;
	}

	bool empty()const {
		return len_ == 0;
	}

	void push_back(T value) {
		// TODO: Very inefficient
		T* new_storage = new T[len_ + 1];
		for(int64 i=0;i<len_;++i) {
			new_storage[i] = storage_[i];
		}
		new_storage[len_] = value;
		len_ += 1;
		storage_ = new_storage;
	}

	T pop_back(int64 n=1) {
		assert(n > 0);
		assert(n <= len_);
		T ret = storage_[len_-1];
		len_ -= n;
		return ret;
	}

	T pop_front(int64 n=1) {
		assert(n > 0);
		assert(n <= len_);
		T ret = storage_[0];
		storage_ += n;
		len_ -= n;
		return ret;
	}

	class iterator {
	public:
		iterator(const vector* to, int64 index)
			: to_(to), index_(index) { }
		bool operator!=(iterator o)const {
			if(to_ != o.to_) {
				return true;
			}
			return index_ != o.index_;
		}
		T operator*()const {
			return (*to_)[index_];
		}
		// prefix
		iterator operator++() {
			++index_;
			return *this;
		}
	private:
		const vector* to_;
		int64 index_;
	};

	iterator begin()const {
		return iterator(this, 0);
	}
	iterator end()const {
		return iterator(this, len_);
	}

private:
	vector(T* storage, int64 len) : storage_(storage), len_(len) {
	}
	T const* storage_ = nullptr;
	int64 len_ = 0;
};

};  // stacklang

#endif//VECTOR_H
