#include "parser.h"
#include "scanner.h"

#include <cstdio>


namespace stacklang {
namespace {

// clang++ -std=c++1z ./scanner_test.cc -o /tmp/scanner_test && /tmp/scanner_test


struct TestSpec {
	string name;
	void(*func)();
};

vector<TestSpec> sTests;
map<string, bool> sTestsPassed;

int64 CountNodes(compiler::Expr* expr) {
	int64 ret = 1;
	for(compiler::Expr* operand : expr->GetOperands()) {
		ret += CountNodes(operand);
	}
	return ret;
}

#define DECLARE_TEST(__name) \
	void test_##__name(); \
	static struct Init__##__name { \
		Init__##__name() { \
			sTests.push_back({.name = #__name, .func = test_##__name}); \
		} \
	} init__##__name; \
	void test_body_##__name(string __test_name); \
	void test_##__name() { \
		sTestsPassed.set(#__name, true); \
		try{ \
			test_body_##__name(#__name); \
		}catch(Status status) { \
			fprintf(stderr, "FAILED test %s: Caught %s\n", #__name, status.message.c_str()); \
			sTestsPassed.set(#__name, false); \
		} \
	} \
	void test_body_##__name(string __test_name) 

compiler::Namespace TestParse(const char* src) throws() {
	vector<string> tokens = compiler::Scan(src) throws();

	return compiler::Parse(tokens) throws();
}


// TODO: Defines
void Expect(string test_name, bool stmt) {
	if(!stmt) {
		fprintf(stderr, "Expect failed!\n");
		sTestsPassed.set(test_name, false);
	}
}

void ExpectEq(string test_name, int64 a, int64 b) {
	if(a != b) {
		fprintf(stderr, "Expect failed! %lx != %lx\n",
			a, b);
		sTestsPassed.set(test_name, false);
	}
}

void ExpectEq(string test_name, string a, string b) {
	if(a != b) {
		fprintf(stderr, "Expect failed! %s != %s\n",
			a.c_str(), b.c_str());
		sTestsPassed.set(test_name, false);
	}
}

void ExpectEq(string test_name, vector<string> a, vector<string> b) {
	if(a.len() != b.len()) {
		fprintf(stderr, "Expect failed! a size %li b size %li\n", a.len(), b.len());
		sTestsPassed.set(test_name, false);
		return;
	}
	for(int64 idx=0;idx<a.len();++idx) {
		if(a[idx] != b[idx]) {
			fprintf(stderr, "Expect failed! a[%li] '%s' != b[%li] '%s'\n", 
				idx, a[idx].c_str(), 
				idx, b[idx].c_str());
			sTestsPassed.set(test_name, false);
		}
	}
}

template<typename T>
void ExpectNotNull(string test_name, T* a) {
	if(a == nullptr) {
		fprintf(stderr, "Expect failed! %p != nullptr\n",
			a);
		sTestsPassed.set(test_name, false);
	}
}


template<typename T>
void ExpectNull(string test_name, T* a) {
	if(a != nullptr) {
		fprintf(stderr, "Expect failed! %p == nullptr\n",
			a);
		sTestsPassed.set(test_name, false);
	}
}

void Assert(string test_name, bool cond) {
	if(!cond) {
		fprintf(stderr, "Assert failed! \n");
		sTestsPassed.set(test_name, false);
	}
}

void FailWithMessage(string test_name, string msg) {
	fprintf(stderr, "FAIL: %s\n", msg.c_str());
	sTestsPassed.set(test_name, false);
}

#define EXPECT_EQ(__a, __b) ExpectEq(__test_name, __a, __b)
#define EXPECT_NE(__a, __b) ExpectNe(__test_name, __a, __b)
#define EXPECT_NOT_NULL(__a) ExpectNotNull(__test_name, __a)
#define EXPECT_NULL(__a) ExpectNull(__test_name, __a)
#define ASSERT(__a) {Assert(__test_name, __a);if(!(__a)) { return; }}
#define FAIL(__msg) FailWithMessage(__test_name, __msg) 


DECLARE_TEST(Simple)
{
//	return x + y;
	const char* src = R"(
int top(int x, int y) {
	return x + y;
}
	)";

	(void)TestParse(src);
}


compiler::Decl* ParseAndGetTop(const char* src) {
	compiler::Namespace parsed = TestParse(src);

	compiler::Decl* top_decl = nullptr;
	for(compiler::Decl* d : parsed.GetDecls()) {
		if(d->GetName() == "top") {
			top_decl = d;
			break;
		}
	}
	assert(top_decl != nullptr);
	return top_decl;
}

vector<compiler::Stmt*> ParseAndGetTopBody(const char* src) {
	compiler::Decl* decl = ParseAndGetTop(src);
	auto func_decl = compiler::AsA<compiler::FuncDecl*>(decl);
	assert(func_decl != nullptr);
	vector<compiler::Stmt*> body = func_decl->GetBody();
	return body;
}

compiler::Expr* TestSingleFunctionSingleReturn(const char* src) {

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 1);
	compiler::Stmt* first_stmt = body[0];
	auto *return_stmt = compiler::AsA<compiler::ReturnStmt*>(first_stmt);
	assert(return_stmt != nullptr);
	compiler::Expr* return_value = return_stmt->GetValue();
	assert(return_value != nullptr);

	return return_value;
}

DECLARE_TEST(OperatorPrecedence) {

//	return x + y;
	const char* src = R"(
int top(int x, int y) {
	return 5 * x + y;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "+");
	EXPECT_EQ(CountNodes(top), 5);
}

DECLARE_TEST(TestOperatorPrecedence2) {

//	return x + y;
	const char* src = R"(
int top(int x, int y) {
	return 5 | x * 3 + y;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "|");
	EXPECT_EQ(CountNodes(top), 7);
}


DECLARE_TEST(TestOperatorPrecedence3)	 {

	const char* src = R"(
int top(int x, int y) {
	return (x+y)*3;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "*");
	EXPECT_EQ(CountNodes(top), 6);
}

DECLARE_TEST(TestOperatorPrecedence4) {

	const char* src = R"(
int top(int x, int y) {
	return 3 / (x+y);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "/");
	EXPECT_EQ(CountNodes(top), 6);
}


DECLARE_TEST(TestOperatorPrecedenceSame) {

	const char* src = R"(
int top(int x, int y) {
	return x+y-10;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "+");
	EXPECT_EQ(CountNodes(top), 5);
}


DECLARE_TEST(CStyleCast)
{
//	return x + y;
	const char* src = R"(
int top(int x, int y) {
	return (int)x + y;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "+");
	EXPECT_EQ(CountNodes(top), 4);
	auto left_op = compiler::AsA<compiler::CastExpr*>(top_op->GetLeft());
	EXPECT_NOT_NULL(left_op);
}


DECLARE_TEST(CStyleCastUnary)
{
	const char* src = R"(
int top(int x, int y) {
	return (int)*x + y;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_op = compiler::AsA<compiler::BinaryOp*>(top);

fprintf(stderr, "top:\n{\n%s\n}\n", top_op->DebugString(0).c_str());

	ASSERT(top_op != nullptr);
	EXPECT_EQ(top_op->GetOp(), "+");
	EXPECT_EQ(CountNodes(top), 5);
	auto left_op_cast = compiler::AsA<compiler::CastExpr*>(top_op->GetLeft());
	EXPECT_NOT_NULL(left_op_cast);
	auto sub_unary = compiler::AsA<compiler::UnaryOp*>(left_op_cast->GetSub());
	EXPECT_NOT_NULL(sub_unary);
}

DECLARE_TEST(Increment)
{
	const char* src = R"(
int top(int x, int y) {
	return ++x;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);

	auto top_top_unary = compiler::AsA<compiler::UnaryOp*>(top);
	ASSERT(top_top_unary != nullptr);

	EXPECT_EQ(top_top_unary->IsPostfix(), false);

	EXPECT_EQ(CountNodes(top), 2);
}

DECLARE_TEST(PostIncrement)
{
	const char* src = R"(
int top(int x, int y) {
	return x + y++;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 4);

	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), "+");

	auto uop = compiler::AsA<compiler::UnaryOp*>(top_bop->GetRight());
	ASSERT(uop != nullptr);

	EXPECT_EQ(uop->IsPostfix(), true);
	EXPECT_EQ(uop->GetOp(), "++");

}

DECLARE_TEST(PostIncrementDeref)
{
	const char* src = R"(
int top(int x, int y) {
	return *y++;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 3);

fprintf(stderr, "top %s\n", top->DebugString(0).c_str());

	auto top_unary = compiler::AsA<compiler::UnaryOp*>(top);
	ASSERT(top_unary != nullptr);
	EXPECT_EQ(top_unary->GetOp(), "*");
	EXPECT_EQ(top_unary->IsPostfix(), false);

	auto sub_unary = compiler::AsA<compiler::UnaryOp*>(top_unary->GetSub());
	ASSERT(sub_unary != nullptr);
	EXPECT_EQ(sub_unary->GetOp(), "++");
	EXPECT_EQ(sub_unary->IsPostfix(), true);
}

DECLARE_TEST(CommaIncrement)
{
	const char* src = R"(
int top(int x, int y) {
	return ++x, y++;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 5);

fprintf(stderr, "top %s\n", top->DebugString(0).c_str());

	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), ",");
}

DECLARE_TEST(CommaAdd)
{
	const char* src = R"(
int top(int x, int y) {
	return 5, x + y;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 5);

fprintf(stderr, "top %s\n", top->DebugString(0).c_str());

	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), ",");
}

DECLARE_TEST(FuncCallNonFunction)
{
	const char* src = R"(

int top(int x, int y) {
	return x(x, y);
}
	)";

	try {
		compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	} catch(Status status) {
		return;
	}

	FAIL("Should fail");
}

DECLARE_TEST(FuncCall)
{
	const char* src = R"(
int sum(int x, int y, int z) {
	return x + y + z;
}

int top(int x, int y) {
	return sum(x, 2*y, 10);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 6);

	auto top_call = compiler::AsA<compiler::FuncCall*>(top);
	ASSERT(top_call != nullptr);
	EXPECT_EQ(top_call->GetCallee()->GetRef()->GetName(), "sum");
}

DECLARE_TEST(FuncCallWrongArgs)
{
	const char* src = R"(
int sum(int x, int y, int z) {
	return x + y + z;
}

int top(int x, int y) {
	return sum(x);
}
	)";

	try {
		compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	} catch(Status status) {
		return;
	}

	FAIL("Should fail");
}


DECLARE_TEST(Recursion)
{
	const char* src = R"(

int top(int x) {
	return top(x-1);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 4);

	auto top_call = compiler::AsA<compiler::FuncCall*>(top);
	ASSERT(top_call != nullptr);
	EXPECT_EQ(top_call->GetCallee()->GetRef()->GetName(), "top");
}


DECLARE_TEST(FuncCallPrecedence)
{
	const char* src = R"(
int sum(int x, int y, int z) {
	return x + y + z;
}

int top(int x, int y) {
	return ++sum(x, 2*y, 10);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 7);

	auto top_uop = compiler::AsA<compiler::UnaryOp*>(top);
	ASSERT(top_uop != nullptr);
	EXPECT_EQ(top_uop->GetOp(), "++");
	EXPECT_EQ(top_uop->IsPostfix(), false);
}


DECLARE_TEST(FuncTemplateIntWrongUsage)
{
	const char* src = R"(
template <int T>
T add1(T x) {
	return x;
}
	)";

	try {
		compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	} catch(Status status) {
		fprintf(stderr, "Caught %s\n", status.message.c_str());
		return;
	}

	FAIL("Should fail");
}


DECLARE_TEST(FuncCallTemplate)
{
	const char* src = R"(
template <typename T>
T add1(T x) {
	return x;
}

int top(int x, int y) {
	return add1<int>(x + y);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 4);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_call = compiler::AsA<compiler::FuncCall*>(top);
	ASSERT(top_call != nullptr);
	EXPECT_EQ(top_call->GetCallee()->GetRef()->GetName(), "add1");
}

DECLARE_TEST(FuncPtrTemplate)
{
	const char* src = R"(
template <typename T>
T add1(T x) {
	return x;
}

int top(int x, int y) {
	return add1<int> > 5;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
fprintf(stderr, "%s\n", top->DebugString(0).c_str());
	EXPECT_EQ(CountNodes(top), 3);


	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), ">");

	auto left_ref = compiler::AsA<compiler::DeclRef*>(top_bop->GetLeft());
	ASSERT(left_ref != nullptr);
	EXPECT_EQ(left_ref->GetTemplateArgs().len(), 1);
	
		auto left_func = compiler::AsA<compiler::FuncDecl*>(left_ref->GetRef());
	ASSERT(left_func != nullptr);
	EXPECT_EQ(left_func->GetName(), "add1");
}


DECLARE_TEST(VarDecl)
{
	const char* src = R"(
int top(int x, int y) {
	int ret = 0;
	ret = x + y;
	return ret;
}
	)";

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 3);

	EXPECT_NOT_NULL(compiler::AsA<compiler::VarDecl*>(body[0]));
	EXPECT_EQ(compiler::AsA<compiler::VarDecl*>(body[0])->GetInitType(), 
			  compiler::VarDeclInitType_Equals);
	EXPECT_NOT_NULL(compiler::AsA<compiler::Expr*>(body[1]));
	EXPECT_NOT_NULL(compiler::AsA<compiler::ReturnStmt*>(body[2]));
}

DECLARE_TEST(StructDecl)
{
	const char* src = R"(
struct top {
	int a = 3;
	int b = 1 + a / 2;
};
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::StructDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_EQ(top_decl->GetInnerDecls().len(), 2);
	EXPECT_EQ(top_decl->GetTemplateParams().len(), 0);
}


DECLARE_TEST(TemplateStruct)
{
	const char* src = R"(
template<typename T>
struct top {
	T a = 3;
	int b = 1 + a / 2;
};
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::StructDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_EQ(top_decl->GetInnerDecls().len(), 2);
	EXPECT_EQ(top_decl->GetTemplateParams().len(), 1);
	EXPECT_EQ(top_decl->GetTemplateParams()[0]->GetKind(), 
			  compiler::TemplateParamKind_Type);
}

DECLARE_TEST(GlobalDecl)
{
	const char* src = R"(
int top = 100;
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::VarDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_NOT_NULL(compiler::AsA<compiler::IntType*>(top_decl->GetType()));
}

DECLARE_TEST(FunctionProto)
{
	const char* src = R"(
int top();
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, ">>> %s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::FuncDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_NOT_NULL(compiler::AsA<compiler::IntType*>(top_decl->GetReturnType()));
	EXPECT_EQ(top_decl->GetParameters().len(), 0);
	EXPECT_EQ(top_decl->GetTemplateParams().len(), 0);
}

DECLARE_TEST(StructParam)
{
	const char* src = R"(
struct Foo {
	int a = 3;
	int b = 1 + a / 2;
};
int top(Foo v) {
	return v.a + v.b;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
fprintf(stderr, "%s\n", top->DebugString(0).c_str());
	EXPECT_EQ(CountNodes(top), 5);


	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), "+");
}


DECLARE_TEST(TemplateStructParam)
{
	const char* src = R"(
template<typename T>
struct Foo {
	T a = 3;
	T b = 1 + a / 2;
};
int top(Foo<int> v) {
	return v.a;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
fprintf(stderr, "%s\n", top->DebugString(0).c_str());
	EXPECT_EQ(CountNodes(top), 2);

	auto top_member = compiler::AsA<compiler::MemberExpr*>(top);
	ASSERT(top_member != nullptr);

	auto base_ref = compiler::AsA<compiler::DeclRef*>(top_member->GetBase());
	ASSERT(base_ref != nullptr);

	auto param = compiler::AsA<compiler::VarDecl*>(base_ref->GetRef());
	ASSERT(param != nullptr);

	fprintf(stderr, "Param %s type %s\n", 
		param->GetName().c_str(),
		param->GetType()->DebugString(0).c_str());

	// Test instantiation
	EXPECT_NOT_NULL(compiler::AsA<compiler::IntType*>(param->GetType()));
}

DECLARE_TEST(CppStyleCast)
{
	const char* src = R"(

int top(int x) {
	return int(x+1);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 4);

	fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_call = compiler::AsA<compiler::CtorCall*>(top);
	ASSERT(top_call != nullptr);
}

DECLARE_TEST(CppStyleCastUserType)
{
	const char* src = R"(

struct Foo {

};

int doit(int x) {
	return x;
}

int top(int x) {
	return Foo(x+1) * doit(x);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 7);

	fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), "*");

	auto top_ctor = compiler::AsA<compiler::CtorCall*>(top_bop->GetLeft());
	ASSERT(top_ctor != nullptr);

	auto top_call = compiler::AsA<compiler::FuncCall*>(top_bop->GetRight());
	ASSERT(top_call != nullptr);
}

DECLARE_TEST(CppStyleCastTemplatedUserType)
{
	const char* src = R"(

template<typename T>
struct Foo {

};

int top(int x) {
	return Foo<int>(1);
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 2);

	fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_ctor = compiler::AsA<compiler::CtorCall*>(top);
	ASSERT(top_ctor != nullptr);
}

DECLARE_TEST(CppStyleCastTemplatedUserTypeWithTemplatedFunction)
{
	const char* src = R"(

template<typename T>
struct Foo {

};

template<typename T>
int bar() {
	return 0;	
}

int top(int x) {
	return bar<int>();
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 1);

	fprintf(stderr, "--- top ---\n%s\n", top->DebugString(0).c_str());

	auto top_call = compiler::AsA<compiler::FuncCall*>(top);
	ASSERT(top_call != nullptr);
}

// TODO: More template arg combinations

DECLARE_TEST(TemplateFunctionIntParam)
{
	const char* src = R"(

template<int N>
int bar() {
	return N;
}

int top(int x) {
	return bar<2>();
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
	EXPECT_EQ(CountNodes(top), 1);

	fprintf(stderr, "--- top ---\n%s\n", top->DebugString(0).c_str());

	auto top_call = compiler::AsA<compiler::FuncCall*>(top);
	ASSERT(top_call != nullptr);
}

DECLARE_TEST(FuncPtrTemplateFunctionIntParam)
{
	const char* src = R"(
template<int N>
int bar() {
	return N;
}

int top(int x) {
	return bar<2> > 2;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
fprintf(stderr, "%s\n", top->DebugString(0).c_str());
	EXPECT_EQ(CountNodes(top), 3);


	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), ">");

	auto left_ref = compiler::AsA<compiler::DeclRef*>(top_bop->GetLeft());
	ASSERT(left_ref != nullptr);
	EXPECT_EQ(left_ref->GetTemplateArgs().len(), 1);
	
		auto left_func = compiler::AsA<compiler::FuncDecl*>(left_ref->GetRef());
	ASSERT(left_func != nullptr);
	EXPECT_EQ(left_func->GetName(), "bar");
}


DECLARE_TEST(FuncPtrTemplateFunctionIntParam2)
{
	// Doesn't compile on clang!
	const char* src = R"(
template<int N, typename T>
T bar() {
	return N;
}

int top(int x) {
	return bar<2, int> > 2;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
fprintf(stderr, "%s\n", top->DebugString(0).c_str());
	EXPECT_EQ(CountNodes(top), 3);


	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), ">");

	auto left_ref = compiler::AsA<compiler::DeclRef*>(top_bop->GetLeft());
	ASSERT(left_ref != nullptr);
	EXPECT_EQ(left_ref->GetTemplateArgs().len(), 2);
	
		auto left_func = compiler::AsA<compiler::FuncDecl*>(left_ref->GetRef());
	ASSERT(left_func != nullptr);
	EXPECT_EQ(left_func->GetName(), "bar");
}

DECLARE_TEST(FuncPtrTemplateFunctionIntParam3)
{
	// Doesn't compile on clang!
	const char* src = R"(
template<typename D, int N, typename T>
T bar(D x) {
	return x+N;
}

int top(int x) {
	return bar<int, 2, int> > 2;
}
	)";

	compiler::Expr* top = TestSingleFunctionSingleReturn(src);
fprintf(stderr, "%s\n", top->DebugString(0).c_str());
	EXPECT_EQ(CountNodes(top), 3);


	auto top_bop = compiler::AsA<compiler::BinaryOp*>(top);
	ASSERT(top_bop != nullptr);
	EXPECT_EQ(top_bop->GetOp(), ">");

	auto left_ref = compiler::AsA<compiler::DeclRef*>(top_bop->GetLeft());
	ASSERT(left_ref != nullptr);
	EXPECT_EQ(left_ref->GetTemplateArgs().len(), 3);
	
		auto left_func = compiler::AsA<compiler::FuncDecl*>(left_ref->GetRef());
	ASSERT(left_func != nullptr);
	EXPECT_EQ(left_func->GetName(), "bar");
}

DECLARE_TEST(VarDeclCtorEmpty)
{
	const char* src = R"(
struct Foo {

};
int top(int x, int y) {
	Foo r();
	return x;
}
	)";

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 2);

	EXPECT_NOT_NULL(compiler::AsA<compiler::VarDecl*>(body[0]));
	EXPECT_NOT_NULL(compiler::AsA<compiler::ReturnStmt*>(body[1]));
}

DECLARE_TEST(GlobalCtor)
{
	const char* src = R"(
struct Foo {

};
Foo top;
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::VarDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_EQ(top_decl->GetInitType(), compiler::VarDeclInitType_None);
	EXPECT_NOT_NULL(compiler::AsA<compiler::StructDecl*>(top_decl->GetType()));
}


DECLARE_TEST(PreferFunctionOverGlobalDecl)
{
	const char* src = R"(
struct Foo {

};
Foo top() {

}
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::FuncDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_NOT_NULL(compiler::AsA<compiler::StructDecl*>(top_decl->GetReturnType()));
}


DECLARE_TEST(PreferFunctionProtoOverGlobalDecl)
{
	const char* src = R"(
struct Foo {

};
Foo top();
	)";

	compiler::Decl* top = ParseAndGetTop(src);

fprintf(stderr, "%s\n", top->DebugString(0).c_str());

	auto top_decl = compiler::AsA<compiler::FuncDecl*>(top);
	ASSERT(top_decl != nullptr);
	EXPECT_NOT_NULL(compiler::AsA<compiler::StructDecl*>(top_decl->GetReturnType()));
}

DECLARE_TEST(CtorCallAsStmtEmpty)
{
	const char* src = R"(
struct Foo {

};
int top(int x, int y) {
	Foo();
	return 0;
}
	)";

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 2);

	EXPECT_NOT_NULL(compiler::AsA<compiler::CtorCall*>(body[0]));
	EXPECT_NOT_NULL(compiler::AsA<compiler::ReturnStmt*>(body[1]));
}

DECLARE_TEST(CtorCallAsStmt)
{
	const char* src = R"(
struct Foo {
};
int top(int x, int y) {
	Foo(x);
	return 0;
}
	)";

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 2);

	EXPECT_NOT_NULL(compiler::AsA<compiler::CtorCall*>(body[0]));
	EXPECT_NOT_NULL(compiler::AsA<compiler::ReturnStmt*>(body[1]));
}

DECLARE_TEST(VarDeclCtor)
{
	const char* src = R"(
struct Foo {
};
int top(int x, int y) {
	Foo a(x, y);
	return 0;
}
	)";

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 2);

	EXPECT_NOT_NULL(compiler::AsA<compiler::VarDecl*>(body[0]));
	EXPECT_EQ(compiler::AsA<compiler::VarDecl*>(body[0])->GetInitType(),
			  compiler::VarDeclInitType_Ctor);
	EXPECT_NOT_NULL(compiler::AsA<compiler::ReturnStmt*>(body[1]));
}

DECLARE_TEST(VarDeclInitList)
{
	const char* src = R"(
struct Foo {

};
int top(int x, int y) {
	Foo a {x, y};
	return 0;
}
	)";

	vector<compiler::Stmt*> body = ParseAndGetTopBody(src);
	assert(body.len() == 2);

	EXPECT_NOT_NULL(compiler::AsA<compiler::VarDecl*>(body[0]));
	EXPECT_EQ(compiler::AsA<compiler::VarDecl*>(body[0])->GetInitType(),
			  compiler::VarDeclInitType_InitList);
	EXPECT_NOT_NULL(compiler::AsA<compiler::ReturnStmt*>(body[1]));
}

// Template int params (remove comma operator stuff)

// TODO: Check that proto decl is linked to main decl

// Typedef

// Template instantiation


}  // namespace
}  // namespace stacklang

int main(int argc, char **argv) {
	if(argc == 2) {
		stacklang::vector<stacklang::TestSpec> filtered_tests;
		for(stacklang::TestSpec spec : stacklang::sTests) {
			if(spec.name == argv[1]) {
				filtered_tests.push_back(spec);
			}
		}
		stacklang::sTests = filtered_tests;
	}

	for(stacklang::TestSpec spec : stacklang::sTests) {
		fprintf(stderr, "--- %s ---\n", spec.name.c_str());
		spec.func();
	}
	fprintf(stderr, "\n");
	for(stacklang::TestSpec spec : stacklang::sTests) {
		if(stacklang::sTestsPassed.at(spec.name)) {
			fprintf(stderr, "PASSED %s\n", spec.name.c_str());
		} else {
			fprintf(stderr, "* FAILED %s\n", spec.name.c_str());
		}
	}
	return 0;
}