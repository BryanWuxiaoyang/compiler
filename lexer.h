//.h头文件中，只能存在变量或者函数的声明，而不要放定义
#include <bits/stdc++.h>

// struct State {
//     std::set<std::string> state;
//
//     State(std::set<std::string> s) {
//         state = s;
//     }
// };
// struct Alpha {
//     std::string alpha;
//
//     Alpha(std::string a) {
//         alpha = a;
//     }
// };
// State END_STATE({"end state"});
// Alpha EPSILON("");


#define State std::set<std::string>
#define Alpha std::string
// const State END_STATE = {"end state"};
// const Alpha EPSILON = "";
#define END_STATE {"EndState"}
#define EPSILON  ""

enum SYMBOLS {
    KEYWORD, IDENTIFIER, CONST, LIMITER, OPERATOR,
    Default, Comment
};


struct FA {

    std::set<State > states;//状态集
    std::set<Alpha> sigma_input_characters;//输入符号表
    std::map<std::pair<State, Alpha>, std::set<State > > f_transition;//状态转移函数
    std::set<State > S_states;//初始状态集(DFA唯一)
    std::set<State > E_states;//终态集

};

void print_FA(FA x);

struct Regular_grammar {
    std::set<State > non_terminal_symbols;//非终结符
    std::set<Alpha> terminal_symbols;//终结符
    std::set<State > start_symbol;//识别符或开始符，属于非终结符
    // std::set<std::pair<State, Alpha>> first_rules;//形如A::=a的规则
    // std::set<std::pair<State, std::pair<Alpha, State>>> second_rules;//形如A::=aB的规则
    std::map<std::pair<State, Alpha>, std::set<State > > rules;//形如A::=aB的规则set < < A , a > B>
};

struct token {
    SYMBOLS symbol;
    std::string value;
    int line_id;
};

std::string symbol2string(SYMBOLS s);

Regular_grammar input_rules2Regular_grammar(std::string path);

FA Regular_grammar2NFA(Regular_grammar G);

FA NFA2DFA(FA NFA);

void
generate_DFA_state(FA NFA, FA &DFA, std::map<std::set<State >, bool> &visited, std::set<State > current_set_states);

std::set<State > move(FA NFA, std::set<State > from_set_states, Alpha alpha);

std::set<State > varepsilon_closure(FA, std::set<State >);

std::vector<token> get_token_list(FA, std::string path);

void get_token(FA DFA, std::string file_line, int &pos, int line_id, token &word);

void print_tokens(std::vector<token>);