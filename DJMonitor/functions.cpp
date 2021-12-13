#include "functions.hpp"

#include <unordered_set>
#include <sstream>
#include <deque>

using namespace std;

long long to_number(string inp) {
	if (inp == "") return 0;
	using namespace exprtk;
	expression<double> expr;
	parser<double>pars;
	if (!pars.compile(inp, expr)) return 0;
	return expr.value();
}

string to_upper(string inp) {
	string outp = inp;
	for (char &it : outp) it = toupper(it);
	return outp;
}

vector<string> split_string(const string &inp, string delims, bool incl_null) {
	vector<string> outp {""};
	unordered_set<char> delimiters;
	for (const char &it : delims) delimiters.insert(it);
	for (const char &it : inp) {
		if (delimiters.contains(it)) {
			if (incl_null || !outp.back().empty()) outp.push_back("");
		} else {
			outp.back().push_back(it);
		}
	}
	return outp;
}

datetime::seconds parse_duration(string inp) {
	using namespace datetime;
	string num;
	for (const char &it : inp) {
		switch (it) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '*':
		case '-':
		case '/':
		case '+':
		case ':':
			num.push_back(it);
		}
	}
	vector<string> inpvec = split_string(num, ":");
	deque<string> outp {inpvec.begin(), inpvec.end()};
	while (outp.size() < 3) outp.push_front("");
	return hours(to_number(outp[0])) + minutes(to_number(outp[1])) + seconds(to_number(outp[2]));
}

vector<string> tokenize(string inp) {
	vector<string>outp {""};
	bool num = true;
	for (const char &i : inp) {
		char it = tolower(i);
		switch (it) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (!num && !outp.back().empty()) outp.push_back("");
			outp.back().push_back(it);
			num = true;
			break;
		case '.':
		case '-':
		case '/':
		case '\\':
		case '_':
			if (!outp.back().empty()) outp.push_back("");
			break;
		default:
			if (num && !outp.back().empty()) outp.push_back("");
			outp.back().push_back(it);
			num = false;
		}
	}
	return outp;
}

bool cmp_version(string tst, string min) {
	// tst >= min
	// Returns false if tst < min, true otherwise
	vector<string> minver = tokenize(min), testver = tokenize(tst);
	size_t minsize = minver.size(), testsize = testver.size();
	for (size_t i = 0; i < minsize && i < testsize; ++i) {
		long long testnum = to_number(testver[i]), minnum = to_number(minver[i]);
		if (testver[i][0] == '0' || testnum) {
			// test version token is a number; it's > non-numbers
			// if min version isn't a number, test version is >
			if (minver[i][0] != '0' && !minnum) return true;
			if (testnum > minnum) return true;
			if (testnum < minnum) return false;
		} else {
			// test version isn't a number; it's < numbers
			// if min version is a number, min version is >
			if (minver[i][0] == '0' || minnum) return false;
			// if neither is a number, compare lexicographically
			int cmp = testver[i].compare(minver[i]);
			if (cmp < 0) return false;
			if (cmp > 0) return true;
		}
	}
	// If there are more tokens in the minimum, check and see if the next token is "alpha" or "beta"
	// If so, test version is >.  Otherwise, min version is >
	if (minsize > testsize) {
		if (minver[testsize] == "alpha" || minver[testsize] == "beta") return true;
		return false;
	}
	// If there are more tokens in the test and the next token is "alpha" or "beta", it's less
	if (testsize > minsize && (testver[minsize] == "alpha" || testver[minsize] == "beta")) return false;
	// Default to true
	return true;
}

wxPoint get_center(const wxRect &inp) {
	return wxPoint(inp.x + (inp.width / 2), inp.y + (inp.height / 2));
}

quadrant compare_rects(const wxRect &first, const wxRect &second) {
	// Gives the quadrant of the first rectangle in relation to the second
	
	wxPoint first_center = get_center(first), second_center = get_center(second);
	if (first_center.x == second_center.x && first_center.y == second_center.y) return quadrant::ON_TOP;
	if (first_center.x == second_center.x) return (first_center.y < second_center.y ? quadrant::IS_TOP : quadrant::IS_BOT);
	if (first_center.y == second_center.y) return (first_center.x < second_center.x ? quadrant::IS_LFT : quadrant::IS_RGT);
	char status = ((first_center.x > second_center.x) << 1) + (first_center.y > second_center.y);
	switch (status) {
	case 0:
		return quadrant::UP_LFT;
	case 1:
		return quadrant::DN_LFT;
	case 2:
		return quadrant::UP_RGT;
	case 3:
		return quadrant::DN_RGT;
	}
	return quadrant::INVALID;
}

string bitmap_calc(const string &inp) {
	// Return a string containing the bit numbers that are set in the input number

	long long num = to_number(inp);
	string outp;
	int bit = 1;
	while (num) {
		if (num & 1) outp += to_string(bit) + ", ";
		bit++;
		num >>= 1;
	}
	while (!outp.empty() && (outp.back() == ' ' || outp.back() == ',')) outp.pop_back();
	return move(outp);
}

string bitmap_create(const vector<string> &inp) {
	// Return a string containing the number you get when you set the bit numbers specified in the input

	long long num = 0, outp = 0;
	for (const string &it : inp) {
		num = to_number(it);
		if (num > 0) outp |= (long long)1 << --num;
	}
	return move(to_string(outp));
}

string bitmap_calc_long(const string &inp) {
	// Take a multi-line string and for each line, calculate either the bits set from the bitmap, or the bitmap from the bits set

	string outp;
	for (const string &it : split_string(inp, "\r\n")) {
		vector<string> bits = split_string(it, ", ");
		if (bits.size() > 1) {
			outp.append(move(bitmap_create(bits)));
		} else {
			outp.append(move(bitmap_calc(it)));
		}
		outp.push_back('\n');
	}
	while (outp.back() == '\n') outp.pop_back();
	return move(outp);
}

// Knuth-Morris-Pratt string search algorithm functions

void kmp_fail(string &inp, vector<size_t> &tree) {
	tree[0] = 0;
	size_t len = 0, i = 1;
	while (i < inp.length()) {
		if (inp[i] == inp[len]) {
			tree[i++] = ++len;
		} else {
			if (len) {
				len = tree[len - 1];
			} else {
				tree[i++] = 0;
			}
		}
	}
}

size_t kmp(string &text, string &pattern, vector<size_t> &tree, size_t start = 0) {
	if (pattern.empty()) return -1;
	size_t tlen = text.length(), plen = pattern.length(), i = start, j = 0;
	while (i < tlen) {
		if (text[i] == pattern[j]) {
			++i;
			++j;
		} else {
			if (i < tlen) {
				if (j) {
					j = tree[j - 1];
				} else {
					i++;
				}
			}
		}
		if (j == plen) {
			return i - plen;
		}
	}
	return -1;
}

size_t kmp(string &text, string &pattern, size_t start = 0) {
	vector<size_t> tmp {pattern.length()};
	kmp_fail(pattern, tmp);
	return kmp(text, pattern, tmp, start);
}

// Split a string by delimeter using Knuth-Morris-Pratt string search

vector<string> pieces(string data, string delim) {
	vector<string> outp;
	vector<size_t> tree;
	size_t len = delim.length();
	tree.resize(len);
	kmp_fail(delim, tree);
	size_t st = 0, ed = kmp(data, delim, tree);
	while (ed != -1) {
		outp.push_back(data.substr(st, ed - st));
		st = ed + len;
		ed = kmp(data, delim, tree, st);
	}
	outp.push_back(data.substr(st));
	return outp;
}
