
//#include "lexer.cpp"
#include <utility>

#include "lexer.h"

using namespace std;

class Token;

class Production;

class Symbol;


// map<string, Symbol *> symbolTable;

Symbol *c1(const string &name);

Symbol *c2(const string &name);

class Production {
public:
    Symbol *head;
    vector<Symbol *> tail;

public:
    bool flag = false;
    vector<set<Symbol *>*> firstSymbolsList;
    set<Symbol *> *transferSymbols = nullptr;

    Production(Symbol *head, vector<Symbol *> tail) {
        this->head = head;
        this->tail = std::move(tail);
    }

    set<Symbol*>* getFirstSymbols(int startIdx = 0){
        return startIdx >= tail.size() ? new set<Symbol*>() : firstSymbolsList[startIdx];
    }
};

class Symbol {
public:
    bool isTerminal;
    string symbolName;
    vector<Production *> productions;
public:
    bool flag = false;
    set<Symbol *> *firstSymbols = nullptr;
    set<Symbol *> *followSymbols = nullptr;

public:
    Symbol(bool isTerminal, string symbolName) {
        this->isTerminal = isTerminal;
        this->symbolName = std::move(symbolName);
    }

    void addProduction(Production *production) {
        productions.push_back(production);
    }

    set<Symbol*>* getFirstSymbols(){
        return firstSymbols;
    }
};

class SyntaxSymbol {
public:
    Symbol *symbol;
    void *attachment;
    vector<SyntaxSymbol *> children;

    explicit SyntaxSymbol(Symbol *symbol, void *attachment = nullptr) {
        this->symbol = symbol;
        this->attachment = attachment;
    }

    void addChild(SyntaxSymbol *child) {
        children.push_back(child);
    }
};


void print(set<Symbol*>* s);
void print(vector<Symbol*> v);
void print();
void print(Production* production);

map<string, Symbol *> symbolTable;
Symbol *startSymbol = nullptr;

class Parser {
    int idx = 0;

    vector<Symbol*> stream;
    vector<token> tokens;

private:
    vector<Production *> chooseProduction(Symbol* symbol) {
        vector<Production*> ret;
        for (Production* production: symbol->productions){
            if (production->transferSymbols->count(stream[idx]) > 0){
                ret.push_back(production);
            }
        }
        return ret;
    }

public:
    vector<pair<Symbol*,token>> errors;

    explicit Parser(vector<Symbol *> stream, vector<token> tokens) {
        this->stream = std::move(stream);
        this->tokens = std::move(tokens);
    }

    bool check(){
        return idx == stream.size();
    }

    SyntaxSymbol *parse(Symbol* symbol) {
        if (symbol->isTerminal) {
            if(symbol == symbolTable["epsilon"]){
                return new SyntaxSymbol(symbol, nullptr);
            }
            else if(symbol == stream[idx]){
                return new SyntaxSymbol(symbol, stream[idx++]);
            }
            else{
                return nullptr;
            }
        } else {
            SyntaxSymbol * root = nullptr;
            int curIdx = idx;
            Production * chosenProduction = nullptr;
            for(Production *production: chooseProduction(symbol)){
//                print(production);
                root = new SyntaxSymbol(symbol);
                root->attachment = symbol;
                chosenProduction = production;
                for (Symbol* nextSymbol : production->tail){
                    SyntaxSymbol *child = parse(nextSymbol);
                    if(child == nullptr) {
                        root = nullptr;
                        chosenProduction = nullptr;
                        idx = curIdx;
                        break;
                    }
                    root->addChild(child);
                }
                if(root != nullptr) break;
            }
            if(chosenProduction != nullptr) print(chosenProduction);
            else{
                errors.push_back(pair<Symbol*, token>(stream[idx], tokens[idx]));
            }
            return root;
        }
    }

};


void clearFlags(){
    for(auto entry: symbolTable){
        Symbol* symbol = entry.second;
        symbol->flag = false;
        for(Production* production : symbol->productions){
            production->flag = false;
        }
    }
}

void generateFirstSymbols(Symbol*);
void generateFirstSymbols(Production*);

void generateFirstSymbols(Symbol *symbol){
    if(symbol->flag) return;
    symbol->flag = true;
//    cout << symbol->symbolName << endl;
    symbol->firstSymbols = new set<Symbol*>();
    if(symbol->isTerminal) symbol->firstSymbols->insert(symbol);
    else{
        for(Production* production: symbol->productions){
            generateFirstSymbols(production);
            symbol->firstSymbols->insert(production->getFirstSymbols()->begin(), production->getFirstSymbols()->end());
        }
    }
}

void generateFirstSymbols(Production* production){
    if(production->flag) return;
    production->flag = true;

    for(int startIdx = 0; startIdx < production->tail.size(); startIdx++){
        auto* firstSet = new set<Symbol*>();

        for(int idx = startIdx; idx < production->tail.size(); idx++){
            Symbol* symbol = production->tail[idx];
            generateFirstSymbols(symbol);
            firstSet->insert(symbol->firstSymbols->begin(), symbol->firstSymbols->end());
            if (firstSet->count(symbolTable["epsilon"]) == 0) break;
        }

        production->firstSymbolsList.push_back(firstSet);
    }

}

void _generateFollowSymbols(Symbol*);
void generateFollowSymbols(Symbol*);

void generateFollowSymbols(Symbol *symbol){
    assert(symbol == startSymbol);
    cout << symbol->symbolName << "\t";
    symbol->followSymbols = new set<Symbol*>();
    symbol->followSymbols->insert(symbolTable["EOS"]);
    _generateFollowSymbols(symbol);
}

void _generateFollowSymbols(Symbol *symbol){
    if(symbol->flag) return;
    symbol->flag = true;
    cout << symbol->symbolName << endl;
    for(Production* production: symbol->productions){
        for(int i = 0; i < production->tail.size(); i++){
            Symbol* sym = production->tail[i];
            if(sym->followSymbols == nullptr) sym->followSymbols = new set<Symbol*>();
            set<Symbol*>* follows = production->getFirstSymbols(i + 1);
            sym->followSymbols->insert(follows->begin(), follows->end());
            if(follows->empty() || follows->count(symbolTable["epsilon"]) > 0){
                sym->followSymbols->insert(symbol->followSymbols->begin(), symbol->followSymbols->end());
            }

            _generateFollowSymbols(sym);
        }
    }
}

void generateTransferSymbols(Symbol*);
void generateTransferSymbols(Production*);

void generateTransferSymbols(Symbol* symbol){
    if(symbol->flag) return;
    symbol->flag = true;
    for (Production* production: symbol->productions){
        generateTransferSymbols(production);
    }
}

void generateTransferSymbols(Production* production){
    if(production->flag) return;
    production->flag = true;

    production->transferSymbols = new set<Symbol*>();
    production->transferSymbols->insert(production->getFirstSymbols()->begin(), production->getFirstSymbols()->end());
    if(production->getFirstSymbols()->count(symbolTable["epsilon"]) > 0){
        production->transferSymbols->insert(production->head->followSymbols->begin(), production->head->followSymbols->end());
    }

    for(Symbol* symbol: production->tail){
        generateTransferSymbols(symbol);
    }
}

Symbol *create(const string &name, bool isTerminal) {
    Symbol *symbol;
    if (symbolTable.count(name)) symbol = symbolTable[name];
    else {
        symbol = new Symbol(isTerminal, name);
        symbolTable[name] = symbol;
    }
    return symbol;
}

Symbol *c1(const string &name) {
    return create(name, false);
}

Symbol *c2(const string &name) {
    return create(name, true);
}

Symbol* getTerminalSymbol(token t){
//        KEYWORD, IDENTIFIER, CONST, LIMITER, OPERATOR,
    Symbol* ret = nullptr;
    switch (t.symbol) {
        case KEYWORD:
            ret = symbolTable[t.value];
            break;
        case IDENTIFIER:
            ret = symbolTable["identifier"];
            break;
        case CONST:
            ret = symbolTable["const"];
            break;
        case LIMITER:
            ret = symbolTable[t.value];
            break;
        case OPERATOR:
            ret = symbolTable["operator"];
            break;
        default:
            assert(0);
    }
    return ret;
}

void init(string path) {
    ifstream in_rules;
    in_rules.open(path, ios::in);
    string file_line;
    Symbol *stringStart = nullptr;
    while (getline(in_rules, file_line)) {
        int pos = 0;
        Symbol *head;
        vector<Symbol *> tail;
        char ch = file_line[pos];
        int end_pos;
        if (ch == '[') {
            end_pos = file_line.find(']', pos + 1);
            head = c1(file_line.substr(pos + 1, end_pos - pos - 1));
            assert(stringStart == nullptr);
            stringStart = head;
        } else if (ch == '<') {
            end_pos = file_line.find('>', pos + 1);
            head = c1(file_line.substr(pos + 1, end_pos - pos - 1));
        }
        pos = end_pos + 1;

        while (pos < file_line.size()) {
            ch = file_line[pos];
            if (ch == '<') {
                end_pos = file_line.find('>', pos + 1);
                tail.push_back(c1(file_line.substr(pos + 1, end_pos - pos - 1)));
                pos = end_pos + 1;
            } else if (ch == '\'') {
                end_pos = file_line.find('\'', pos + 1);
                string nonN = file_line.substr(pos + 1, end_pos - pos - 1);
                if (nonN == "")nonN = "epsilon";

                tail.push_back(c2(nonN));
                pos = end_pos + 1;
            } else pos++;
        }

        head->addProduction(new Production(head, tail));
    }

    startSymbol = symbolTable[stringStart->symbolName];
    for(Production* production : startSymbol->productions){
        production->tail.push_back(c2("EOS"));
    }

    clearFlags();
    generateFirstSymbols(startSymbol);
    clearFlags();
    generateFollowSymbols(startSymbol);
    clearFlags();
    generateTransferSymbols(startSymbol);
}

void print(set<Symbol*>* s){
    for(Symbol* symbol : *s){
        cout << symbol->symbolName << " ";
    }
}
void print(vector<Symbol*>* v){
    for(Symbol* symbol: *v){
        cout << symbol->symbolName << " ";
    }
}

void print(){
    cout << "symbol table:" << endl;
    for(auto entry: symbolTable){
        Symbol* symbol = entry.second;
        cout << symbol->symbolName << " = " << (symbol->isTerminal ? "t" : "nt");
        cout << endl;
        cout << "\t\tfirst symbols: ";
        print(symbol->firstSymbols);
        cout << endl;
        cout << "\t\tfollow symbols: ";
        print(symbol->followSymbols);
        cout << endl;

        for(auto production: symbol->productions){
            cout << "\t" << symbol->symbolName << " = ";
            for(auto elementSymbol: production->tail){
                cout << ' ' << elementSymbol->symbolName;
            }
            cout << endl;
            cout << "\t\tfirst symbols: ";
            print(production->getFirstSymbols());
            cout << endl;
            cout << "\t\ttransfer symbols: ";
            print(production->transferSymbols);
            cout << endl;
        }
    }
    cout << endl;
}
void print(Production* production){
    cout << production->head->symbolName << " = ";
    print(&production->tail);
    cout << endl;
}

struct TransferEntry{
    Symbol* from;
    Symbol* to;
    Production* production;
};

vector<TransferEntry> generateTransferTable(){
    vector<TransferEntry> ret;
    for(auto p : symbolTable){
        Symbol* symbol = p.second;
        for(Production* production: symbol->productions){
            for(Symbol* symbol2: *(production->transferSymbols)){
                TransferEntry entry;
                entry.from = symbol;
                entry.to = symbol2;
                entry.production = production;
                ret.push_back(entry);
            }
        }
    }

    return ret;
}

void printTransferTable(){
    cout << "====================================" << endl;
    cout << "transfer table: " << endl;
    vector<TransferEntry> table = generateTransferTable();
    for(TransferEntry entry: table){
        cout << entry.from->symbolName << ", " << entry.to->symbolName << " ::= ";
        print(entry.production);
//        cout << endl;
    }
    cout << "========================================" << endl;
}

int main() {
    // string lexer_rules_path = "test_lexerRules.txt";
    Regular_grammar G = input_rules2Regular_grammar("lexer_rules.txt");
    print();
    FA NFA = Regular_grammar2NFA(G);
    FA DFA = NFA2DFA(NFA);
    // print_FA(DFA);
    string program_path = "program.txt";
    vector<token> token_list = get_token_list(DFA, program_path);
    // print_tokens(token_list);

    init("production.txt");
    printTransferTable();

    vector<Symbol*> stream;
    vector<token> tokens;
    cout << "tokens: ";
    for (const auto& t: token_list){
        Symbol* symbol = getTerminalSymbol(t);
        stream.push_back(symbol);
        tokens.push_back(t);
        cout << "(" << symbol2string(t.symbol) << ", \"" << t.value << "\")\t";
    }
    stream.push_back(c2("EOS"));
    cout << endl;

    cout << "stream: ";
    for(Symbol* symbol: stream){
        cout << symbol->symbolName << "\t";
    }
    cout << endl;
    Parser parser(stream, tokens);
    SyntaxSymbol *syntaxTree = parser.parse(startSymbol);
    if(parser.check()){
        cout << "YES" << endl;
    }
    else{
        cout << "NO" << endl;
        for(auto error: parser.errors){
            Symbol* symbol = error.first;
            token tok = error.second;
            cout << symbol->symbolName << ":" << tok.line_id << endl;
        }
    }
    return 0;
}