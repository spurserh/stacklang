#include "set.h"

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
	set<int64> foo;
	ExpectEq(foo.size(), 0);
}


void TestInitList() {
	set<int64> foo{4, 10};
	ExpectEq(foo.size(), 2);
}

void TestAdd1() {
	set<int64> foo;
	Expect(!foo.contains(10));
	foo.add(10);
	ExpectEq(foo.size(), 1);
	Expect(foo.contains(10));
	for(int64 v : foo) {
		ExpectEq(v, 10);
	}
}


void TestAddRepeat() {
	set<int64> foo;
	Expect(!foo.contains(10));
	foo.add(10);
	foo.add(10);
	ExpectEq(foo.size(), 1);
	Expect(foo.contains(10));
	for(int64 v : foo) {
		ExpectEq(v, 10);
	}
}

void TestRemove() {
	set<int64> foo;
	foo.add(10);
	foo.add(15);
	foo.add(22);
	ExpectEq(foo.size(), 3);
	Expect(foo.contains(15));
	foo.remove(15);
	Expect(!foo.contains(15));
}




}  // namespace

}  // namespace stacklang


int main() {
	stacklang::TestSimple();
	stacklang::TestInitList();
	stacklang::TestAdd1();
	stacklang::TestAddRepeat();
	stacklang::TestRemove();
	return 0;
}
