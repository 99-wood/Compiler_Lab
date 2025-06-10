#include <bits/stdc++.h>
#include "lexer.h"
#include "parser.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
std::string readFileToString(const std::string& fileName) {
    const std::ifstream file(fileName);
    std::ostringstream buffer;
    if (!file) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        exit(1);
    }
    buffer << file.rdbuf();
    return buffer.str();
}
int main() {
    const string code = readFileToString(string("..\\test.txt"));
    vector<lexer::Token> ans;
    vector<string> err;
    vector<int> ci;
    vector<float> cf;
    vector<string> I;
    lexer::scan(code, ans, I, ci, cf, err);

    for(auto s : err) std::cerr << s << endl;

    cout << "token:";
    for(auto token : ans) cout << " " << token;
    cout << endl;

    cout << "I:";
    for(auto str : I) cout << " " << str;
    cout << endl;

    cout << "CI:";
    for(auto token : ci) cout << " " << token;
    cout << endl;

    cout << "CF:";
    for(float x : cf) cout << " " << x;
    cout << endl;

    parser::Parser parser;
    parser.changeStr(ans, ci, cf, I);
    if(!parser.run()){
        for(const auto parseErr = parser.getErr(); const auto& s : parseErr) cout << s << endl;
    }
    else{
        const auto res = parser.getRes();
        for(size_t i = 0; i < res.size(); ++i){
            cout << i << ": " << res[i] << endl;
        }
        const auto mid = parser.getMid();
        for(size_t i = 0; i < mid.size(); ++i){
            cout << i << ": " << mid[i] << endl;
        }
    }
    for(const auto parseWarn = parser.getWarn(); const auto& s : parseWarn){
        std::cout << "\033[33m" << s << endl << "\033[0m";
    }
    cout << "Enter to close screen";
    getchar();
    return 0;
}