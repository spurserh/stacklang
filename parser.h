#ifndef PARSER_H
#define PARSER_H

#include "string.h"
#include "vector.h"
#include "set.h"
#include "utils.h"
#include "scanner.h"

// STL
#include <initializer_list>
#include <assert.h>
#include <string>

namespace stacklang {
namespace compiler {

struct Identifier {
	vector<string> parts;
	bool global = false; // starts with ::
	LocationRef loc;

	string DebugString()const {
		string ret;
		if(global) {
			ret = ret + "::";
		}
		for(int64 p=0;p<parts.len();++p) {
			ret = ret + parts[p];
			if(p != (parts.len()-1)) {
				ret = ret + "::";
			}
		}
		return ret;
	}
};

bool IsDigit(char c) {
	return (c >= '0') && (c <= '9');
}

bool IsLetter(char c) {
	return (c >= 'a') && (c <= 'z') &&
		   (c >= 'A') && (c <= 'Z');
}

void IsValidID(string id, LocationRef loc) throws(Status) {
	if(id.len() <= 0) {
		return;
	}
	if(IsDigit(id[0])) {
		return;
	}
	for(int64 i=0;i<id.len();++i) {
		char c = id[i];
		if(!(IsDigit(c) || IsLetter(c) || (c == '_'))) {
			return;
		}
	}
	throw Status{.message = string("Invalid identifier: ") + id};
}

class Type {
public:
	virtual string DebugString()const = 0;
};

class VoidType : public Type {
public:
	string DebugString()const override {
		return "void";
	}
};

class IntType : public Type {
public:
	string DebugString()const override {
		return "int";
	}
};

class Value {
public:
	virtual string DebugString()const = 0;
	virtual Type* GetType()const = 0;
};

class VoidValue : public Value {
public:
	string DebugString()const override {
		return "void";
	}
	Type* GetType()const override {
		return new VoidType;
	}
};

class IntegerValue : public Value {
public:
	IntegerValue(int64 value) : value_(value) { }
	string DebugString()const override {

		return string("int(") + std::to_string(value_).c_str() + ")";
	}
	Type* GetType()const override {
		return new IntType;
	}
private:
	int64 value_;
};


class Decl {
public:
  Decl(string name, LocationRef loc) : name_(name), loc_(loc) {
 	IsValidID(name, loc) throws();
  }
  virtual ~Decl() {} ;
  string GetName()const {
  	return name_;
  }
  LocationRef GetLoc() const {
  	return loc_;
  }
  virtual string DebugString()const = 0;
private:
  string name_;
  LocationRef loc_;
};


class Stmt {
public:
	Stmt(LocationRef loc) : loc_(loc) { }
	virtual string DebugString()const = 0;
private:
	LocationRef loc_;
};


class Expr : public Stmt {
public:
	Expr(LocationRef loc) : Stmt(loc) { }
private:
};

class Literal : public Expr {
public:
	Literal(Value* value, LocationRef loc) : 
		Expr(loc), value_(value) {

	}
  	string DebugString()const {
  		return string("Literal (") + value_->DebugString() + ")";
  	}
	Value* GetValue()const {
		return value_;
	}
private:
	Value* value_;
};


class DeclRef : public Expr {
public:
	DeclRef(Decl* ref, LocationRef loc) : 
		Expr(loc), ref_(ref) {

	}
  	string DebugString()const {
  		return string("Ref (") + ref_->DebugString() + ")";
  	}
	Decl* GetValue()const {
		return ref_;
	}
private:
	Decl* ref_;
};


class BinaryOp : public Expr {
public:
	BinaryOp(string op, Expr* left, Expr* right, LocationRef loc) : 
		Expr(loc), op_(op), left_(left), right_(right) {
	}
	string DebugString() const {
		return left_->DebugString() + " + " + right_->DebugString();
	}
	string GetOp()const {
		return op_;
	}
	Expr* GetLeft()const {
		return left_;
	}
	Expr* GetRight()const {
		return right_;
	}
private:
	string op_;
	Expr* left_;
	Expr* right_;
};

class ReturnStmt : public Stmt {
public:
	ReturnStmt(Expr* value, LocationRef loc) 
		: Stmt(loc), value_(value) {
	}
	string DebugString()const {
		return string("Return(") + value_->DebugString() + ")";
	}
	Expr* GetValue()const {
		return value_;
	}
private:
	Expr* value_;
};


class VarDecl : public Decl {
public:
	VarDecl(string name, LocationRef loc, Type* type) 
	  : Decl(name, loc), type_(type) {
	}
	~VarDecl() override {}
    string DebugString()const override {
  	  return string("VarDecl(") + GetName() + " : " + type_->DebugString() + ")";
    }
private:
	Type* type_;
};

class FuncDecl : public Decl {
public:
	FuncDecl(string name, Type* return_type, vector<VarDecl*> parameters, vector<Stmt*> body, LocationRef loc) 
	  : Decl(name, loc), return_type_(return_type), parameters_(parameters), body_(body) {
	}
	~FuncDecl() override {}
	string DebugString()const override {
		string params;
		for(VarDecl* param : parameters_) {
			params = params + param->DebugString();
			params = params + ", ";
		}
	  	string ret = string("FuncDecl ") + GetName() + "(" + params + ") -> " + return_type_->DebugString();
	  	ret = ret + "{\n";
	  	for(Stmt* stmt : body_) {
	  		ret = ret + stmt->DebugString() + "\n";
	  	}
	  	ret = ret + "}\n";
	  	return ret;
	}
	vector<VarDecl*> GetParameters()const {
		return parameters_;
	}
	Type* GetReturnType()const {
		return return_type_;
	}
private:
	Type* return_type_;
	vector<VarDecl*> parameters_;
	vector<Stmt*> body_;
};


class Namespace {
public:
	Namespace() : name_("") {}
	Namespace(string name, LocationRef loc) : name_(name), loc_(loc) throws(Status) {
		IsValidID(name, loc) throws();
	}
  	string DebugString()const {
  		string ret = string("Namespace (") + name_ + ") {\n";
  		for(Decl* decl : decls_) {
  			ret = ret + decl->DebugString() + "\n";
  		}
  		ret = ret + "}\n";

  		return ret;
  	}
  	string GetName() {
  		return name_;
  	}
  	void AddNested(Namespace nested) {
  		nested_.push_back(nested);
  	}
  	void AddDecl(Decl* decl) {
  		decls_.push_back(decl);
  	}
private:
 	string name_;
 	LocationRef loc_;
 	vector<Namespace> nested_;
 	vector<Decl*> decls_;
 	// TODO: throws
};

struct Token {
	string content;
	LocationRef loc;
};

Type* ParseType(vector<Token>& tokens) throws(Status) {
	Token next_token = tokens[0];
	if(next_token.content == "void") {
		tokens.pop_front();
		return new VoidType;
	} else if(next_token.content == "int") {
		tokens.pop_front();
		return new IntType;
	}
	// TODO: Identifier
	throw Status{.message = string("Don't know how to translate token to type: ") + next_token.content};
}

bool PeekAndConsumeUtil(vector<Token>& tokens,
					vector<string> look_for) {
	if(tokens.len() < look_for.len()) {
		return false;
	}
	for(int64 i=0;i<look_for.len();++i) {
		if(tokens[i].content != look_for[i]) {
			return false;
		}
	}
	tokens.pop_front(look_for.len());
	return true;
}

void ConsumeOrError(vector<Token>& tokens,
					vector<string> look_for) throws(Status) {
	if(!PeekAndConsumeUtil(tokens, look_for)) {
		string message = "Expected token(s): ";
		for(string s : look_for) {
			message = message + s;
		}
		throw Status{.message = message};
	}
}

Identifier ParseIdentifier(vector<Token>& tokens) throws(Status) {
	Identifier ret;
	if(PeekAndConsumeUtil(tokens, {"::"})) {
		ret.global = true;
	}
	do {
		Token next_token = tokens.pop_front();
		IsValidID(next_token.content, next_token.loc) throws();
		ret.parts.push_back(next_token.content);
		ret.loc = next_token.loc;
	}while(PeekAndConsumeUtil(tokens, {"::"}));
	return ret;
}

struct ContextFrame {
	Namespace* in_namespace = nullptr;
	
};

struct Context {
	// Front is the top of the stack
	vector<ContextFrame> frames;
};

Decl* GetDeclByIdentifier(Context&context, Identifier id) throws(Status) {

	throw Status{.message = string("Couldn't find identifier ") + id.DebugString()};
}

VarDecl* ParseVarDecl(vector<Token>& tokens) throws(Status) {
	Type* type = ParseType(tokens);
	Token name_tok = tokens.pop_front();
	
	if(PeekAndConsumeUtil(tokens, {"="})) {
		throw Status{.message="TODO: initializer"};
	}

	return new VarDecl(name_tok.content, name_tok.loc, type);
}

Expr* ParseExpr(Context& context,
				vector<Token>& tokens) {
	fprintf(stderr, "ParseExpr next %s\n", tokens[0].content.c_str());

	Expr* leaf_parsed = nullptr;

	// Integer literal
	try{
		long long integer_literal = std::stoll(tokens[0].content.c_str()) throws();
		assert(leaf_parsed == nullptr);
		tokens.pop_front();
		leaf_parsed = new Literal(new IntegerValue(integer_literal), tokens[0].loc);
	} catch(std::invalid_argument invalid) {
	} catch(std::out_of_range invalid) {
	}

	// Decl for identifier
	bool parsed_identifier = false;
	Identifier id;
	try {
		id = ParseIdentifier(tokens) throws();
		assert(leaf_parsed == nullptr);
		parsed_identifier = true;
	} catch(Status status) {
	}

	if(parsed_identifier) {
		Decl* decl = GetDeclByIdentifier(context, id) throws();
		leaf_parsed = new DeclRef(decl, decl->GetLoc());
	}

	// TODO: Operator precedence
	const set<string> infix_operators = {"+", "-", "<=", "/"};

	if(leaf_parsed && infix_operators.contains(tokens[0].content)) {
		Token operator_token = tokens.pop_front();
		Expr* right_side = ParseExpr(context, tokens);
		return new BinaryOp(operator_token.content,
							leaf_parsed,
							right_side,
							operator_token.loc);
	}

	if(leaf_parsed != nullptr) {
		return leaf_parsed;
	}

	throw Status{.message=string("Unable to parse expr starting at ") + tokens[0].content};
}

Stmt* ParseStmt(Context& context,
				vector<Token>& tokens) {

	Token next_token = tokens[0];
	LocationRef loc = next_token.loc;

	fprintf(stderr, "ParseStmt next %s\n", next_token.content.c_str());

	Stmt* ret = nullptr;

	if(PeekAndConsumeUtil(tokens, {"return"})) {
		ret = new ReturnStmt(ParseExpr(context, tokens), loc);
	} else if(ret == nullptr) {
		ret = ParseExpr(context, tokens) throws ();
	}

	ConsumeOrError(tokens, {";"}) throws ();

	return ret;
}

// Starts from after the "return_type name(", before the parameter specs
FuncDecl* ParseFuncDecl(Context& context,
						vector<Token>& tokens,
						string name,
						Type* return_type,
						LocationRef loc) throws(Status) {
	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};
	vector<VarDecl*> parameters;

	for(bool first = true;
		!PeekAndConsume({")"});
		first = false) {

		if(!first) {
			ConsumeOrError(tokens, {","}) throws();
		}

		parameters.push_back(ParseVarDecl(tokens));
	}

	if(PeekAndConsume({";"})) {
		throw Status{.message="TODO: prototype"};
	}

	ConsumeOrError(tokens, {"{"});

	vector<Stmt*> body;

	while(!PeekAndConsume({"}"})) {
		body.push_back(ParseStmt(context, tokens));
	}

	return new FuncDecl(name, return_type, parameters, body, loc);
}

void ParseNamespaceContents(Context& context,
							vector<Token>& tokens,
							Namespace& result) throws(Status) {
	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};
fprintf(stderr, "ParseNamespaceContents %li\n", tokens.len());

	int64 debug_prev_token_count = -1;
	while(!tokens.empty()) {
fprintf(stderr, "-- ParseNamespaceContents %li\n", tokens.len());
		assert(debug_prev_token_count != tokens.len());
		debug_prev_token_count = tokens.len();

		if(PeekAndConsume({"namespace"})) {
			Token name_tok = tokens.pop_front();
			if(!PeekAndConsume({"{"})) {
				throw Status{.message="Expected { after", .loc=name_tok.loc};
			}

			Namespace nested(name_tok.content, name_tok.loc);
			context.frames.push_back(ContextFrame{.in_namespace = &nested});
			ParseNamespaceContents(context, tokens, nested);
			result.AddNested(nested);
		}
		if(PeekAndConsume({"typedef"})) {
			throw Status{.message="TODO: typedef"};
		}
		if(PeekAndConsume({"using"})) {
			throw Status{.message="TODO: using"};
		}
		if(PeekAndConsume({"class"})) {
			throw Status{.message="TODO: class"};
		}
		if(PeekAndConsume({"struct"})) {
			throw Status{.message="TODO: struct"};
		}

		bool static_specified = false;
		if(PeekAndConsume({"static"})) {
			static_specified = true;
		}

		// Parse as type
		Type* type = ParseType(tokens);
		assert(type != nullptr);

		Token name_tok = tokens.pop_front();

		if(PeekAndConsume({"("})) {
			result.AddDecl(ParseFuncDecl(context, tokens, name_tok.content, type, name_tok.loc)) throws();
			continue;
		}

		throw Status{.message="TODO: decl"};
	}
}


// Returns the anonymous namespace
Namespace Parse(vector<string> tokens_raw) throws(Status) {

	// Process line markers
	LocationRef last_marker;
	vector<Token> tokens;

	while(!tokens_raw.empty()) {
		string next_token = tokens_raw[0];
		tokens_raw.pop_front();

		if(next_token[0] == '#') {
			// TODO: Actually parse and place in last_marker
			continue;
		}

		tokens.push_back(Token{.content = next_token, .loc = last_marker});
	}

	// Anonymous
	Namespace result(/*name=*/"", /*loc=*/LocationRef{});
	Context context;
	context.frames.push_back(ContextFrame{.in_namespace = &result});
	ParseNamespaceContents(context, tokens, result);
	assert(tokens.empty());
	return result;
}

}  // compiler
}  // stacklang

#endif//PARSER_H
