#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
using namespace std;
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <climits>
#include <iomanip>

static int HELP_STATE = 0;
static int DEBUG_STATE = 0;

const int MAX_ROUND = 2000;

bool seenQ = false, seenS = false, seenG = false;
bool seenq0 = false, seenB = false, seenF = false, seenN = false;

vector<string> Q;    // set of states
vector<char> S, G;  // set of input symbols, set of tape symbols
string q0;                // initial state
char B = '_';            // blank character, default '_'
vector<string> F;   // set of final states
int N = 0;               // number of tapes

struct delta_item {
	int old_state_hash;
	int new_state_hash;
	string old_symbols;
	string new_symbols;
	string dirs;
};
vector<delta_item> delta; // transition function
							// every item contains: old_state old_chars new_chars dirs new_state
struct new_delta_item {
	string new_state;
	string new_symbols;
	string dirs;
};
map<string, new_delta_item> delta_map;  
	//use 'ole_state;old_symbols' to uniquely identify one transition

map<string, int> stateHash;  // hash code of states in Q
map<char, int> symbolHash; // hash code of symbols in G
map<string, int> stateHashF;

/* prehandle(argc,argv):
	return 0: enter help module
	return 1: enter verbose module
	reutrn 2: enter parse module
	o.w. enter stderr module
	3: error_input,4: non-existent file, >=5: may need to be added
*/
int prehandle(int argc, char* argv[],string& filename) {
	if (argc < 2 || argc>5)
		return 3;
	if (argc == 2) {
		string val = argv[1];
		if (val == "-h" || val == "--help") {
			HELP_STATE = 1;
			return 0;
		}
		return 3;
	}
	filename = argv[2];
	if (argc == 4) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
			HELP_STATE = 1;
		else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0)
			DEBUG_STATE = 1;
		else
			return 3;
	}
	else if (argc == 5) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
			HELP_STATE = 1;
		else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0)
			DEBUG_STATE = 1;
		else
			return 3;
		if (strcmp(argv[2], "-h") == 0 || strcmp(argv[2], "--help") == 0)
			HELP_STATE = 1;
		else if (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0)
			DEBUG_STATE = 1;
		else
			return 3;
		filename = argv[3];
	}
	else {
		filename = argv[1];
	}

	if (!fopen(filename.c_str(), "r"))
		return 4;
	return 2;
}
void help() {
	printf("usage: turing [-v|--verbose] [-h|--help] <tm> <input>\n");
}
void errinput() {
	printf("error: The input format is wrong.\n");
}
void nonexistentfile() {
	printf("error: non-existent filename.\n");
}

bool storeStates2QF(int line, string str,string type) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == ';') {
			str = str.substr(0, i);
			break;
		}
	}
	bool syntaxError = false;
	bool seenEqual = false;
	int p = 2;
	for (; p < str.length(); p++) {
		if (str[p] == ' ') continue;
		if (str[p] == '=')
			seenEqual = true;
		else if (str[p] == '{') {
			if (!seenEqual) {
				if (DEBUG_STATE) printf("syntax error: Line %d, miss character '=' before '{'.\n", line);
				syntaxError = true;
			}
			//seenLeftBracket = true;
			break;
		}
	}
	if (p == str.length()) {
		if (DEBUG_STATE) printf("syntax error: Line %d, miss character '{'.\n", line);
		syntaxError = true;
	}
	p++;
	int state_begin = 0, state_end = 0;
	for (; p < str.length(); p++) {
		if (str[p] == ' ') continue;
		if (str[p] == ',' || str[p] == '}') {
			if (state_end == state_begin) {
				if (DEBUG_STATE) printf("syntax error: Line %d, there exists a state whose identifier is blank in %s.\n", line,type.c_str());
				syntaxError = true;
			}
			if(type=="Q") Q.push_back(str.substr(state_begin, p - state_begin));
			else F.push_back(str.substr(state_begin, p - state_begin));
			state_end = state_begin = p;
			if (str[p] == '}') break;
		}
		else {
			if (!((str[p] >= 'a' && str[p] <= 'z') || (str[p] >= 'A' && str[p] <= 'Z') || (str[p] >= '0' && str[p] <= '9') || str[p] == '_')) {
				if (DEBUG_STATE) printf("syntax error: Line %d, unrecognized character in identifier of state in %s.\n", line, type.c_str());
				syntaxError = true;
			}
			if (state_end == state_begin)
				state_begin = p;
		}
	}
	if (p == str.length()) {
		if (DEBUG_STATE) printf("syntax error: Line %d, miss character '}'.\n", line);
		syntaxError = true;
	}
	p++;
	for (; p < str.length(); p++) {
		if (str[p] != ' ') {
			if (DEBUG_STATE) printf("syntax error: Line %d, there exists something else after the character '}'.\n", line);
			syntaxError = true;
			break;
		}
	}
	return !syntaxError;
}

bool storeChar2SG(int line, string str,string type) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == ';') {
			str = str.substr(0, i);
			break;
		}
	}
	bool syntaxError = false;
	bool seenEqual = false;
	int p = 2;
	for (; p < str.length(); p++) {
		if (str[p] == ' ') continue;
		if (str[p] == '=')
			seenEqual = true;
		else if (str[p] == '{') {
			if (!seenEqual) {
				if (DEBUG_STATE) printf("syntax error: Line %d, miss character '=' before '{'.\n", line);
				syntaxError = true;
			}
			break;
		}
	}
	if (p == str.length()) {
		if (DEBUG_STATE) printf("syntax error: Line %d, miss character '{'.\n", line);
		syntaxError = true;
	}
	p++;
	int symbol_begin = 0, symbol_end = 0;
	for (; p < str.length(); p++) {
		if (str[p] == ' ') continue;
		if (str[p] == ',' || str[p] == '}') {
			if (symbol_end == symbol_begin) {
				if (DEBUG_STATE) printf("syntax error: Line %d, there exists a symbol whose identifier is blank in %s.\n", line,type.c_str());
				syntaxError = true;
			}
			if (p - symbol_begin != 1) {
				if (DEBUG_STATE) printf("syntax error: Line %d, there exists a symbol whose length not equal 1 in %s.\n", line, type.c_str());
				syntaxError = true;
			}
			else {
				if (type == "S") S.push_back(str[symbol_begin]);
				else G.push_back(str[symbol_begin]);
			}				
			symbol_end = symbol_begin = p;
			if (str[p] == '}') break;
		}
		else {
			if (str[p] == ';' || str[p] == '*' || (str[p] == '_' && type == "S")) {
				if (DEBUG_STATE) printf("syntax error: Line %d, disallowed expression of the symbol in %s.\n", line, type.c_str());
				syntaxError = true;
			}
			if (symbol_end == symbol_begin)
				symbol_begin = p;
		}
	}
	if (p == str.length()) {
		if (DEBUG_STATE) printf("syntax error: Line %d, miss character '}'.\n", line);
		syntaxError = true;
	}
	p++;
	for (; p < str.length(); p++) {
		if (str[p] != ' ') {
			if (DEBUG_STATE) printf("syntax error: Line %d, there exists something else after the character '}'.\n", line);
			syntaxError = true;
			break;
		}
	}
	return !syntaxError;
}

bool storeNum2q0BN(int line, string str, string type) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == ';') {
			str = str.substr(0, i);
			break;
		}
	}
	bool syntaxError = false;
	bool seenEqual = false;
	int p = 2;
	if (type == "q0") p = 3;
	for (; p < str.length(); p++) {
		if (str[p] == ' ') continue;
		if (str[p] == '=')
			seenEqual = true;
		else {
			if (!seenEqual) {
				if (DEBUG_STATE) printf("syntax error: Line %d, miss character '='.\n", line);
				syntaxError = true;
			}
			break;
		}
	}
	if (type == "B") {
		if (str[p] != '_') {
			if (DEBUG_STATE) printf("Note: Line %d, the blank symbol B of this turing machine is not '_'\n", line);
			B = str[p];
		}
		p++;
		for (; p < str.length(); p++) {
			if (str[p] == ';') break;
			if (str[p] != ' ') {
				if (DEBUG_STATE) printf("syntax error: Line %d, there exists something else after the blank symbol B.\n", line);
				syntaxError = true;
				break;
			}
		}
	}
	else if (type == "q0") {
		int start = p;
		for (; p < str.length(); p++) {
			if (str[p] == ' ') continue;
			if (str[p] == ';') break;
			if (!((str[p] >= 'a' && str[p] <= 'z') || (str[p] >= 'A' && str[p] <= 'Z') || (str[p] >= '0' && str[p] <= '9') || str[p] == '_')) {
				if (DEBUG_STATE) printf("syntax error: Line %d, unrecognized character %c in identifier of initial state q0.\n", line,str[p]);
				syntaxError = true;
			}
		}
		q0 = str.substr(start, p - start);
	}
	else {
		int start = p;
		for (; p < str.length(); p++) {
			if (str[p] == ' ') continue;
			if (str[p] == ';') break;
			if (!(str[p]>='0'&&str[p]<='9')) {
				if (DEBUG_STATE) printf("syntax error: Line %d, %c is not a digit in tape number N.\n", line,str[p]);
				syntaxError = true;
			}
		}
		N = atoi(str.substr(start, p - start).c_str());
		if (N < 1) {
			if (DEBUG_STATE) printf("syntax error: Line %d, tape number N is less than 1 which should not happen.\n", line);
			syntaxError = true;
		}
	}
	return !syntaxError;
}

bool storeItem2delta(int line, string str) {
	string ele[5]; // old_state old_chars new_chars directions new_state
	int begin = -1, end = -1, num = 0;
	bool syntaxError = false;
	for (int i = 0; i <= str.length(); i++) {
		if (i == str.length() || str[i] == ' ') {
			if (begin == end) continue;
			if (num < 5) ele[num] = str.substr(begin, i - begin);
			num++;
			begin = end = i;
		}
		else if (str[i] == ';')
			break;
		else {
			if (begin == end) begin = i;
		}
	}
	if (num < 5) {
		if (DEBUG_STATE) printf("syntax error: Line %d, delta has less than 5 arguments which can not construct a delta item.\n", line);
		syntaxError = true;
	}
	else if (num > 5) {
		if (DEBUG_STATE) printf("syntax error: Line %d, delta has more than 5 arguments which exceeds the requirement.\n", line);
		syntaxError = true;
	}
	else {
		delta_item item;
		new_delta_item new_item;
		string identifier = "";
		if (!stateHash.count(ele[0]) || stateHash[ele[0]] == 0) {
			if (DEBUG_STATE) printf("syntax error: Line %d, the old state in delta is not defined in Q.\n", line);
			syntaxError = true;
		}
		else { 
			item.old_state_hash = stateHash[ele[0]];
			identifier = ele[0] + ";";
		}
		if (!stateHash.count(ele[4]) || stateHash[ele[4]] == 0) {
			if (DEBUG_STATE) printf("syntax error: Line %d, the new state in delta is not defined in Q.\n", line);
			syntaxError = true;
		}
		else { 
			item.new_state_hash = stateHash[ele[4]];
			new_item.new_state = ele[4];
		}
		if (ele[1].length() != N) {
			if (DEBUG_STATE) printf("syntax error: Line %d, the length of group of old symbols is not N.\n", line);
			syntaxError = true;
		}
		else {
			for (int p = 0; p < N; p++) {
				if (!symbolHash.count(ele[1][p]) || symbolHash[ele[1][p]] == 0) {
					if (DEBUG_STATE) printf("syntax error: Line %d, there exists some symbol not defined in G in group of old symbols.\n", line);
					syntaxError = true;
				}
			}
			item.old_symbols = ele[1];
			identifier += ele[1];
		}
		if (ele[2].length() != N) {
			if (DEBUG_STATE) printf("syntax error: Line %d, the length of group of new symbols is not N.\n", line);
			syntaxError = true;
		}
		else {
			for (int p = 0; p < N; p++) {
				if (!symbolHash.count(ele[2][p]) || symbolHash[ele[2][p]] == 0) {
					if (DEBUG_STATE) printf("syntax error: Line %d, there exists some symbol not defined in G in group of new symbols.\n", line);
					syntaxError = true;
				}
			}
			item.new_symbols = ele[2];
			new_item.new_symbols = ele[2];
		}
		if (ele[3].length() != N) {
			if (DEBUG_STATE) printf("syntax error: Line %d, the length of group of directions is not N.\n", line);
			syntaxError = true;
		}
		else {
			for (int p = 0; p < N; p++) {
				if (ele[3][p] != 'l' && ele[3][p] != 'r' && ele[3][p] != '*') {
					if (DEBUG_STATE) printf("syntax error: Line %d, there exists some symbol can not be recognized by directions.\n", line);
					syntaxError = true;
				}
			}
			item.dirs = ele[3];
			new_item.dirs = ele[3];
		}
		if (!syntaxError) { 
			delta.push_back(item); 
			delta_map[identifier] = new_item;
		}
	}
	return !syntaxError;
}

bool parser(string filename,string input) {
	ifstream infile1(filename.c_str(), ios::in);
	int line = 0;
	string temp;
	bool syntaxError = false;
	
	//while (infile.ios::eof() == 0)  is gapped by space, making handling more difficult.
	while(getline(infile1,temp)){
		line++;
		//cout << "temp:" << temp << "   " << temp.length() << endl;
		if (temp.length() < 2 || temp[0] == ';')   // eliminate something useless
			continue;
		if (temp[0] == '#') {
			switch (temp[1]) {
			case 'Q': 
				if (seenQ) {
					if (DEBUG_STATE) printf("syntax error: Line %d, Q  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeStates2QF(line, temp,"Q") == false) syntaxError = true;
				seenQ = true;
				// create hash code for Q to make it easier to search state
				for (int i = 0; i < Q.size(); i++) {
					stateHash[Q[i]] = i + 1;
				}
				break;
			case 'S':
				if (seenS) {
					if (DEBUG_STATE) printf("syntax error: Line %d, S  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeChar2SG(line, temp, "S") == false) syntaxError = true;
				seenS = true;
				break;
			case 'G':
				if (seenG) {
					if (DEBUG_STATE) printf("syntax error: Line %d, G  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeChar2SG(line, temp, "G") == false) syntaxError = true;
				seenG = true;
				// create hash code for G to make it easier to search symbol
				for (int i = 0; i < G.size(); i++) {
					symbolHash[G[i]] = i + 1;
				}
				break;
			case 'q':
				if (seenq0) {
					if (DEBUG_STATE) printf("syntax error: Line %d, q0  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeNum2q0BN(line, temp, "q0") == false) syntaxError = true;
				seenq0 = true;
				break;
			case 'B':
				if (seenB) {
					if (DEBUG_STATE) printf("syntax error: Line %d, B  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeNum2q0BN(line, temp, "B") == false) syntaxError = true;
				seenB = true;
				break;
			case 'F':
				if (seenF) {
					if (DEBUG_STATE) printf("syntax error: Line %d, F  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeStates2QF(line, temp, "F") == false) syntaxError = true;
				seenF = true;
				for (int i = 0; i < F.size(); i++) {
					stateHashF[F[i]] = i + 1;
				}
				break;		
			case 'N':
				if (seenN) {
					if (DEBUG_STATE) printf("syntax error: Line %d, N  is re-defined.\n", line);
					syntaxError = true;
					break;
				}
				if (storeNum2q0BN(line, temp, "N") == false) syntaxError = true;
				seenN = true;
				break;
			default: break;
			}
		}
		/*else if (storeItem2delta(line, temp) == false) syntaxError = true;*/
	}
	infile1.close();
	if (DEBUG_STATE) {
		if (!seenQ) printf("syntax error:  Q is not defined.\n");
		if (!seenS) printf("syntax error:  S is not defined.\n");
		if (!seenG) printf("syntax error:  G is not defined.\n");
		if (!seenq0) printf("syntax error:  q0 is not defined.\n");
		if (!seenB) printf("syntax error:  B is not defined.\n");
		if (!seenF) printf("syntax error:  F is not defined.\n");
		if (!seenN) printf("syntax error:  N is not defined.\n");
		if (!seenQ || !seenS || !seenG || !seenq0 || !seenB || !seenF || !seenN) syntaxError = true;
	}
	else {
		if (!seenQ || !seenS || !seenG || !seenq0 || !seenB || !seenF || !seenN) {
			syntaxError = true;
		}
	}
	if (!symbolHash.count(B) || symbolHash[B] == 0) {
		if (DEBUG_STATE) printf("syntax error: B is not in G.\n");
		syntaxError = true;
	}
	for (int i = 0; i < S.size(); i++) {
		if (!symbolHash.count(S[i]) || symbolHash[S[i]] == 0) {
			if (DEBUG_STATE) printf("syntax error: Some symbol %c in S is not in G.\n", S[i]);
			syntaxError = true;
		}
	}
	if (!stateHash.count(q0) || stateHash[q0] == 0) {
		if (DEBUG_STATE) printf("syntax error: q0 is not in Q.\n");
		syntaxError = true;
	}
	// To avoid delta definition before Q/S/G/...,
	// we read file twice and handle delta in the second time.
	ifstream infile2(filename.c_str(), ios::in);
	line = 0;
	while (getline(infile2, temp)) {
		line++;
		if (temp.length() < 2 || temp[0] == ';' || temp[0] == '#')   // eliminate something useless
			continue;
		// enter the transition function module
		if (storeItem2delta(line, temp) == false) syntaxError = true;
	}
	/*
	map<string, new_delta_item>::iterator iter = delta_map.begin();
	for (; iter != delta_map.end(); iter++)
		cout << iter->first << " " << iter->second.new_state << endl;*/
	infile2.close();
	if (syntaxError && !DEBUG_STATE) fprintf(stderr, "syntax error\n");
	return !syntaxError;
}

bool judgeInput(string input) {
	// examine whether there exists illegal symbol in input.
	bool legal = true;
	vector<int> error_index;
	for (int i = 0; i < input.length(); i++) {
		int j = 0;
		for (; j < S.size(); j++) {
			if (input[i] == S[j])
				break;
		}
		if (j == S.size()) {
			if (!DEBUG_STATE) {
				fprintf(stderr, "illegal input\n");
				return false;
			}
			error_index.push_back(i);
			legal = false;
		}
	}
	if (!legal) { 
		fprintf(stderr,"Input: %s\n", input.c_str());
		fprintf(stderr, "==================== ERR ====================\n");
		fprintf(stderr, "error: ");
		for(int i=0;i<error_index.size();i++)
			fprintf(stderr, "'%c'",input[error_index[i]]);
		fprintf(stderr, "was not declared in the set of input symbols\n");
		fprintf(stderr, "Input: %s\n", input.c_str());
		string ss(7 + input.length(), ' ');
		for (int i = 0; i < error_index.size(); i++)
			ss[error_index[i] + 7] = '^';
		fprintf(stderr, "%s\n", ss.c_str());
		fprintf(stderr, "==================== END ====================\n");
		return false;
	}
	if (DEBUG_STATE) {
		fprintf(stdout, "Input: %s\n", input.c_str());
		fprintf(stdout, "==================== RUN ====================\n");
	}
	return true;
}

bool simulator(string input) {
	// initial
	struct interval {
		int left, right;
		interval() :left(0), right(0) {};
		interval(int l, int r) :left(l), right(r) {};
	};
	vector<map<int, char>> tapes(N);
	vector<interval> valid_interval(N);
	for (int i = 1; i < N; i++)
		valid_interval[i].left = valid_interval[i].right = 0;
	//interval* valid_interval = new interval[N]();
	string cur_state = q0;
	vector<int> index(N, 0);
	int step = 0;
	for (int i = 0; i < input.length(); i++)
		tapes[0][i] = input[i];
	valid_interval[0].left = 0;
	valid_interval[0].right = input.length() - 1;

	// repeat before reach final state(accept) or halt
	while (step < MAX_ROUND) {
		if (DEBUG_STATE) {
			fprintf(stdout, "Step   : %d\n", step);
			for (int i = 0; i < N; i++) {
				string output_index = "Index" + to_string(i) + " : ";
				string output_tape = "Tape" + to_string(i) + "  : ";
				int head_index = 9;
				//fprintf(stdout, "Index%d : ", i);
				int left_border = valid_interval[i].right + 1, right_border = valid_interval[i].left - 1;
				for (int j = valid_interval[i].left; j <= valid_interval[i].right; j++)
					if (!tapes[i].count(j)) tapes[i][j] = '_';
				for (int j = valid_interval[i].left; j <= valid_interval[i].right; j++) {
					if (left_border == valid_interval[i].right + 1 && tapes[i][j] != '_') {
						left_border = j;
						break;
					}
				}
				if (left_border == valid_interval[i].right + 1) {
					left_border = right_border = index[i];
				}
				else {
					for (int j = valid_interval[i].right; j >= valid_interval[i].left; j--) {
						if (right_border == valid_interval[i].left - 1 && tapes[i][j] != '_') {
							right_border = j;
							break;
						}
					}
					if (index[i] < left_border) left_border = index[i];
					else if (index[i] > right_border) right_border = index[i];
				}
				for (int j = left_border; j <= right_border; j++) {	
					if (!tapes[i].count(j)) tapes[i][j] = '_';
					if (index[i] == j) head_index = output_index.length();
					output_index += to_string(abs(j)) + " ";
					output_tape.append(1, tapes[i][j]);
					if (abs(j) <= 9) output_tape += " ";// here, if directly + tapes[i][j], make error				
					else if(abs(j)<=99) output_tape += "  ";
					else output_tape +=  "   ";
				}
				fprintf(stdout, "%s\n", output_index.c_str());
				fprintf(stdout, "%s\n", output_tape.c_str());
				fprintf(stdout, "Head%d  : ", i);
				string output_head(head_index - 9, ' ');
				output_head += "^";
				fprintf(stdout, "%s\n", output_head.c_str());
			}
			fprintf(stdout, "State  : %s\n", cur_state.c_str());
			fprintf(stdout, "---------------------------------------------\n");
		}
		if (stateHashF.count(cur_state) && stateHashF[cur_state] > 0) { // true
			int left_border = valid_interval[0].right + 1, right_border = valid_interval[0].left - 1;
			for (int j = valid_interval[0].left; j <= valid_interval[0].right; j++)
				if (!tapes[0].count(j)) tapes[0][j] = '_';
			for (int j = valid_interval[0].left; j <= valid_interval[0].right; j++) {
				if (left_border == valid_interval[0].right + 1 && tapes[0][j] != '_') {
					left_border = j;
					break;
				}
			}
			if (left_border == valid_interval[0].right + 1) {
				left_border = right_border = index[0];
			}
			else {
				for (int j = valid_interval[0].right; j >= valid_interval[0].left; j--) {
					if (right_border == valid_interval[0].left - 1 && tapes[0][j] != '_') {
						right_border = j;
						break;
					}
				}
				//if (index[0] < left_border) left_border = index[0];
				//else if (index[0] > right_border) right_border = index[0];
			}
			fprintf(stdout, "The turing machine accepts.\n");
			fprintf(stdout, "Result: ");
			for (int i = left_border; i <= right_border; i++)
				fprintf(stdout, "%c", tapes[0][i]);
			fprintf(stdout, "\n");
			fprintf(stdout, "==================== END ====================\n");
			break;
		}
		string id = cur_state + ";";
		for (int i = 0; i < N; i++) {
			if (!tapes[i].count(index[i])) tapes[i][index[i]] = '_';
			id.append(1,tapes[i][index[i]]);
		}
		if (!delta_map.count(id)) {   // false
			int left_border = valid_interval[0].right + 1, right_border = valid_interval[0].left - 1;
			for (int j = valid_interval[0].left; j <= valid_interval[0].right; j++)
				if (!tapes[0].count(j)) tapes[0][j] = '_';
			for (int j = valid_interval[0].left; j <= valid_interval[0].right; j++) {
				if (left_border == valid_interval[0].right + 1 && tapes[0][j] != '_') {
					left_border = j;
					break;
				}
			}
			if (left_border == valid_interval[0].right + 1) {
				left_border = right_border = index[0];
			}
			else {
				for (int j = valid_interval[0].right; j >= valid_interval[0].left; j--) {
					if (right_border == valid_interval[0].left - 1 && tapes[0][j] != '_') {
						right_border = j;
						break;
					}
				}
				//if (index[0] < left_border) left_border = index[0];
				//else if (index[0] > right_border) right_border = index[0];
			}
			fprintf(stdout, "The turing machine halts.\n");
			fprintf(stdout, "Result: ");
			for (int i = left_border; i <= right_border; i++)
				fprintf(stdout, "%c", tapes[0][i]);
			fprintf(stdout, "\n");
			fprintf(stdout, "==================== END ====================\n");
			break;
		}
		new_delta_item item = delta_map[id];
		for (int i = 0; i < N; i++) {
			tapes[i][index[i]] = item.new_symbols[i];
			switch (item.dirs[i]) {
			case 'l': {
				index[i]--; 
				if (index[i] < valid_interval[i].left) valid_interval[i].left = index[i];
				//else if (index[i] < valid_interval[i].right) valid_interval[i].right = index[i];
				break;
			}
			case 'r': {
				index[i]++; 
				if (index[i] > valid_interval[i].right) valid_interval[i].right = index[i];
				//else if (index[i] > valid_interval[i].left) valid_interval[i].left = index[i];
				break; 
			}
			default:break;
			}
		}
		cur_state = item.new_state;
		step++;
	}
	//delete[] valid_interval;
	return true;
}

int main(int argc, char* argv[]) {
	// step1. prepare handle
	string filename = ".tm";
	int pre = prehandle(argc, argv, filename);
	if (pre == 3) {
		errinput(); return 1;
	}
	else if (pre == 4) {
		nonexistentfile(); return 2;
	}
	if (HELP_STATE)
		help();
	if (pre == 0)
		return 0;

	// step2. turing parser
	string input = argv[argc - 1];
	bool syntaxError = parser(filename,input);
	if (!syntaxError)
		return 3;

	// step3. turing simulator
	bool inputLegal = judgeInput(input);
	if (!inputLegal)
		return 4;
	bool sim = simulator(input);
	return 0;
}