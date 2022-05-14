// #include "parser.h"
//
//
// Context_free_grammar::Context_free_grammar(string path) {
//     /*
//      *
//      * [start_State]    表示初始态
//      * <State>          表示状态
//      * 'Alpha'          表示输入字符
//      * ''               表示空字符epsilon
//      * :=               表示左部定义于右部
//     */
//     ifstream in_rules;
//     in_rules.open(path, ios::in);
//     string file_line;
//     while (getline(in_rules, file_line)) {
//         int pos = 0;
//         Sigma left;
//         vector<Sigma> right;
//         bool is_identity_left = false;
//         while (pos < file_line.size()) {
//             char ch = file_line[pos];
//             if (ch == '[') {
//                 int end_pos = file_line.find(']', pos + 1);
//                 start_symbol = file_line.substr(pos + 1, end_pos - pos - 1);
//                 non_terminal_symbols.insert(start_symbol);
//                 left = start_symbol;
//                 is_identity_left = true;
//                 pos = end_pos + 1;
//                 continue;
//             }
//             if (ch == '<') {
//                 int end_pos = file_line.find('>', pos + 1);
//                 Sigma ns = file_line.substr(pos + 1, end_pos - pos - 1);
//                 non_terminal_symbols.insert(ns);
//                 if (!is_identity_left)left = ns;
//                 else right.push_back(ns);
//                 pos = end_pos + 1;
//                 continue;
//             }
//             if (ch == '\'') {
//                 int end_pos = file_line.find('\'', pos + 1);
//                 Sigma ts = file_line.substr(pos + 1, end_pos - pos - 1);
//                 terminal_symbols.insert(ts);
//                 right.push_back(ts);
//                 pos = end_pos + 1;
//                 continue;
//             }
//         }
//         if (production_rules.find(left) == production_rules.end())
//             production_rules[left] = {right};
//         else
//             production_rules[left].insert(right);
//     }
// }
//
// bool Context_free_grammar::FIRST_function(Sigma x, set<Sigma> ret) {//x终结符和非终结符
//
//     if (terminal_symbols.find(x) != terminal_symbols.end()) {//终结符
//         ret.insert(x);
//     }
//     if (non_terminal_symbols.find(x) != non_terminal_symbols.end()) {//非终结符
//         set<Sigma> set_first;
//         for (auto beta: production_rules[x]) {
//             if (terminal_symbols.find(beta[0]) != terminal_symbols.end())
//                 set_first.insert(beta[0]);
//                 // else if (beta[0] == "") {
//                 //     set_first.insert("");
//                 //     have_epsilon = true;
//                 // }
//             else {
//                 set<Sigma> s_t = FIRST_function(beta[0]);
//                 for (auto s: s_t)set_first.insert(s);
//             }
//         }
//     }
//
// }