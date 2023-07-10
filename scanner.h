#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"
#include "vector.h"
#include "set.h"
#include "utils.h"
#include "tokens.h"

namespace stacklang {
namespace compiler {

set<char> init_word_chars() {
	set<char> ret{'_'};
	for(char c='a';c<='z';++c) {
		ret.add(c);
	}
	for(char c='A';c<='Z';++c) {
		ret.add(c);
	}
	for(char c='0';c<='9';++c) {
		ret.add(c);
	}
	return ret;
}

set<char> init_special_chars(set<string> special_tokens) {
	set<char> ret;
	for(string tok : special_tokens) {
		for(char c : tok) {
			ret.add(c);
		}
	}
	return ret;
}

void filter_tokens(set<string>& tokens, int64 idx, char c) {
	set<string> ret;
	for(string t : tokens) {
		if(idx >= t.len()) {
			continue;
		}
		if(t[idx] == c) {
			ret.add(t);
		}
	}
	tokens = ret;
}

vector<string> Scan(string input) throws (Status) {
	const set<string> special_tokens = GetAllSpecialTokens();
	const set<char> special_chars = init_special_chars(special_tokens);
	const set<char> whitespaces{' ', '\t', '\n', '\r'};
	const set<char> word_chars = init_word_chars();

	enum char_type {
		char_type_null=0,
		char_type_whitespace=1,
		char_type_word=2,
		char_type_special=3
	};

	auto classify_char = [special_chars, whitespaces, word_chars](char c) -> char_type {
		if(special_chars.contains(c)) {
			return char_type_special;
		}
		if(whitespaces.contains(c)) {
			return char_type_whitespace;
		}
		if(word_chars.contains(c)) {
			return char_type_word;
		}
		return char_type_null;
	};

	vector<string> ret;

	set<string> remaining_special_tokens;
	string current_token = "";

	auto complete_token = [&ret, &current_token]() {
		if(current_token.len()) {
			ret.push_back(current_token);
			current_token = "";
		}
	};

	auto read_line_marker = [&input, &ret]() {
		string token = "";
		while(!input.empty()) {
			char next = input[0];
			input = input.tail(1);
			if(next == '\n') {
				break;
			}
			token = token + next;
		}
		ret.push_back(token);
	};

	int64 debug_last_len = -1;
	char_type last_type = char_type_null;

	while(!input.empty()) {
		assert(input.len() != debug_last_len);
		debug_last_len = input.len();

		char next = input[0];

		// Special line marker mode
		if(next == '#') {
			read_line_marker();
			continue;
		}

		char_type next_type = classify_char(next);
		input = input.tail(1);

		if(next_type != last_type) {
			complete_token();
		}
		last_type = next_type;

		if(next_type == char_type_whitespace) {
			continue;
		}

		if(next_type == char_type_word) {
			current_token = current_token + next;
			continue;
		}

		if(next_type == char_type_special) {
			if(current_token.empty()) {
				remaining_special_tokens = special_tokens;
			}
			filter_tokens(remaining_special_tokens, current_token.len(), next);

			// Several special tokens one after another
			if(remaining_special_tokens.size() == 0) {
				complete_token();
				// Put it back
				input = string(next) + input;
				debug_last_len = 0;
				continue;
			}

			current_token = current_token + next;

			if(remaining_special_tokens.size() == 1) {
				assert(current_token == *remaining_special_tokens.begin());
				complete_token();
			}

			continue;
		}

		//return Status{.message=string("Didn't know what to do with char: ") + next};
		throw Status{.message=string("Didn't know what to do with char: ") + next};
	}

	complete_token();

	return ret;
}

}  // compiler
}  // stacklang

#endif//SCANNER_H
