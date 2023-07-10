#include "map.h"

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
	fprintf(stderr, "--- TestSimple ---\n");
	map<string, int64> foo;
	ExpectEq(foo.size(), 0);
}


void TestInitList() {
	fprintf(stderr, "--- TestInitList ---\n");
	map<string, int64> foo{{"hey", 10}, {"foo", 3}};
	ExpectEq(foo.size(), 2);
}

void TestAddRemove() {
	fprintf(stderr, "--- TestAddRemove ---\n");
	map<string, int64> foo{{"hey", 10}};
	ExpectEq(foo.size(), 1);

	foo.set("you", 100);
	ExpectEq(foo.size(), 2);

	try {
		ExpectEq(foo.at("you"), 100) throws();
		ExpectEq(foo.at("hey"), 10) throws();
		foo.remove("hey");
		Expect(!	foo.contains("hey"));
		ExpectEq(foo.at("you"), 100) throws();
	} catch(Status status) {
		fprintf(stderr, "Failed: %s\n", status.message.c_str());
		exit(1);
	}



}


void TestOverwrite() {
	fprintf(stderr, "--- TestOverwrite ---\n");
	map<string, int64> foo{{"hey", 10}, {"foo", 3}};
	ExpectEq(foo.size(), 2);
	foo.set("foo", 111);
	try {
		ExpectEq(foo.at("hey"), 10) throws();
		ExpectEq(foo.at("foo"), 111) throws();
	} catch(Status status) {
		fprintf(stderr, "Failed: %s\n", status.message.c_str());
		exit(1);
	}

}


void TestKeys() {
	fprintf(stderr, "--- TestKeys ---\n");
	map<string, int64> foo{{"hey", 10}, {"foo", 3}};
	ExpectEq(foo.size(), 2);
	set<string> keys = foo.keys();
	ExpectEq(keys.size(), 2);
	Expect(keys.contains("hey"));
	Expect(keys.contains("foo"));
}


}  // namespace

}  // namespace stacklang


int main() {
	stacklang::TestSimple();
	stacklang::TestInitList();
	stacklang::TestAddRemove();
	stacklang::TestOverwrite();
	stacklang::TestKeys();
	return 0;
}
