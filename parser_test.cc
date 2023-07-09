#include "parser.h"
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

//	return x + y;
	const char* src = R"(
int add(int x, int y) {
	return x + y;
}
	)";

	try {
		vector<string> tokens = compiler::Scan(src) throws();

		compiler::Namespace parsed = compiler::Parse(tokens) throws();

		fprintf(stderr, "result:\n%s\n", parsed.DebugString().c_str());
	} catch(Status error) {
		fprintf(stderr, "failed: %s\n", error.message.c_str());
		exit(1);
	}

}

}  // namespace
}  // namespace stacklang

int main() {
	stacklang::TestSimple();
	return 0;
}