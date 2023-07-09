#include "vector.h"

#include <cstdio>

namespace stacklang {
namespace {

// TODO: Defines
void Expect(bool stmt) {
	if(!stmt) {
		fprintf(stderr, "Expect failed!\n");
	}
}

void ExpectEq(int64 a, int64 b) {
	if(a != b) {
		fprintf(stderr, "Expect failed! %lx != %lx\n",
			a, b);
	}
}

void TestSimple() {
	vector<int64> foo;
	ExpectEq(foo.len(), 0);
}

void TestInitList() {
	vector<int64> foo({4, 6});
	ExpectEq(foo.len(), 2);
}


void TestAdd1() {
	vector<int64> foo;
	foo.push_back(10);
	assert(foo.len() == 1);
	ExpectEq(foo[0], 10);
}

void TestRemove() {
	vector<int64> foo;
	foo.push_back(10);
	foo.push_back(20);
	assert(foo.len() == 2);
	ExpectEq(foo[0], 10);
	ExpectEq(foo[1], 20);
	foo.pop_back();
	assert(foo.len() == 1);
	ExpectEq(foo[0], 10);
}

void TestRemove2() {
	vector<int64> foo;
	foo.push_back(10);
	foo.push_back(20);
	assert(foo.len() == 2);
	ExpectEq(foo[0], 10);
	ExpectEq(foo[1], 20);
	foo.pop_front();
	assert(foo.len() == 1);
	ExpectEq(foo[0], 20);
}

void TestIterate() {
	vector<int64> foo;
	foo.push_back(10);
	foo.push_back(20);
	assert(foo.len() == 2);
	int64 ref_array[2] = {10, 20};
	int64* ref_array_ptr = &ref_array[0];
	for(int64 v : foo) {
		ExpectEq(v, *(ref_array_ptr++));
	}
}

}  // namespace

}  // namespace stacklang


int main() {
	stacklang::TestSimple();
	stacklang::TestAdd1();
	stacklang::TestInitList();
	stacklang::TestRemove();
	stacklang::TestRemove2();
	stacklang::TestIterate();
	return 0;
}
