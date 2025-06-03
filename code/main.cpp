#include <bits/stdc++.h>
#include "lexer.h"
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    std::ostringstream buffer;
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
    }
    buffer << file.rdbuf();
    return buffer.str();
}
int main() {
    string s = readFileToString("..\\test.txt");
    vector<lexer::Token> ans;
    vector<string> err;
    vector<int> ci;
    vector<float> cf;
    lexer::scan(s, ans, ci, cf, err);
    for(auto s : err) std::cerr << s << endl;
    for(auto token : ans) cout << token << " ";
    cout << endl;
    for(float x : cf) cout << x << " ";
    cout << endl;
    return 0;
}