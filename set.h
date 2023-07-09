#ifndef SET_H
#define SET_H

#include "types.h"

#include "vector.h"

// STL
#include <initializer_list>
#include <assert.h>

namespace stacklang {

// Immutable vector
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

	void remove(T value) {
		if(!contains(value)) {
			return;
		}
		vector<T> new_storage;
		for(T v : storage_) {
			if(v == value) {
				continue;
			}
			new_storage.push_back(v);
		}
		storage_ = new_storage;
	}

	typename vector<T>::iterator begin()const {
		return storage_.begin();
	}
	typename vector<T>::iterator end()const {
		return storage_.end();
	}


private:
	vector<T> storage_;
};

};  // stacklang

#endif//SET_H
