#include "scanner.h"

#include <cstdio>

namespace stacklang {
namespace {

// clang++ -std=c++1z ./scanner_test.cc -o /tmp/scanner_test && /tmp/scanner_test


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

void ExpectEq(vector<string> a, vector<string> b) {
	if(a.len() != b.len()) {
		fprintf(stderr, "Expect failed! a size %li b size %li\n", a.len(), b.len());
		return;
	}
	for(int64 idx=0;idx<a.len();++idx) {
		if(a[idx] != b[idx]) {
			fprintf(stderr, "Expect failed! a[%li] '%s' != b[%li] '%s'\n", 
				idx, a[idx].c_str(), 
				idx, b[idx].c_str());
		}
	}
}

void TestSimple() {
	fprintf(stderr, "--- TestSimple ---\n");

	const char* src = R"(
int add(int x, int y) {
	return x > y;
}
	)";

#if 0
	status_or<vector<string>> ret = compiler::Scan(src);

	if(!ret.ok()) {
		fprintf(stderr, "failed: %s\n", ret.status().message.c_str());
		exit(1);
	}

	vector<string> ref{"int", "add", "(", "int", "x", ",", "int", "y", ")", "{",
			"return", "x", ">", "y", ";",
			"}"};
	ExpectEq(ret.value(), 
			ref);
#endif
	try {
		vector<string> ret = compiler::Scan(src) throws();

		vector<string> ref{"int", "add", "(", "int", "x", ",", "int", "y", ")", "{",
				"return", "x", ">", "y", ";",
				"}"};
		ExpectEq(ret, 
				ref);
	} catch(Status error) {
		fprintf(stderr, "failed: %s\n", error.message.c_str());
		exit(1);
	}
}

#if 0
void TestSimple2() {
	fprintf(stderr, "--- TestSimple2 ---\n");

	const char* src = R"(
int add(int x, int y) {
	return x >> y;
} )";

	status_or<vector<string>> ret = compiler::Scan(src);

	if(!ret.ok()) {
		fprintf(stderr, "failed: %s\n", ret.status().message.c_str());
		exit(1);
	}

	vector<string> ref{"int", "add", "(", "int", "x", ",", "int", "y", ")", "{",
			"return", "x", ">>", "y", ";",
			"}"};
	ExpectEq(ret.value(), 
			ref);
}


void TestUnrecognizedSpecial() {
	fprintf(stderr, "--- TestUnrecognizedSpecial ---\n");

	const char* src = R"(
int add(int x, int y) {
	return x @ y;
}
	)";

	status_or<vector<string>> ret = compiler::Scan(src);

	Expect(!ret.ok());

	if(!ret.ok()) {
		fprintf(stderr, "failed: %s\n", ret.status().message.c_str());
	}
}

void TestLineMarker() {
	fprintf(stderr, "--- TestLineMarker ---\n");

	const char* src = R"(
int add(int x, int y) {
# 100 foo.c
	return x >> y;
} )";

	status_or<vector<string>> ret = compiler::Scan(src);

	if(!ret.ok()) {
		fprintf(stderr, "failed: %s\n", ret.status().message.c_str());
		exit(1);
	}

	vector<string> ref{"int", "add", "(", "int", "x", ",", "int", "y", ")", "{",
			"# 100 foo.c",
			"return", "x", ">>", "y", ";",
			"}"};
	ExpectEq(ret.value(), 
			ref);
}
#endif

}  // namespace
}  // namespace stacklang

int main() {
	stacklang::TestSimple();
#if 0
	stacklang::TestSimple2();
	stacklang::TestUnrecognizedSpecial();
	stacklang::TestLineMarker();
#endif
	return 0;
}