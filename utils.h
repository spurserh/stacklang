#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "string.h"

// TODO: Put behind define
#define throws(...)

namespace stacklang {

struct LocationRef {
	int64 fileno = -1;
	int64 lineno = -1;
	int64 colno = -1;
};

struct Status {
	string message;
	LocationRef loc;

	bool ok()const {
		return message.len() == 0;
	}
};

template<typename T>
class status_or {
public:
	status_or(const T& value) : value_(value) { }
	status_or(const Status& status) : status_(status) { }

	bool ok()const {
		return status_.ok();
	}

	T value() const { return value_; }
	Status status() const { return status_; }
private:
	T value_;
	Status status_;
};

};  // stacklang

#endif//UTILS_H
