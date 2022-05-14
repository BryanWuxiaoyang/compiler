
// #include "lexer.h"
// #include "syntax.cpp"
//
// using namespace std;


// int main() {
//     // string lexer_rules_path = "test_lexerRules.txt";
//     Regular_grammar G = input_rules2Regular_grammar("lexer_rules.txt");
//     FA NFA = Regular_grammar2NFA(G);
//     FA DFA = NFA2DFA(NFA);
//     // print_FA(DFA);
//     string program_path = "program.txt";
//     vector<token> token_list = get_token_list(DFA, program_path);
//     // print_tokens(token_list);
//
//
//     init("production.txt");
//
//
//     vector<Token *> stream;
//     for (auto t: token_list)
//         stream.push_back(new Token(t));
//
//     Parser parser(stream);
//     SyntaxSymbol *syntaxTree = parser.parse();
//     return 0;
// }





//
// int main() {
//     init("production.txt");
//     vector<Token *> stream;
//     Parser parser(stream);
//     SyntaxSymbol *syntaxTree = parser.parse();
//     return 0;
// }