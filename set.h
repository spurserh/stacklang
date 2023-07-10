#ifndef SET_H
#define SET_H

#include "types.h"

#include "vector.h"
#include "utils.h"

// STL
#include <initializer_list>
#include <assert.h>

namespace stacklang {

template<typename T>
class set {
public:
	set() {}
	set(const set& other) 
		: storage_(other.storage_) {
	}
	set(std::initializer_list<T> inits) {
		for(T init : inits) {
			add(init);
		}
	}

	int64 size()const {
		return storage_.len();
	}

	bool empty()const {
		return storage_.len() == 0;
	}

	bool contains(T value)const {
		for(T v : storage_) {
			// Check equality
			if(!(v < value) && !(value < v)) {
				return true;
			}
		}
		return false;
	}

	void add(T value) {
		if(!contains(value)) {
			storage_.push_back(value);
		}
	}
	void add(set other) {
		for(T v : other) {
			add(v);
		}
	}

	void remove(T value) {
		if(!contains(value)) {
			return;
		}
		vector<T> new_storage;
		for(T v : storage_) {
			// TODO: Just use <
			if(v == value) {
				continue;
			}
			new_storage.push_back(v);
		}
		storage_ = new_storage;
	}

	T get(T value)const throws(Status) {
		// TODO: Inefficient
		// TODO: Just use <
		for(T v : storage_) {
			if(v == value) {
				return v;
			}
		}
		throw Status{.message = "Couldn't find element"};
	}

	typedef typename vector<T>::iterator iterator;

	iterator begin()const {
		return storage_.begin();
	}
	iterator end()const {
		return storage_.end();
	}


private:
	vector<T> storage_;
};

};  // stacklang

#endif//SET_H
