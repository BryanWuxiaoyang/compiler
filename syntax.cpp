
//#include "lexer.cpp"
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
    token token;
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


class Parser {
    int idx = 0;

    vector<Symbol*> stream;

private:
    Production *chooseProduction(Symbol* head, vector<Production *> &productions) {
        Production* ret = nullptr;
        for (Production* production: productions){
            set<Symbol *> *symbols = production->transferSymbols;
            Symbol *symbol = stream[idx];
            if (symbols->count(symbol) > 0){
                ret = production;
                break;
            }
        }
        if(ret == nullptr){
            cout << "no matched symbol: " << head->symbolName << ", at line: " << stream[idx]->token.line_id << endl;
            assert(false);
        }
        return ret;
    }

public:
    explicit Parser(vector<Symbol *> stream) {
        this->stream = std::move(stream);
    }

    SyntaxSymbol *parse(Symbol* symbol) {
        if (symbol->isTerminal) {
            assert(idx < stream.size());
            assert(symbol == stream[idx]);
            return new SyntaxSymbol(symbol, stream[idx++]);
        } else {
            SyntaxSymbol *root = new SyntaxSymbol(symbol);
            root->attachment = symbol;

            Production *production = chooseProduction(symbol, symbol->productions);
            for (Symbol* nextSymbol : production->tail){
                SyntaxSymbol *child = parse(nextSymbol);
                root->addChild(child);
            }
            return root;
        }
    }

};

map<string, Symbol *> symbolTable;
Symbol *startSymbol = nullptr;

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
//    c2("EOS");

    startSymbol = symbolTable[stringStart->symbolName];

    clearFlags();
    generateFirstSymbols(startSymbol);
    clearFlags();
    generateFollowSymbols(startSymbol);
    clearFlags();
    generateTransferSymbols(startSymbol);
}

void printSet(set<Symbol*>* s){
    for(Symbol* symbol : *s){
        cout << symbol->symbolName << " ";
    }
}

void printSymbolTable(){
    cout << "symbol table:" << endl;
    for(auto entry: symbolTable){
        Symbol* symbol = entry.second;
        cout << symbol->symbolName << " = " << (symbol->isTerminal ? "t" : "nt");
        cout << endl;
        cout << "\t\tfirst symbols: ";
        printSet(symbol->firstSymbols);
        cout << endl;
        cout << "\t\tfollow symbols: ";
        printSet(symbol->followSymbols);
        cout << endl;

        for(auto production: symbol->productions){
            cout << "\t" << symbol->symbolName << " = ";
            for(auto elementSymbol: production->tail){
                cout << ' ' << elementSymbol->symbolName;
            }
            cout << endl;
            cout << "\t\tfirst symbols: ";
            printSet(production->getFirstSymbols());
            cout << endl;
            cout << "\t\ttransfer symbols: ";
            printSet(production->transferSymbols);
            cout << endl;
        }
    }
    cout << endl;
}

int main() {
    // string lexer_rules_path = "test_lexerRules.txt";
    Regular_grammar G = input_rules2Regular_grammar("lexer_rules.txt");
    init("production.txt");
    printSymbolTable();
    FA NFA = Regular_grammar2NFA(G);
    FA DFA = NFA2DFA(NFA);
    // print_FA(DFA);
    string program_path = "program.txt";
    vector<token> token_list = get_token_list(DFA, program_path);
    // print_tokens(token_list);

    vector<Symbol*> stream;
    cout << "tokens: ";
    for (const auto& t: token_list){
        Symbol* symbol = getTerminalSymbol(t);
        symbol->token = t;
        stream.push_back(symbol);
        cout << "(" << symbol2string(t.symbol) << ", \"" << t.value << "\")\t";
    }
    cout << endl;

    cout << "stream: ";
    for(Symbol* symbol: stream){
        cout << symbol->symbolName << "\t";
    }
    cout << endl;

    Parser parser(stream);
    SyntaxSymbol *syntaxTree = parser.parse(startSymbol);
    return 0;
}