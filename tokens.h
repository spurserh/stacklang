#ifndef TOKENS_H
#define TOKENS_H

#include "set.h"
#include "map.h"

namespace stacklang {
namespace compiler {



map<string, int64> GetAllInfixOperatorsWithPrecedence() {
	int64 next_prec = 1;
	map<string, int64> ret;
	auto add_with_prec = [&next_prec, &ret](set<string> ops) {
		for(string op : ops) {
			ret.set(op, next_prec);
		}
		++next_prec;
	};

	add_with_prec({"*", "/", "%"});
	add_with_prec({"+", "-"});
	add_with_prec({"<<", ">>"});
	add_with_prec({"<", "<="});
	add_with_prec({">", ">="});
	add_with_prec({"==", "!="});
	add_with_prec({"&"});
	add_with_prec({"|"});
	add_with_prec({"^"});
	add_with_prec({"|"});
	add_with_prec({"&&"});
	add_with_prec({"||"});
	add_with_prec({"?"});
	add_with_prec({"=", "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", ">>=", "<<="});
	add_with_prec({","});

	return ret;
}

map<string, int64> GetAllUnaryOperatorsWithPrecedence() {
	int64 next_prec = 1;
	map<string, int64> ret;
	auto add_with_prec = [&next_prec, &ret](set<string> ops) {
		for(string op : ops) {
			ret.set(op, next_prec);
		}
		++next_prec;
	};

	add_with_prec({"++", "--"});
	add_with_prec({"!", "~", "*", "&", "-", "+"});

	return ret;
}

set<string> GetAllUnaryPostfixOperators() {
	return {"++", "--", ".", "->"};
}

set<string> GetAllInfixOperators() {
	return GetAllInfixOperatorsWithPrecedence().keys();
}

set<string> GetAllUnaryOperators() {
	return GetAllUnaryOperatorsWithPrecedence().keys();
}

set<string> GetAllSpecialTokens() {
	set<string> special_tokens{"(", ")", "{", "}",  
									",", ";", ":", "::"};
	special_tokens.add(GetAllInfixOperators());
	special_tokens.add(GetAllUnaryOperators());
	special_tokens.add(GetAllUnaryPostfixOperators());
	return special_tokens;
}

}  // namespace compiler
}  // namespace stacklang

#endif//TOKENS_H