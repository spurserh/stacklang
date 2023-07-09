#include "string.h"

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
	const char*literal = "foo";
	string foo(literal);
	Expect(strcmp(foo.c_str(), literal) == 0);
}

void TestAdd() {
	string foo("foo");
	string bar("bar");
	string foobar = foo + bar;
	Expect(strcmp(foobar.c_str(), "foobar") == 0);
}

void TestHeadTail() {
	string test("this is a string");
	string tail = test.tail(5);
	fprintf(stderr, "-- tail '%s'\n", tail.c_str());
	Expect(strcmp(tail.c_str(), "is a string") == 0);
	string head = tail.head(9);
	fprintf(stderr, "-- head '%s'\n", head.c_str());
	Expect(strcmp(head.c_str(), "is") == 0);
}

void TestSubstr() {
	string test("hello big world");
	string str = test.substr(6, 3);
	fprintf(stderr, "-- str '%s'\n", str.c_str());
	Expect(strcmp(str.c_str(), "big") == 0);
}

void TestIterate() {
	fprintf(stderr, "-- TestIterate --\n");
	const char* init = "hello big world";
	string test(init);
	int i=0;
	for(char c : test) {
		ExpectEq(c, init[i++]);
	}

}

}  // namespace

}  // namespace stacklang


int main() {
	stacklang::TestSimple();
	stacklang::TestAdd();
	stacklang::TestHeadTail();
	stacklang::TestSubstr();
	stacklang::TestIterate();
	return 0;
}
