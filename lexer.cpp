#include "lexer.h"

using namespace std;
set<char> Operator_beginning = {'&', '%', '|', '!', '^', '~', '+', '-', '*', '/', '=', '>', '<'};
set<char> Limiter_beginning = {';', ']', '[', ',', '(', ')', '{', '}'};
set<string> Keywords = {"auto", "bool", "break", "case", "catch", "char", "class", "const", "continue", "do", "double",
                        "default", "delete", "else", "enum", "explicit", "export", "extern", "false", "float", "for",
                        "friend", "goto", "if", "include", "inline", "int", "long", "namespace", "new", "operator",
                        "private", "protected", "public", "return", "register", "short", "signed", "sizeof", "static",
                        "string", "struct", "switch", "typedef", "typename", "template", "this", "throw", "true", "try",
                        "union", "unsigned", "using", "virtual", "void", "while"};


void print_FA(FA x) {
    cout << "states:" << endl;
    for (auto s: x.states) {
        for (auto i: s) {
            cout << '\t' << i;
        }
        cout << endl;
    }
    cout << "end states:" << endl;
    for (auto s: x.E_states) {
        for (auto i: s) {
            cout << '\t' << i;
        }
        cout << endl;
    }
    cout << "start states:" << endl;
    for (auto s: x.S_states) {
        for (auto i: s) {
            cout << '\t' << i;
        }
        cout << endl;
    }
    cout << "input_alpha_table:" << endl;
    for (auto i: x.sigma_input_characters) {
        cout << '\t' << i << endl;
    }
    cout << "transition:" << endl;
    for (auto f: x.f_transition) {
        State s_b = f.first.first;
        cout << "\t{";
        for (auto s: s_b)cout << '\"' << s << "\",";
        cout << '}';
        cout << "\t\t::=\t" << f.first.second << "\t\t";
        set<State > s_e = f.second;
        cout << '{';
        for (auto ss: s_e) for (auto s: ss) { cout << '\"' << s << "\","; }
        cout << '}';
        cout << endl;
    }
}

FA Regular_grammar2NFA(Regular_grammar G) {
    FA M;

    M.sigma_input_characters = G.terminal_symbols;//M的字母表和G的终结符相同
    M.states = G.non_terminal_symbols;
    M.S_states = G.start_symbol;
    M.E_states.insert(END_STATE);//Z作为M的终态

    for (auto i: G.rules) {//形如A::=aB的规则
        M.f_transition[i.first] = i.second;
    }
    return M;
}

set<State > varepsilon_closure(FA NFA, set<State > from_set_states) {
    set<State > closure = from_set_states;

    for (State T: from_set_states) {
        auto searched = NFA.f_transition.find(make_pair(T, EPSILON));
        if (searched != NFA.f_transition.end()) {
            for (auto s: NFA.f_transition[make_pair(T, EPSILON)])
                closure.insert(s);
        }
    }
    //递归实现
    return (closure == from_set_states ? closure : varepsilon_closure(NFA, closure));

}

set<State > move(FA NFA, set<State > from_set_states, Alpha alpha) {
    set<State > current_states;

    for (State T: from_set_states) {
        auto searched = NFA.f_transition.find(make_pair(T, alpha));
        if (searched != NFA.f_transition.end()) {
            for (auto s: NFA.f_transition[make_pair(T, alpha)])
                current_states.insert(s);
        }
    }
    return current_states;
}

State set_states2state(set<State > set_x) {
    State x;
    for (auto i: set_x)
        for (auto j: i)
            x.insert(j);
    return x;
}

bool is_set_state_end(set<State > set_x, FA NFA) {
    for (auto i: set_x)
        if (NFA.E_states.find(i) != NFA.E_states.end())
            return true;
    return false;
}

FA NFA2DFA(FA original_NFA) {
    FA DFA;

    map<set<State >, bool> visited;

    set<State > set_K = varepsilon_closure(original_NFA, original_NFA.S_states);
    State T0 = set_states2state(set_K);
    DFA.S_states = {T0};
    DFA.states.insert(T0);
    if (is_set_state_end(set_K, original_NFA))DFA.E_states.insert(T0);

    set<set<State > > C;
    C.insert(set_K);
    while (!C.empty()) {
        set<State > set_T = *C.begin();
        State DFA_T = set_states2state(set_T);//from state
        if (visited.find(set_T) == visited.end() || visited[set_T] == false) {
            for (auto alpha: original_NFA.sigma_input_characters) {
                set<State > set_U = varepsilon_closure(original_NFA, move(original_NFA, set_T, alpha));
                if (set_U.empty())continue;
                State DFA_U = set_states2state(set_U);//to state
                if (visited.find(set_U) == visited.end() || visited[set_U] == false) {//新状态，要加入状态集
                    C.insert(set_U);
                    DFA.states.insert(DFA_U);
                    if (is_set_state_end(set_U, original_NFA))
                        DFA.E_states.insert(DFA_U);
                }
                if (DFA.f_transition.find(make_pair(DFA_T, alpha)) == DFA.f_transition.end()) {
                    DFA.f_transition[make_pair(DFA_T, alpha)] = {DFA_U};
                } else {
                    DFA.f_transition[make_pair(DFA_T, alpha)].insert(DFA_U);
                }
            }
        }
        C.erase(set_T);
    }
    return DFA;

}

// FA NFA2DFA1(FA NFA) {
//     FA DFA;
//     //输入字母表是一样的
//     DFA.sigma_input_characters = NFA.sigma_input_characters;
//     //DFA的初始态是NFA初态的闭包的集合
//
//     set<State > current_states = varepsilon_closure(NFA, NFA.S_states);//注意，是针对NFA的set
//     State cur;
//     for (auto c: current_states) {
//         for (auto s: c) {
//             cur.insert(s);
//         }
//     }
//     DFA.S_states = {cur};
//     DFA.states.insert(cur);
//     //DFA的状态集是NFA状态集的一些子集
//     map<set<State >, bool> visited;//！NFA 的子集 是否被标记过
//     visited[current_states] = true;
//     generate_DFA_state(NFA, DFA, visited, current_states);
//
// }
//
// void generate_DFA_state(FA NFA, FA &DFA, map<set<State >, bool> &visited, set<State > current_set_states) {
//     bool new_state = false;
//     set<State > next_set_states;
//     for (auto i: current_set_states) {
//
//         for (auto alpha: DFA.sigma_input_characters) {
//             set<State > subset = varepsilon_closure(NFA, move(NFA, {i}, alpha));
//             if (subset.empty())continue;
//             State one_sub_state_DFA;//DFA的新状态
//             set<State > set_state_NFA;//对应的NFA的状态集
//             bool is_end_state = false;//是否为终态
//
//             for (auto s: subset) {
//                 if (NFA.E_states.find(s) != NFA.E_states.end())
//                     is_end_state = true;
//
//                 set_state_NFA.insert(s);
//
//                 for (auto item: s) {
//                     one_sub_state_DFA.insert(item);
//                 }
//             }
//
//             // DFA.f_transition[make_pair(cur, alpha)].insert(one_sub_state_DFA);
//             if (visited.find(set_state_NFA) == visited.end() || visited[set_state_NFA] == false) {
//                 visited[set_state_NFA] = true;
//                 for (auto s: set_state_NFA)
//                     next_set_states.insert(s);
//                 new_state = true;
//                 DFA.states.insert(one_sub_state_DFA);
//                 if (is_end_state)
//                     DFA.E_states.insert(one_sub_state_DFA);
//                 // generate_DFA_state(NFA, DFA, visited, next_set_states, one_sub_state_DFA);
//
//             }
//         }
//
//
//         if (new_state) {//存在新的子集，递归实现
//             generate_DFA_state(NFA, DFA, visited, next_set_states);
//         }
//     }
// }

Regular_grammar input_rules2Regular_grammar(string path) {
    /*
     *
     * [start_State]    表示初始态
     * <State>          表示状态
     * 'Alpha'          表示输入字符
     * ''               表示空字符epsilon
     * :=               表示左部定义于右部
    */
    Regular_grammar G;
    ifstream in_rules;
    in_rules.open(path, ios::in);
    string file_line;
    while (getline(in_rules, file_line)) {
        int pos = 0;
        State left_part;
        Alpha right_part_alpha;
        State right_part_state;
        bool is_right_state = false;
        if (file_line[pos] == '[') {
            int end_pos = file_line.find(']', pos + 1);
            State start_state = {file_line.substr(pos + 1, end_pos - pos - 1)};
            left_part = start_state;
            G.start_symbol.insert(start_state);
            pos = end_pos + 1;
        } else if (file_line[pos] == '<') {
            int end_pos = file_line.find('>', pos + 1);
            left_part = {file_line.substr(pos + 1, end_pos - pos - 1)};
            pos = end_pos + 1;
        }
        while (file_line[pos] != '\'')
            pos++;
        int end_pos = file_line.find('\'', pos + 1);
        right_part_alpha = file_line.substr(pos + 1, end_pos - pos - 1);
        pos = end_pos + 1;
        if (file_line[pos] == '<') {
            int end_pos = file_line.find('>', pos + 1);
            right_part_state = {file_line.substr(pos + 1, end_pos - pos - 1)};
            is_right_state = true;
        }

        G.non_terminal_symbols.insert(left_part);
        if (right_part_alpha != "")G.terminal_symbols.insert(right_part_alpha);
        if (!is_right_state)right_part_state = {END_STATE};
        G.non_terminal_symbols.insert(right_part_state);
        auto G_pos = G.rules.find(make_pair(left_part, right_part_alpha));
        if (G_pos == G.rules.end()) {
            G.rules[make_pair(left_part, right_part_alpha)] = {right_part_state};
        } else {
            G.rules[make_pair(left_part, right_part_alpha)].insert(right_part_state);
        }
    }
    return G;
}

vector<token> get_token_list(FA DFA, string path) {
    ifstream in_program;
    in_program.open(path, ios::in);
    string file_line;
    int line_id = 0;
    vector<token> token_list;
    while (getline(in_program, file_line)) {
        line_id++;
        int pos = 0;
        while (pos < file_line.size()) {
            char ch = file_line[pos];
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\0') {
                pos++;
                continue;
            }
            token word;
            get_token(DFA, file_line, pos, line_id, word);
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '-') {
                //keyword or identifier
                if (Keywords.find(word.value) != Keywords.end())
                    word.symbol = KEYWORD;
                else word.symbol = IDENTIFIER;
            } else if (ch >= '0' && ch <= '9' || ch == '"') {
                //const
                word.symbol = CONST;
            } else if (Operator_beginning.find(ch) != Operator_beginning.end()) {
                //operator
                word.symbol = OPERATOR;
            } else if (Limiter_beginning.find(ch) != Limiter_beginning.end()) {
                //limiter
                word.symbol = LIMITER;
            }

            if (!word.value.size())/*error todo*/;
            else token_list.push_back(word);
        }
    }
    return token_list;
}

string char2alpha(char ch) {
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '-')return "letter";
    if (ch >= '0' && ch <= '9')return "number";
    return string(1, ch);
}

void get_token(FA DFA, string file_line, int &pos, int line_id, token &word) {
    int begin_pos = pos;
    State cur;
    for (auto i: DFA.S_states) {
        cur = i;
    }
    while (true) {
        if (DFA.f_transition.find(make_pair(cur, string(1, file_line[pos]))) != DFA.f_transition.end()) {
            set<State > set_cur = DFA.f_transition[make_pair(cur, string(1, file_line[pos]))];
            for (auto i: set_cur) {
                cur = i;
            }
            pos++;
        } else if (DFA.f_transition.find(make_pair(cur, char2alpha(file_line[pos]))) != DFA.f_transition.end()) {
            set<State > set_cur = DFA.f_transition[make_pair(cur, char2alpha(file_line[pos]))];
            for (auto i: set_cur) {
                cur = i;
            }
            pos++;
        } else break;
    }
    if (DFA.E_states.find(cur) != DFA.E_states.end()) {
        word.line_id = line_id;
        word.value = file_line.substr(begin_pos, pos - begin_pos);
    }
    //error todo
}

template<typename T>
void printElement(T t, const int &width) {
    cout << left << setw(width) << t;
}

string symbol2string(SYMBOLS s) {
    switch (s) {
        case KEYWORD:
            return "keyword";
        case IDENTIFIER:
            return "identifier";
        case CONST:
            return "const";
        case LIMITER:
            return "limiter";
        case OPERATOR:
            return "operator";
        default:
            return "other";
    }
}

void print_tokens(vector<token> tokens) {
    int table_width = 10;
    printElement("line_id", table_width);
    printElement("content", table_width);
    printElement("symbol", table_width);
    cout << endl;
    for (auto i = tokens.begin(); i < tokens.end(); i++) {
        printElement(i->line_id, table_width);
        printElement(i->value, table_width);
        printElement(symbol2string(i->symbol), table_width);
        cout << endl;
    }
}