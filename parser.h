// #include<bits/stdc++.h>
//
// using namespace std;
// #define Sigma string
//
//
// class Context_free_grammar {
// public:
//     set<Sigma> non_terminal_symbols;//非终结符
//     set<Sigma> terminal_symbols;//终结符
//     Sigma start_symbol;//识别符或开始符，属于非终结符
//     map<Sigma, set<vector<Sigma> >> production_rules;//形如A::=\beta的产生式规则
//
//     map<set<vector<Sigma> >, set<Sigma>> production_first_set;
//
//
//     Context_free_grammar(string);
//
//     set<Sigma> FIRST_function(Sigma x, bool &have_epsilon);//x终结符和非终结符
//     set<Sigma> FOLLOW_function(Sigma x);//x非终结符
//     set<Sigma> SELECT_function(Production x);//x产生式
// };