#ifndef STRING_H
#define STRING_H

#include "types.h"

#include <string.h>
#include <assert.h>

#include <stdio.h>

namespace stacklang {

// Immutable string
class string {
 public:
 	string() {}
 	// Assumes this is a literal and will not change
 	string(const char* literal) 
 		: storage_(literal), 
 		  len_(strlen(literal)) {
 	}
 	string(char c) {
 		len_ = 1;
 		storage_ = new char(c);
 	}
 	string(const string& other) : 
 		storage_(other.storage_), 
 		len_(other.len_) {
 	}
 	bool operator ==(string o)const {
 		if(len_ != o.len_) {
 			return false;
 		}
 		return memcmp(storage_, o.storage_, len_) == 0;
 	}
 	bool operator !=(string o)const {
 		return !(*this == o);
 	}
 	bool operator <(string o)const {
 		if(len_ != o.len_) {
 			return len_ < o.len_;
 		}
 		return memcmp(storage_, o.storage_, len_) < 0;
 	}
 	char operator[](int64 index)const {
 		assert(index >= 0);
 		assert(index < len_);
 		return storage_[index];
 	}
 	string operator+(string other)const {
 		int64 new_len = len_ + other.len_;
 		char* storage = new char[new_len];
 		memcpy(storage, storage_, len_);
 		memcpy(storage + len_, other.storage_, other.len_);
 		return string(storage, new_len);
 	}
 	int64 len() const {
 		return len_;
 	}
 	bool empty()const {
 		return len_ == 0;
 	}
 	string tail(int64 without_n)const {
 		assert(len_ >= without_n);
 		return string(storage_ + without_n, len_ - without_n);
 	}
 	string head(int64 without_n)const {
 		assert(len_ >= without_n);
 		return string(storage_, len_ - without_n);
 	}
 	string substr(int64 pos, int64 len)const {
 		assert(len_ >= (pos + len));
 		return string(storage_ + pos, len);
 	}
 	char* c_str() {
 		char* ret = new char[len_+1];
 		memcpy(ret, storage_, len_);
 		ret[len_] = 0;
 		return ret;
 	}
 	class iterator {
 	public:
 		iterator(const string* to, int64 index)
 		  : to_(to), index_(index) {
 		}
 		bool operator!=(iterator o)const {
			if(to_ != o.to_) {
				return true;
			}
			return index_ != o.index_;
		}
		char operator*()const {
			return (*to_)[index_];
		}
		// prefix
		iterator operator++() {
			++index_;
			return *this;
		}
 	private:
 		const string* to_;
 		int64 index_;
 	};
 	iterator begin()const {
 		return iterator(this, 0);
 	}
 	iterator end()const {
 		return iterator(this, len_);
 	}
 private:
 	string(const char* storage, int64 len) 
 		: storage_(storage), len_(len) { }
 	const char* storage_ = "";
 	int64 len_ = 0;
};

};  // stacklang

#endif//STRING_H
