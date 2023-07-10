#ifndef MAP_H
#define MAP_H

#include "types.h"
#include "set.h"
#include "utils.h"

namespace stacklang {

template<typename K, typename V>
class map {
public:

	struct Pair {
		K key;
		V value;

		Pair() {}
		Pair(Pair const&other) : key(other.key), value(other.value) {}
		Pair(K key, V value) : key(key), value(value) {}


		bool operator<(Pair other)const {
			return key < other.key;
		}
		bool operator==(Pair other)const {
			return key == other.key;
		}
	};

	map() {}
	map(const map& other) 
		: pairs_(other.pairs_) {
	}
	map(std::initializer_list<Pair> inits) {
		for(Pair init : inits) {
			set(init.key, init.value);
		}
	}

	int64 size()const {
		return pairs_.size();
	}

	bool empty()const {
		return pairs_.empty();
	}

	bool contains(K key)const {
		return pairs_.contains(Pair(key, V()));
	}

	stacklang::set<K> keys() const {
		stacklang::set<K> ret;
		for(Pair p : pairs_) {
			ret.add(p.key);
		}
		return ret;
	}

	void set(K key, V value) {
		Pair pair(key, value);
		pairs_.remove(pair);
		pairs_.add(pair);
	}

	void remove(K key) {
		pairs_.remove(Pair(key, V()));
	}

	V at(K key)const throws(Status) {
		return pairs_.get(Pair(key, V())).value throws();
	}

	typedef typename stacklang::set<Pair>::iterator iterator;

	iterator begin()const {
		return pairs_.begin();
	}
	iterator end()const {
		return pairs_.end();
	}

private:

	stacklang::set<Pair> pairs_;
};

}  // namespace stacklang

#endif//MAP_H
