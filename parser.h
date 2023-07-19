#ifndef PARSER_H
#define PARSER_H

#include "string.h"
#include "vector.h"
#include "set.h"
#include "utils.h"
#include "scanner.h"
#include "map.h"
#include "tokens.h"

// STL
#include <initializer_list>
#include <assert.h>
#include <string>
#include <sstream>

namespace stacklang {
namespace compiler {

template<typename PtrTo, typename PtrFrom>
PtrTo AsA(PtrFrom *p) {
	return dynamic_cast<PtrTo>(p);
}

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
	return ((c >= 'a') && (c <= 'z')) ||
		   ((c >= 'A') && (c <= 'Z'));
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
		if(!IsDigit(c) && !IsLetter(c) && (c != '_')) {
			throw Status{.message = string("Invalid identifier: ") + id};
		}
	}
}

string FormatIndent(int64 indent) {
	assert(indent >= 0);
	string ret;
	for(int64 i=0;i<indent;++i) {
		ret += "  ";
	}
	return ret;
}

class Decl;
class Expr;

class Type {
public:
	virtual string DebugString(int64 indent)const = 0;
};

class VoidType : public Type {
public:
	string DebugString(int64 indent)const override {
		return "void";
	}
};

class IntType : public Type {
public:
	string DebugString(int64 indent)const override {
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


class Stmt {
public:
	Stmt(LocationRef loc) : loc_(loc) { }
	virtual string DebugString(int64 indent)const = 0;
	LocationRef GetLoc()const {
		return loc_;
	}
private:
	LocationRef loc_;
};

class Decl : public Stmt {
public:
  Decl(string name, LocationRef loc) : Stmt(loc), name_(name) {
 	IsValidID(name, loc) throws();
  }
  virtual ~Decl() {} ;
  string GetName()const {
  	return name_;
  }
  virtual string DebugString(int64 indent)const = 0;
private:
  string name_;
};

enum TemplateParamKind {
	TemplateParamKind_Null=0,
	TemplateParamKind_Int=1,
	TemplateParamKind_Type=2,
};

class TemplateParam : public Decl, public Type {
public:
	TemplateParam(string name, TemplateParamKind kind, LocationRef loc)
	 : Decl(name, loc), kind_(kind) {

	 }
	 ~TemplateParam() override {}
	  string DebugString(int64 indent)const override {
	  	string ret;
	  	if(kind_ == TemplateParamKind_Int) {
	  		ret += "int";
	  	} else if (kind_ == TemplateParamKind_Type) {
	  		ret += "typename";
	  	}
	  	ret += " ";
	  	ret += GetName();
	  	return ret;
	  }
	  TemplateParamKind GetKind()const { return kind_; }
private:
	TemplateParamKind kind_ = TemplateParamKind_Null;
};


class TemplatedDecl : public Decl {
public:
  TemplatedDecl(string name, vector<TemplateParam*> template_params, LocationRef loc) throws()
  	: Decl(name, loc), template_params_(template_params) {
  }
  virtual ~TemplatedDecl() {} ;
  bool IsTemplated()const {
  	return !template_params_.empty();
  }
  vector<TemplateParam*> GetTemplateParams()const {
  	return template_params_;
  }
  virtual string DebugString(int64 indent)const = 0;

  string TemplateParamsString()const {
  	if(!IsTemplated()) {
  		return "";
  	}
  	string ret;
	 ret += "<";
	bool first = true;
	for(TemplateParam* param : GetTemplateParams()) {
		if(!first) {
			ret += ", ";
		}
		ret += param->GetName();
		first = false;
	}
	ret += ">";
	return ret;
  }
private:
  vector<TemplateParam*> template_params_;
};


class Expr : public Stmt {
public:
	Expr(LocationRef loc) : Stmt(loc) { }
	virtual vector<Expr*> GetOperands()const = 0;
private:
};


struct TemplateArg {
	Type* type = nullptr;
	Expr* int_value = nullptr;

	string DebugString()const {
		if(type) {
			return type->DebugString(0);
		}
		if(int_value) {
			return int_value->DebugString(0);
		}
		return "(null)";
	}
};

class DeclRef : public Expr {
public:
	DeclRef(Decl* ref, vector<TemplateArg> template_args, LocationRef loc) : 
		Expr(loc), ref_(ref), template_args_(template_args) {

	}
  	string DebugString(int64 indent)const override {
  		string ret = string("&") + ref_->GetName();
  		if(!template_args_.empty()) {
  			ret += "<<";
  			for(TemplateArg arg : template_args_) {
  				ret += arg.DebugString();
  				ret += " ";
  			}
  			ret += ">>";
  		}
  		return ret;
  	}
	Decl* GetRef()const {
		return ref_;
	}
	vector<Expr*> GetOperands()const override { return {}; }
	vector<TemplateArg> GetTemplateArgs()const {
		return template_args_;
	}
private:
	Decl* ref_;
	vector<TemplateArg> template_args_;
};


class DeclRefType : public Type {
public:
	DeclRefType(DeclRef* ref) : ref_(ref) {
	}
	string DebugString(int64 indent)const override {
		return ref_->DebugString(indent);
	}
	DeclRef* GetDeclRef()const {
		return ref_;
	}
private:
	DeclRef* ref_;
};

class Literal : public Expr {
public:
	Literal(Value* value, LocationRef loc) : 
		Expr(loc), value_(value) {

	}
  	string DebugString(int64 indent)const override {
  		return value_->DebugString();
  	}
	Value* GetValue()const {
		return value_;
	}
	vector<Expr*> GetOperands()const override { return {}; }
private:
	Value* value_;
};


class MemberExpr : public Expr {
public:
	MemberExpr(Expr* base, string member_name, bool pointer, LocationRef loc)
		: Expr(loc), base_(base), member_name_(member_name), pointer_(pointer) {

	}

	vector<Expr*> GetOperands()const override {
		return {base_};
	}

	string DebugString(int64 indent) const override {
		string ret = base_->DebugString(0);
		ret += string(pointer_ ? " -> " : " . ");
		ret += member_name_;
		return ret;
	}

	Expr* GetBase()const {
		return base_;
	}

private:
	Expr* base_;
	string member_name_;
	bool pointer_;
};

class BinaryOp;

class UnaryOp : public Expr {
public:
	// op may be "" if derived
	UnaryOp(string op, bool postfix, Expr* sub, LocationRef loc)
		: Expr(loc), op_(op), postfix_(postfix), sub_(sub) {
//		assert(!AsA<BinaryOp*>(sub));
	}
	string DebugString(int64 indent) const override {
		string ret = op_;
		if(postfix_) {
			ret += " post ";
		}
		ret += string("(") + sub_->DebugString(indent) + ")";
		return ret;
	}
	string GetOp()const {
		return op_;
	}
	Expr* GetSub()const {
		return sub_;
	}
	bool IsPostfix()const {
		return postfix_;
	}
	void SetSub(Expr* sub) {
		sub_ = sub;
	}
	vector<Expr*> GetOperands()const override {
		return {sub_};
	}
private:
	string op_;
	bool postfix_;
	Expr* sub_;
};


enum CastType {
	CastType_Null=0,
	CastType_CStyle,
	CastType_CPPStyle,
	CastType_Static,
	CastType_Dynamic,
	CastType_Const,
	CastType_Reinterpret,
};

class CastExpr : public UnaryOp {
public:
	CastExpr(CastType cast_type, Type* to_type, Expr* sub, LocationRef loc) 
	  : UnaryOp("", /*postfix=*/false, sub, loc), cast_type_(cast_type), to_type_(to_type) {

	}
	string DebugString(int64 indent) const override {
		return string("cast<") + to_type_->DebugString(indent) + ">(" + GetSub()->DebugString(indent) + ")";
	}
	CastType GetCastType() const {
		return cast_type_;
	}
	Type* GetToType() const {
		return to_type_;
	}
 private:
 	CastType cast_type_;
 	Type* to_type_;
};

// Exists to prevent precedence adjustment
class ParenExpr : public Expr {
public:
	ParenExpr(Expr* sub, LocationRef loc) : Expr(loc), sub_(sub) {

	}
	string DebugString(int64 indent) const override {
		return string("(( ") + sub_->DebugString(indent) + " ))";
	}
	vector<Expr*> GetOperands()const override {
		return {sub_};
	}
 private:
	Expr* sub_;
};

class BinaryOp : public Expr {
public:
	// Throws if precedence can't be found for op
	BinaryOp(string op, Expr* left, Expr* right, LocationRef loc) throws(Status) : 
		Expr(loc), op_(op), left_(left), right_(right) {
		AdjustPrecedence();
	}
	string DebugString(int64 indent) const override {
		return string("( ") + left_->DebugString(indent) + " " + op_ + " " + right_->DebugString(indent) + " )";
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
	void SetLeft(Expr* sub) {
		left_ = sub;
	}
	void SetRight(Expr* sub) {
		right_ = sub;
	}
	vector<Expr*> GetOperands()const override {
		return {left_, right_};
	}
private:
	string op_;
	Expr* left_;
	Expr* right_;

	// Throws if precedence can't be found for op
	// Only called from constructor, so doesn't violate mutabiilty
	void AdjustPrecedence() throws(Status) {
		BinaryOp* right_bop = AsA<BinaryOp*>(right_);
		if(right_bop == nullptr) {
			return;
		}
		map<string, int64> precedences = GetAllInfixOperatorsWithPrecedence();
		// Don't need to consider left: tree is built left to right
		int64 my_prec = precedences.at(op_);
		int64 right_prec = precedences.at(right_bop->op_);

		if(my_prec < right_prec) {
			// -- original --
			// x*(y+z)
			// ----------

			// -- goal --
			// (x*y)+z
			// ----------


			Expr* x = left_;
			Expr* y = right_bop->left_;
			Expr* z = right_bop->right_;
			BinaryOp* sub = right_bop;

			std::swap(op_, right_bop->op_);
			// x+(y*z)
			std::swap(left_, right_);
			// (y*z)+x
			sub->left_ = x;
			sub->right_ = y;
			right_ = z;
		}
	}
};

class ReturnStmt : public Stmt {
public:
	ReturnStmt(Expr* value, LocationRef loc) 
		: Stmt(loc), value_(value) {
	}
	string DebugString(int64 indent)const {
		return string("Return(") + value_->DebugString(indent) + ")";
	}
	// Can be null for void return
	Expr* GetValue()const {
		return value_;
	}
private:
	Expr* value_;
};

enum VarDeclInitType {
	VarDeclInitType_None=0,
	VarDeclInitType_Equals,
	VarDeclInitType_Ctor,
	VarDeclInitType_InitList
};

class VarDecl : public Decl {
public:
	VarDecl(string name, LocationRef loc, Type* type, 
			VarDeclInitType init_type,
			vector<Expr*> init_params) 
	  : Decl(name, loc), type_(type), 
	  	init_type_(init_type), init_params_(init_params) {
	 	assert(!(init_type_ == VarDeclInitType_Equals && 
	 			 init_params.len() != 1));
	}
	~VarDecl() override {}
    string DebugString(int64 indent)const override {
      // TODO: Temp
	  std::ostringstream stream;
	  stream << this;
      string ptr = stream.str().c_str();

      string ret = string("VarDecl ") + ptr + " (" 
  	  	+ GetName() + " : " + type_->DebugString(indent) + ")";

  	  if(init_type_ == VarDeclInitType_Equals) {
  	  	ret += string(" = ") + init_params_[0]->DebugString(0);
  	  } else if(init_type_ == VarDeclInitType_Ctor) {
  	  	ret += string("(");
  	  	for(Expr* param : init_params_) {
  	  		ret += param->DebugString(0).c_str();
  	  		ret += " ";
  	  	}
  	  	ret += string(")");
  	  }

  	  return ret;
    }
    Type* GetType()const {
    	return type_;
    }
    VarDeclInitType GetInitType()const {
    	return init_type_;
    }
    vector<Expr*> GetInitParams()const {
    	return init_params_;
    }
private:
	Type* type_ = nullptr;
	VarDeclInitType init_type_ = VarDeclInitType_None;
	vector<Expr*> init_params_;
};

class FuncDecl : public TemplatedDecl {
public:
	FuncDecl(string name,
			 vector<TemplateParam*> template_params,
			 Type* return_type,
			 vector<VarDecl*> parameters,
			 bool is_prototype,
			 vector<Stmt*> body,
			 LocationRef loc) 
	  : TemplatedDecl(name, template_params, loc),
	  	return_type_(return_type),
	  	parameters_(parameters),
	  	is_prototype_(is_prototype),
	  	body_(body) {
	}
	~FuncDecl() override {}
	string DebugString(int64 indent)const override {
		string params;
		for(VarDecl* param : parameters_) {
			params = params + param->DebugString(indent);
			params = params + ", ";
		}
	  	string ret = string("FuncDecl ") + GetName() + TemplateParamsString();
	  	ret += string("(") + params + ") -> " + return_type_->DebugString(indent);
	  	ret = ret + "{\n";
	  	for(Stmt* stmt : body_) {
	  		ret = ret + FormatIndent(indent) + stmt->DebugString(indent+1) + "\n";
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
	vector<Stmt*> GetBody() const{
		return body_;
	}
	void SetBody(vector<Stmt*> body) {
		body_ = body;
	}
private:
	Type* return_type_;
	vector<VarDecl*> parameters_;
	bool is_prototype_;
	vector<Stmt*> body_;
};

class StructDecl : public TemplatedDecl, public Type {
public:
	StructDecl(string name,
			   bool declared_class,
			   vector<TemplateParam*> template_params,
			   vector<Decl*> inner_decls,
			   LocationRef ref)
	  : TemplatedDecl(name, template_params, ref),
	    inner_decls_(inner_decls),
	    declared_class_(declared_class) {
	}
	vector<Decl*> GetInnerDecls()const {
		return inner_decls_;
	}
	string DebugString(int64 indent)const override {
		string ret = declared_class_ ? "class " : "struct ";
		ret += GetName() + " ";
		ret += TemplateParamsString();
		ret += string("\n") + FormatIndent(indent) + "{\n";
		bool first = true;
		for(Decl* decl : inner_decls_) {
			ret += FormatIndent(indent) + decl->DebugString(indent+1) + "\n";
		}
		ret += FormatIndent(indent) + "}";
		return ret;
	}
private:
	vector<Decl*> inner_decls_;
	bool declared_class_;
};

class TypedefDecl : public Decl, public Type {
public:
	TypedefDecl(string name, Type* base, LocationRef loc) 
		: Decl(name, loc), base_(base) {
	}
	~TypedefDecl() {
	}

	Type* GetBase()const {
		return base_;
	}

	string DebugString(int64 indent)const override {
		return string("typedef ") + GetName() + ": " + base_->DebugString(indent);
	}
private:
	Type* base_ = nullptr;
};

class UsingDecl : public TemplatedDecl, public Type {
public:
	UsingDecl(string name,
			  Type* base,
			  LocationRef loc,
			  vector<TemplateParam*> template_params = {}) 
		: TemplatedDecl(name, template_params, loc), base_(base) {
	}
	~UsingDecl() {
	}

	Type* GetBase()const {
		return base_;
	}

	string DebugString(int64 indent)const override {
		return string("using(") + GetName() + "): " + base_->DebugString(indent);
	}
private:
	Type* base_ = nullptr;
};

class UsingAliasDecl : public UsingDecl {
public:
	UsingAliasDecl(string name, 
				   Type* base, 
				   vector<TemplateParam*> template_params, 
				   LocationRef loc) 
		: UsingDecl(name, base, loc, template_params) {
	}
	~UsingAliasDecl() {
	}

	string DebugString(int64 indent)const override {
		return TemplateParamsString() + 
				string("using(") + GetName() + ") =  " +
				GetBase()->DebugString(indent);
	}
};

class Namespace {
public:
	Namespace() : name_("") {}
	Namespace(string name, LocationRef loc) : name_(name), loc_(loc) throws(Status) {
		IsValidID(name, loc) throws();
	}
  	string DebugString(int64 indent=0)const {
  		string ret = string("Namespace (") + name_ + ") {\n";
  		for(Decl* decl : decls_) {
  			ret = ret + FormatIndent(indent) + decl->DebugString(indent+1) + "\n";
  		}
  		ret = ret + "}\n";

  		return ret;
  	}
  	string GetName() {
  		return name_;
  	}
  	void AddNested(Namespace* nested) {
  		// TODO: Throw on duplicate
  		nested_.push_back(nested);
  	}
  	void AddDecl(Decl* decl) {
  		// TODO: Throw on duplicate
  		decls_.push_back(decl);
  	}
  	string GetName()const {
  		return name_;
  	}
  	vector<Decl*> GetDecls() const {
  		return decls_;
  	}
private:
 	string name_;
 	LocationRef loc_;
 	vector<Namespace*> nested_;
 	vector<Decl*> decls_;
};

class FuncCall : public Expr {
public:
	FuncCall(DeclRef* callee, vector<Expr*> args, LocationRef loc) 
		: Expr(loc), callee_(callee), args_(args) {
	}
	string DebugString(int64 indent) const override {
		string ret = string("call(") + callee_->DebugString(indent) + ": ";
		bool first = true;
		for(Expr* arg : args_) {
			if(!first) {
				ret += ",, ";
			}
			ret += arg->DebugString(indent);
			first = false;
		}
		ret += ")";
		return ret;
	}
	vector<Expr*> GetOperands()const override {
		return args_;
	}
	vector<Expr*> GetArgs()const {
		return args_;
	}
	DeclRef* GetCallee() const {
		return callee_;
	}
 private:
 	DeclRef* callee_;
	vector<Expr*> args_;
};

class CtorCall : public Expr {
public:
	CtorCall(Type* type, vector<Expr*> args, LocationRef loc) 
		: Expr(loc), type_(type), args_(args) {
	}
	string DebugString(int64 indent) const override {
		string ret = string("ctor(") + type_->DebugString(indent) + ": ";
		bool first = true;
		for(Expr* arg : args_) {
			if(!first) {
				ret += ",, ";
			}
			ret += arg->DebugString(indent);
			first = false;
		}
		ret += ")";
		return ret;
	}
	vector<Expr*> GetOperands()const override {
		return args_;
	}
	vector<Expr*> GetArgs()const {
		return args_;
	}
	Type* GetType() const {
		return type_;
	}
 private:
 	Type* type_;
	vector<Expr*> args_;
};

struct Token {
	string content;
	LocationRef loc;
};


struct ContextFrame {
	Namespace* in_namespace = nullptr;
	Namespace* top_namespace = nullptr;
	map<string, Decl*> decls;
};

struct Context {
	// Front is the top of the stack
	vector<ContextFrame> frames;

	void PushFrame() {
		ContextFrame new_frame = frames.front();
		frames.push_front(new_frame);
	}
	void PopFrame() {
		frames.pop_front();
	}
	void AddDecl(Decl* decl) throws(Status) {
		ContextFrame top = frames.front();
		if(top.decls.contains(decl->GetName())) {
			throw Status{.message = string("Duplicate declaration ") + decl->GetName()};
		}
		top.decls.set(decl->GetName(), decl);
		frames.set(0, top);
	}

  	void RemoveDecl(Decl* decl) {
		ContextFrame top = frames.front();
		if(!top.decls.contains(decl->GetName())) {
			throw Status{.message = string("Declaration does not exist to remove ") + decl->GetName()};
		}
		top.decls.remove(decl->GetName());
		frames.set(0, top);
  	}
};


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

bool PeekForAnyUtil(vector<Token>& tokens, set<string> look_for) {
	if(tokens.len() < 1) {
		throw Status{.message="No tokens to consume"};
	}
	string next = tokens[0].content;
	bool found = false;
	for(string s : look_for) {
		if(s == next) {
			found = true;
			break;
		}
	}
	return found;
}

// Throws if none fouond
Token ConsumeOneOfOrError(vector<Token>& tokens,
					set<string> look_for) throws() {

	if(!PeekForAnyUtil(tokens, look_for)) {
		string message = "Expected one of: ";
		for(string s : look_for) {
			message += s + " ";
		}
		throw Status{.message = message};
	}

	Token next = tokens.pop_front(1);
	return next;
}

void ConsumeOrError(vector<Token>& tokens,
					vector<string> look_for) throws(Status) {
	if(!PeekAndConsumeUtil(tokens, look_for)) {
		string message = string("Got token ") + tokens[0].content + " Expected token(s): ";
		for(string s : look_for) {
			message = message + s;
		}
		throw Status{.message = message};
	}
}

Expr* ParseExpr(Context& context,
				vector<Token>& tokens,
				set<string> disallow_infixes);
DeclRef* ParseDeclRef(Context& context,
				vector<Token>& tokens) throws(Status);

vector<Expr*> ParseCommaSeparatedArguments(Context& context,
											vector<Token>& tokens,
											string terminator);

// Throws on failure
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

Identifier ConsumeIdentifierFromSingleToken(vector<Token>& tokens) throws(Status) {
	Token name_tok = tokens.pop_front();
	IsValidID(name_tok.content, name_tok.loc) throws();
	return {.global = false, .parts = {name_tok.content}, .loc = name_tok.loc};
}

Decl* GetDeclByIdentifier(Namespace* in_namespace, Identifier id) throws(Status) {
	
}

Decl* GetDeclByIdentifier(Context& context, Identifier id) throws(Status) {
	if(id.global) {
		throw Status{.message = "TODO: Get global identifiers"};
	}
	assert(id.parts.len() > 0);
	if(id.parts.len() > 1) {
		throw Status{.message = "TODO: Get qualified identifiers"};
	}

	const string name = id.parts[0];

	// Search from the top
	for(ContextFrame frame : context.frames) {
		if(frame.decls.contains(name)) {
			return frame.decls.at(name);
		}
	}

	// TODO: namespaces above
fprintf(stderr, "Couldn't find identifier %s\n", id.DebugString().c_str());
	throw Status{.message = string("Couldn't find identifier ") + id.DebugString()};
}

// Returns nullptr on failure when throw_on_fail = false
// Only consumes tokens on success
Type* ParseType(Context& context, vector<Token>& tokens, bool throw_on_fail=true) throws(Status) {
	try {
		const vector<Token> prev_tokens = tokens;
		auto prev_tokens_guard = MakeLambdaGuard(
			[&tokens, prev_tokens]() {
				tokens = prev_tokens;
			}
		);

		Token next_token = tokens[0];
		if(next_token.content == "void") {
			tokens.pop_front();
			prev_tokens_guard.deactivate();
			return new VoidType;
		} else if(next_token.content == "int") {
			tokens.pop_front();
			prev_tokens_guard.deactivate();
			return new IntType;
		}
		DeclRef* decl = nullptr;
		try {
//			Identifier id = ParseIdentifier(tokens) throws();
//			decl = GetDeclByIdentifier(context, id) throws();
			decl = ParseDeclRef(context, tokens) throws ();
fprintf(stderr, "In ParseType: decl %s\n", decl ? decl->DebugString(0).c_str() : "(null)");
		} catch(Status status) {
			throw status;
		}

		if(decl) {
			if(auto param = AsA<TemplateParam*>(decl->GetRef())) {
				if(param->GetKind() != TemplateParamKind_Type) {
					throw Status{.message = "Only typenames template parameters can be used as types"};
				}

				prev_tokens_guard.deactivate();
				return param;
			}
			if(auto type = AsA<Type*>(decl->GetRef())) {
				prev_tokens_guard.deactivate();
				return type;
			}
			auto func_decl = AsA<FuncDecl*>(decl->GetRef());
			auto templated_decl = AsA<TemplatedDecl*>(decl->GetRef());
			if(templated_decl && !func_decl) {
	fprintf(stderr, "!!! TODO: TemplatedDecl\n");
				exit(1);
			}

			string message = string("Decl can't be interpreted as type: ") + decl->DebugString(0);
fprintf(stderr, "LOG: %s\n", message.c_str());
			throw Status{.message = message};
		}

		throw Status{.message = string("Don't know how to translate token to type: ") + next_token.content};
	} catch (Status status) {

		if(throw_on_fail) {
			throw status;
		} else {
			return nullptr;
		}
	}
}


// If there's no <, then returns empty without consuming input
vector<TemplateParam*> ParseTemplateParams(Context& context,
										  vector<Token>& tokens) {
	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};

	if(!PeekAndConsume({"<"})) {
		return {};
	}
	vector<TemplateParam*> template_params;
	for(bool first = true;
		!PeekAndConsume({">"});
		first = false) {

		if(!first) {
			ConsumeOrError(tokens, {","}) throws();
		}

		Token kind_tok = ConsumeOneOfOrError(tokens, {"int", "typename"}) throws();
		string kind_word = kind_tok.content;

		TemplateParamKind kind = TemplateParamKind_Null;

		if(kind_word == "int") {
			kind = TemplateParamKind_Int;
		} else if(kind_word == "typename") {
			kind = TemplateParamKind_Type;
		}

		Token name_tok = tokens.pop_front();
		string name = name_tok.content;
		LocationRef loc = name_tok.loc;

		auto* decl = new TemplateParam(name, kind, loc);

		template_params.push_back(decl);
		context.AddDecl(decl);
	}
	return template_params;
}

vector<TemplateArg> ParseTemplateArgs(Context& context,
									  vector<Token>& tokens,
									  vector<TemplateParam*> template_params) throws() {
	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};

	if(template_params.empty()) {
		return {};
	}

	ConsumeOrError(tokens, {"<"}) throws();

	vector<TemplateArg> ret;

	bool first = true;
	for(TemplateParam* param : template_params) {

		if(!first) {
			ConsumeOrError(tokens, {","}) throws();
		}

		if(param->GetKind() == TemplateParamKind_Type) {
fprintf(stderr, "---- Parse type TemplateArg ---\n");
			ret.push_back(TemplateArg{.type = ParseType(context, tokens)}) throws();
		} else if(param->GetKind() == TemplateParamKind_Int) {
fprintf(stderr, "---- Parse int TemplateArg ---\n");
			ret.push_back(TemplateArg{.int_value = ParseExpr(context, tokens, /*disallow_infix=*/{",", ">"})}) throws();
		} else {
			// TODO: Parse args
			// TODO: Unpack commas becomes annoying here..
			throw Status{.message="Don't know how to handle template param kind"};
		}

		first = false;
	}

	ConsumeOrError(tokens, {">"}) throws();

	return ret;
}


// Throws on error
// Only consumes tokens on success
// param_mode disallows ctor, init list
// Does not consume the ;
VarDecl* ParseVarDecl(Context& context,
				vector<Token>& tokens,
				Identifier id,
				vector<TemplateParam*> template_params,
				Type* type,
				bool static_specified,
				bool param_mode=false) throws() {
	assert(id.parts.len() > 0);
	if(id.global || id.parts.len() > 1) {
		throw Status{.message="VarDecl can't have qualified name"};
	}

	string name = id.parts[0];

	vector<Token> prev_tokens = tokens;
	auto tokens_guard = MakeLambdaGuard(
		[prev_tokens, &tokens]() {
			tokens = prev_tokens;
		}
	);
	
	VarDeclInitType init_type = VarDeclInitType_None;
	vector<Expr*> init_params;

	if(PeekAndConsumeUtil(tokens, {"="})) {
		init_type = VarDeclInitType_Equals;
		init_params.push_back(ParseExpr(context, tokens, /*disallow_infix=*/{","}));
	} else if(!param_mode && PeekAndConsumeUtil(tokens, {"("})) {
		init_type = VarDeclInitType_Ctor;
		init_params = ParseCommaSeparatedArguments(context, tokens, /*terminator*/{")"});
	} else if(!param_mode && PeekAndConsumeUtil(tokens, {"{"})) {
		init_type = VarDeclInitType_InitList;
		init_params = ParseCommaSeparatedArguments(context, tokens, /*terminator*/{"}"});
	}

	VarDecl* decl = new VarDecl(name, id.loc, type, 
								init_type, init_params);
	
	context.AddDecl(decl);
	tokens_guard.deactivate();
	return decl;
}

VarDecl* ParseParamDecl(Context& context,
					  vector<Token>& tokens) throws(Status) {
fprintf(stderr, "ParseParamDecl next %s\n", tokens[0].content.c_str());

	Type* type = ParseType(context, tokens) throws();

fprintf(stderr, "ParseParamDecl type %s\n", type->DebugString(0).c_str());

	Identifier id = ConsumeIdentifierFromSingleToken(tokens) throws();

fprintf(stderr, "ParseParamDecl id %s\n", id.DebugString().c_str());


	return ParseVarDecl(context, tokens, id, 
						/*template_params=*/{}, 
						type,
						/*static_specified=*/false, 
						/*param_mode=*/true);
}


// Only consumes tokens if successful
// Returns nullptr if an identifier couldn't be parsed
// Throws if it was an identifier but it couldn't be resolved, or missing template args
DeclRef* ParseDeclRef(Context& context,
				vector<Token>& tokens) throws(Status) {
fprintf(stderr, "ParseDeclRef %s\n", tokens[0].content.c_str());

	vector<Token> prev_tokens = tokens;

	auto tokens_guard = MakeLambdaGuard(
		[prev_tokens, &tokens]() {
			tokens = prev_tokens;
		}
	);

	LocationRef loc = tokens[0].loc;

	// Decl for identifier
	Identifier id;
	try {
		id = ParseIdentifier(tokens) throws();
	} catch(Status status) {
		return nullptr;
	}

	Decl* decl = GetDeclByIdentifier(context, id) throws();

	vector<TemplateArg> template_args;
	auto templated_decl = AsA<TemplatedDecl*>(decl);
	if(templated_decl) {
		vector<TemplateParam*> template_params = templated_decl->GetTemplateParams();
		template_args = ParseTemplateArgs(context, tokens, template_params);
	}

	DeclRef* ret = new DeclRef(decl, /*template_params=*/template_args, loc);

	tokens_guard.deactivate();
	return ret;
}

Expr* AdjustUnaryPrecedence(UnaryOp* uop) {
	Expr* subexpr = uop->GetSub();

	if(auto bop = AsA<BinaryOp*>(subexpr)) {
		// --- Goal ---
		// (-x)+y
		// --- Original ---
		// -(x+y)

		Expr* x = bop->GetLeft();
		Expr* y = bop->GetRight();

		uop->SetSub(x);
		bop->SetLeft(uop);
		bop->SetRight(y);

		return bop;
	}
	return uop;
}

BinaryOp* IsCommaOp(Expr* expr) {
	auto bop = AsA<BinaryOp*>(expr);
	return (bop && bop->GetOp() == ",") ? bop : nullptr;
}

// Consumes terminator, such as ")"
vector<Expr*> ParseCommaSeparatedArguments(Context& context,
											vector<Token>& tokens,
											string terminator) {
	vector<Expr*> args;
	if(PeekAndConsumeUtil(tokens, {terminator})) {
		return args;
	}
	do {
		args.push_back(ParseExpr(context, tokens, /*disallow_infix=*/{","}));
	} while(PeekAndConsumeUtil(tokens, {","}));
	ConsumeOrError(tokens, {terminator});
	return args;
}

// Returns nullptr on non-function form
// Only consumes tokens on success
FuncCall* ParseFuncCall(Context& context,
				vector<Token>& tokens, 
				DeclRef* decl_ref) {

	vector<Token> prev_tokens = tokens;

	auto tokens_guard = MakeLambdaGuard(
		[prev_tokens, &tokens]() {
			tokens = prev_tokens;
		}
	);

	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};

	if(!PeekAndConsume({"("})) {
		return nullptr;
	}

	// --- We know it's a call now, so we throw errors after this ---

	LocationRef loc = decl_ref->GetLoc();

	// Check that the decl is a function
	Decl* callee_decl = decl_ref->GetRef();
	auto callee = AsA<FuncDecl*>(callee_decl);

	if(!callee) {
		throw Status{.message = string("Decl is not a function: ") + callee_decl->DebugString(0)};
	}

	// We can throw errors after this, as it must be a call
	vector<Expr*> args = ParseCommaSeparatedArguments(context, tokens, /*terminator=*/")");

	if(args.len() != callee->GetParameters().len()) {
		throw Status{.message = string("Function ") + callee->GetName() 
		+ " expects " + std::to_string(callee->GetParameters().len()).c_str()
		+ " parameters"};
	}

	FuncCall* funccall = new FuncCall(decl_ref, args, loc);

	tokens_guard.deactivate();

	return funccall;
}

Expr* ParseExpr(Context& context,
				vector<Token>& tokens,
				set<string> disallow_infixes) {
	fprintf(stderr, "ParseExpr %s\n", tokens[0].content.c_str());

	LocationRef loc;
	if(tokens.len()) {
		loc = tokens[0].loc;
	}
	Expr* leaf_parsed = nullptr;

	// Integer literal
	try{
		long long integer_literal = std::stoll(tokens[0].content.c_str()) throws();
		assert(leaf_parsed == nullptr);
		leaf_parsed = new Literal(new IntegerValue(integer_literal), tokens[0].loc);
		tokens.pop_front();
	} catch(std::invalid_argument invalid) {
	} catch(std::out_of_range invalid) {
	}

	// C style cast or parenthesis
	if(!leaf_parsed && tokens[0].content == "(") {
		LocationRef paren_loc = tokens[0].loc;
		tokens.pop_front();

		// C style cast
		//DeclRef* decl_ref = ParseDeclRef(context, tokens);
		Type* cast_to = ParseType(context, tokens, /*throw_on_failure=*/false);

		if(cast_to != nullptr) {
			ConsumeOrError(tokens, {")"});
			Expr* sub_expr = ParseExpr(context, tokens, disallow_infixes);
			Expr* cast_expr = new CastExpr(CastType_CStyle, cast_to, sub_expr, paren_loc);
			Expr* ret = AdjustUnaryPrecedence(AsA<UnaryOp*>(cast_expr));
			return ret;
		}

		// Regular parenthetical
		Expr* inner = new ParenExpr(ParseExpr(context, tokens, disallow_infixes), paren_loc);
		ConsumeOrError(tokens, {")"});
		leaf_parsed = inner;
	}


	// Ctor / CPP style cast
	Type* ctor_of_type = ParseType(context, tokens, /*throw_on_fail=*/false);
fprintf(stderr, "-- ctor_of_type %p\n", ctor_of_type);
	if(!leaf_parsed && ctor_of_type) {
		ConsumeOrError(tokens, {"("});
		auto ctor_of_struct = AsA<StructDecl*>(ctor_of_type);
		if(ctor_of_struct) {
			fprintf(stderr, "!! TODO: Ctor call on struct check param count\n");
		}
		// TODO: Typedef
		fprintf(stderr, "ParseExpr ctor_of_type %s\n", 
			ctor_of_type->DebugString(0).c_str());
		vector<Expr*> args = ParseCommaSeparatedArguments(context, tokens, /*terminator=*/")");
		leaf_parsed = new CtorCall(ctor_of_type, args, loc);
	}


	// Decl for identifier
	DeclRef* decl_ref = nullptr;
	if(!leaf_parsed) {
		decl_ref = ParseDeclRef(context, tokens);
	}
	if(decl_ref != nullptr) {
fprintf(stderr, "-- decl_ref %s\n", decl_ref->GetRef()->DebugString(0).c_str());
		assert(!leaf_parsed);
		leaf_parsed = decl_ref;
	}

	// Function call
	if(decl_ref && compiler::AsA<FuncDecl*>(decl_ref->GetRef())) {
fprintf(stderr, "-- Trying ParseFuncCall next %s\n", tokens[0].content.c_str());
		FuncCall* call = ParseFuncCall(context, tokens, decl_ref);
		if(call != nullptr) {
			leaf_parsed = call;
		}
	}

	const set<string> unary_operators = GetAllUnaryOperators();
	if(!leaf_parsed && unary_operators.contains(tokens[0].content)) {
		Token uop_tok = tokens[0];
		tokens.pop_front();

		Expr* sub_expr = ParseExpr(context, tokens, disallow_infixes);
		UnaryOp* uop_expr = new UnaryOp(uop_tok.content, /*postfix=*/false, sub_expr, uop_tok.loc);
		return AdjustUnaryPrecedence(uop_expr);
	}

	const set<string> unary_postfix = GetAllUnaryPostfixOperators();
	if(leaf_parsed && unary_postfix.contains(tokens[0].content)) {
		Token uop_tok = tokens.pop_front();
		if(uop_tok.content == "." || uop_tok.content == "->") {
			Identifier id = ConsumeIdentifierFromSingleToken(tokens);
			leaf_parsed = new MemberExpr(leaf_parsed,
										 id.parts[0],
										 uop_tok.content == "->",
										 uop_tok.loc);
		} else {
			leaf_parsed = new UnaryOp(uop_tok.content, /*postfix=*/true, leaf_parsed, uop_tok.loc);
		}
	}

	set<string> infix_operators = GetAllInfixOperators();
	infix_operators.remove(disallow_infixes);

	if(leaf_parsed && infix_operators.contains(tokens[0].content)) {
		Token operator_token = tokens.pop_front();
		Expr* right_side = ParseExpr(context, tokens, disallow_infixes);
		return new BinaryOp(operator_token.content,
							leaf_parsed,
							right_side,
							operator_token.loc) throws();
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

	if(PeekAndConsumeUtil(tokens, {"return"})) {
		Stmt* ret = new ReturnStmt(ParseExpr(context, tokens, /*disallow_infix=*/{}), loc);
fprintf(stderr, "ParseStmt return next %s ret %s\n", 
	tokens[0].content.c_str(),
	ret->DebugString(0).c_str());
		ConsumeOrError(tokens, {";"}) throws ();
		return ret;
	}

	// TODO: Static

	try {
		vector<Token> prev_tokens = tokens;
		auto tokens_guard = MakeLambdaGuard(
			[prev_tokens, &tokens]() {
				tokens = prev_tokens;
			}
		);
		Type* type = ParseType(context, tokens, /*throw_on_failure=*/false) throws();

		if(type != nullptr) {
			Identifier id = ConsumeIdentifierFromSingleToken(tokens) throws();
			Stmt* ret = ParseVarDecl(context, tokens, id, 
							/*template_params=*/{}, 
							type,
							/*static_specified=*/false, 
							/*param_mode=*/false) throws();
			ConsumeOrError(tokens, {";"}) throws ();
			tokens_guard.deactivate();
			return ret;
		}
	} catch (Status status) {
		
	}

	Stmt* ret = ParseExpr(context, tokens, /*disallow_infix=*/{}) throws ();
	ConsumeOrError(tokens, {";"}) throws ();
	return ret;
}

// Throws on failure
// Starts from after the "return_type name"
// Only consumes tokens on success
FuncDecl* ParseFuncDecl(Context& context,
						vector<Token>& tokens,
						Identifier id,
						vector<TemplateParam*> template_params,
						Type* return_type,
						bool static_specified) throws(Status) {
	vector<Token> prev_tokens = tokens;

	auto tokens_guard = MakeLambdaGuard(
		[prev_tokens, &tokens]() {
			tokens = prev_tokens;
		}
	);

	assert(id.parts.len() > 0);
	if(id.global || id.parts.len() > 1) {
		throw Status{.message="FuncDecl qualified names not yet supported"};
	}

	string name = id.parts[0];
	LocationRef loc = id.loc;

fprintf(stderr, "-- ParseFuncDecl %s\n", name.c_str());

	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};

	context.PushFrame();

	auto context_pop_guard = MakeLambdaGuard(
		[&context]() {
			context.PopFrame();
	});

	ConsumeOrError(tokens, {"("});

	vector<VarDecl*> parameters;

	for(bool first = true;
		!PeekAndConsume({")"});
		first = false) {

		if(!first) {
			ConsumeOrError(tokens, {","}) throws();
		}

		parameters.push_back(ParseParamDecl(context, tokens));
	}

	bool is_prototype = false;
	if(PeekAndConsume({";"})) {
		is_prototype = true;
	}

	auto funcdecl = new FuncDecl(name, /*template_params=*/template_params, 
								 return_type, parameters, 
								 /*is_prototype=*/is_prototype,
								 /*body=*/{}, loc);
	// Add as soon as the signature is ready for recursion to find it
	context.AddDecl(funcdecl);

	if(!is_prototype) {
		ConsumeOrError(tokens, {"{"});

		vector<Stmt*> body;

		while(!PeekAndConsume({"}"})) {
			body.push_back(ParseStmt(context, tokens));
		}

		funcdecl->SetBody(body);
	}

	tokens_guard.deactivate();

	return funcdecl;
}

StructDecl* ParseStructDecl(Context& context,
							vector<Token>& tokens,
							vector<TemplateParam*> template_params) throws();

// Consumes ;
TypedefDecl* ParseTypedef(Context& context,
						  vector<Token>& tokens) {
	Type* type = ParseType(context, tokens);
	Identifier id = ConsumeIdentifierFromSingleToken(tokens);
	ConsumeOrError(tokens, {";"});
	return new TypedefDecl(id.parts[0], type, id.loc);
}

// Consumes the ;
UsingDecl* ParseUsing(Context& context,
					  vector<Token>& tokens,
					  vector<TemplateParam*> template_params) {
	Identifier id = ParseIdentifier(tokens) throws();
	if(PeekAndConsumeUtil(tokens, {"="})) {
		if(id.global || id.parts.len() > 1) {
			throw Status{.message = "Using = can't specify qualified identifier as alias"};
		}
fprintf(stderr, "--- ParseUsing %s = %s\n", id.parts.back().c_str(), tokens[0].content.c_str());
		// TODO: Apply template params
		Type* base = ParseType(context, tokens) throws();
fprintf(stderr, "----- base %s\n", base->DebugString(0).c_str());
		ConsumeOrError(tokens, {";"});
		return new UsingAliasDecl(id.parts[0],
							 base,
							 template_params, 
							 id.loc);
	}
	if(template_params.len() > 0) {
		throw Status{.message = "Using can't have template params unless aliasing"};		
	}
fprintf(stderr, "ParseUsing id %s\n", id.DebugString().c_str());
	Decl* decl = GetDeclByIdentifier(context, id) throws();
	auto type = AsA<Type*>(decl);
	if(type == nullptr) {
		throw Status{.message = "Using declaration must be on type name"};		
	}
	ConsumeOrError(tokens, {";"});
	return new UsingDecl(id.parts.back(),
						 type, 
			  			 id.loc);
}

// Consumes the ;
Decl* ParseDecl(Context& context, vector<Token>& tokens) {
	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};

	context.PushFrame();
	auto template_context_pop_guard = MakeLambdaGuard(
		[&context]() {
			context.PopFrame();
	});


	if(PeekAndConsume({"typedef"})) {
		return ParseTypedef(context, tokens);
	}

	vector<TemplateParam*> template_params;
	if(PeekAndConsume({"template"})) {
		template_params = ParseTemplateParams(context, tokens);
	}

	if(PeekAndConsume({"using"})) {
		return ParseUsing(context, tokens, template_params);
	}
	if(PeekForAnyUtil(tokens, {"class", "struct"})) {
		return ParseStructDecl(context, tokens, template_params);
	}

	bool static_specified = false;
	if(PeekAndConsumeUtil(tokens, {"static"})) {
		static_specified = true;
	}

	// Parse as type
	Type* type = ParseType(context, tokens) throws();

	Identifier id = ParseIdentifier(tokens) throws();

	// Where ambiguous, prefer to interpret as function prototype
	try {
		// Function proto is default, as it's the most complicated to parse
		return ParseFuncDecl(context, tokens, id, template_params, type, static_specified);
	} catch (Status status) {
		fprintf(stderr, "ParseDecl:ParseFuncDecl status %s\n", status.message.c_str());
	}

	Decl* ret = ParseVarDecl(context, tokens, id, template_params, type, static_specified);
	ConsumeOrError(tokens, {";"});
	return ret;
}

// Consumes struct/class token
StructDecl* ParseStructDecl(Context& context,
							vector<Token>& tokens,
							vector<TemplateParam*> template_params) throws() {

	Token keyword_tok = tokens.pop_front();
	LocationRef loc = keyword_tok.loc;
	string keyword = keyword_tok.content;

	bool declared_class = false;

	if(keyword == "class") {
		declared_class = true;
	} else if (keyword == "struct") {
		declared_class = false;
	} else {
		throw Status{.message=string("INTERNAL: ParseStructDecl called with first token ") + keyword};
	}

	Token name_tok = tokens.pop_front();

	context.PushFrame();
	auto template_context_pop_guard = MakeLambdaGuard(
		[&context]() {
			context.PopFrame();
	});

	vector<Decl*> inner_decls;

	ConsumeOrError(tokens, {"{"});

	while(!PeekAndConsumeUtil(tokens, {"}"})) {
		Decl* decl = ParseDecl(context, tokens);
		context.AddDecl(decl);
		inner_decls.push_back(decl);
	}

	// TODO: inline decls
	ConsumeOrError(tokens, {";"});

	return new StructDecl(name_tok.content,
						   declared_class,
						   template_params,
						   inner_decls,
						   loc);
}

void ParseNamespaceContents(Context& context,
							vector<Token>& tokens,
							Namespace& result) throws(Status) {
	auto PeekAndConsume = [&tokens](vector<string> look_for) {
		return PeekAndConsumeUtil(tokens, look_for);
	};

	context.PushFrame();

	auto context_pop_guard = MakeLambdaGuard(
		[&context]() {
			context.PopFrame();
	});

	int64 debug_prev_token_count = -1;
	while(!tokens.empty() && !PeekAndConsumeUtil(tokens, {"}"})) {
		assert(debug_prev_token_count != tokens.len());
		debug_prev_token_count = tokens.len();

		if(PeekAndConsume({"namespace"})) {
			Token name_tok = tokens.pop_front();
			if(!PeekAndConsume({"{"})) {
				throw Status{.message="Expected { after", .loc=name_tok.loc};
			}

			ContextFrame prev_frame = context.frames.back();
			auto nested = new Namespace(name_tok.content, name_tok.loc);
			context.frames.push_back(ContextFrame{.in_namespace = nested, 
												  .top_namespace = prev_frame.top_namespace});
			ParseNamespaceContents(context, tokens, *nested);
			result.AddNested(nested);
		}

		Decl* decl = ParseDecl(context, tokens);

		result.AddDecl(decl) throws();
		context.AddDecl(decl) throws();
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
