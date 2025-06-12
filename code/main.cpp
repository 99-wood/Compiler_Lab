#include <bits/stdc++.h>
#include "lexer.h"
#include "parser.h"
#include "quad.h"
#include "target.h"
#include "node.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

std::string readFileToString(const std::string &fileName) {
    const std::ifstream file(fileName);
    std::ostringstream buffer;
    if(!file){
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        exit(1);
    }
    buffer << file.rdbuf();
    return buffer.str();
}
std::string getLineFromString(const std::string& content, size_t lineNumber) {
    assert(lineNumber > 0);
    --lineNumber;
    std::istringstream stream(content);
    std::string line;
    size_t currentLine = 0;

    while (std::getline(stream, line)) {
        if (currentLine == lineNumber) {
            return line;
        }
        ++currentLine;
    }

    return "";  // 返回空字符串表示行号超出范围
}
std::pair<int, int> parseErrorLineCol(const std::string& message) {
    int line = -1, column = -1;
    sscanf(message.c_str(), "Wrong on %d line %d column.", &line, &column);
    return {line, column};
}
int extractErrTokenNumber(const std::string& message) {
    const std::regex pattern(R"(Wrong on (\d+) token)");
    std::smatch match;

    if (std::regex_search(message, match, pattern)) {
        return std::stoi(match[1]);  // 提取第一个捕获组里的数字
    } else {
        return -1;  // 找不到就返回 -1，表示错误
    }
}
int extractWarnTokenNumber(const std::string& message) {
    const std::regex pattern(R"(Warn on (\d+) token)");
    std::smatch match;

    if (std::regex_search(message, match, pattern)) {
        return std::stoi(match[1]);  // 提取第一个捕获组里的数字
    } else {
        return -1;  // 找不到就返回 -1，表示错误
    }
}

int main() {
    const string code = readFileToString(string("..\\test.txt"));
    cout << code << endl;
    vector<lexer::Token> tokens;
    vector<string> err;
    vector<int> ci;
    vector<float> cf;
    vector<string> I;
    vector<std::pair<int, int>> tokenLoacation;
    lexer::scan(code, tokens, I, ci, cf, err, tokenLoacation);
    assert(tokenLoacation.size() == tokens.size());
    if(!err.empty()){
        for(auto s : err){
            std::cerr << s << endl;
            const auto [l, c] =parseErrorLineCol(s);
            const string errCode = getLineFromString(code, l);
            cout << errCode << endl;
            constexpr int tokenLen = 1;
            for(int i = 1; i < c - tokenLen; ++i) cout << " ";
            for(int i = 1; i <= tokenLen; ++i) cout << "\033[31m" << "~" << "\033[0m";
            cout << "^";
            cout << endl;
        }
        return 0;
    }


    cout << "token:" << endl;
    for(int i = 0, line = 1; i < tokens.size(); ++i){
        const lexer::Token token = tokens[i];
        const auto [l, c] = tokenLoacation[i];
        while(line < l){
            cout << endl;
            ++line;
        }
        cout << token << " ";
    }
    // for(auto token : tokens) cout << " " << token;
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
    parser.changeStr(tokens, ci, cf, I);
    auto tokenToSize = [&](const lexer::Token& token) {
        if(token.type == lexer::TokenType::I) return I[token.id - 1].size();
        if(token.type == lexer::TokenType::K) return lexer::K[token.id - 1].size();
        if(token.type == lexer::TokenType::P) return lexer::P[token.id - 1].size();
        else return static_cast<size_t>(1);
    };
    if(!parser.run()){

        for(const auto parseErr = parser.getErr(); const auto &s : parseErr){
            cout << s << endl;
            const int x = extractErrTokenNumber(s);
            if(x == -1) continue;
            const auto [l, c] = tokenLoacation[x - 1];
            const string errCode = getLineFromString(code, l);
            cout << errCode << endl;
            const int tokenLen = tokenToSize(tokens[x - 1]);
            for(int i = 1; i < c - tokenLen; ++i) cout << " ";
            for(int i = 1; i <= tokenLen; ++i) cout << "\033[31m" << "~" << "\033[0m";
            cout << "^";
            cout << endl;
        }
    }
    else{
        cout << "------------------------------Warn------------------------------\n";
        for(const auto parseWarn = parser.getWarn(); const auto &s : parseWarn){
            std::cout << "\033[33m" << s << endl << "\033[0m";
            const int x = extractWarnTokenNumber(s);
            if(x == -1) continue;
            const auto [l, c] = tokenLoacation[x - 1];
            const string errCode = getLineFromString(code, l);
            cout << errCode << endl;
            const int tokenLen = tokenToSize(tokens[x - 1]);
            for(int i = 1; i < c - tokenLen; ++i) cout << " ";
            for(int i = 1; i <= tokenLen; ++i) cout << "\033[33m" << "~" << "\033[0m";
            cout << "^";
            cout << endl;
        }
        cout << "--------------------------Symbol Table--------------------------\n";
        const auto symbolTable = parser.getGlobal();
        cout<< std::right << std::setw(10) << "name"
                        << std::right << std::setw(10) << "token"
                        << std::right << std::setw(10) << "type"
                        << std::right << std::setw(10) << "kind" << endl;
        for(const auto &[token, type, kind, ptr, id] : symbolTable){
            cout<< std::right << std::setw(10) << I[token.id - 1]
                << std::right << std::setw(10) << token.toString()
                << std::right << std::setw(10) << type->toString()
                << std::right << std::setw(10) << kind << endl;
        }
        cout << "------------------------Intermediate Code-----------------------\n";
        const vector<quad::Quad> mid = parser.getMid();
        for(size_t i = 0; i < mid.size(); ++i){
            cout << i << ": " << mid[i] << endl;
        }
        cout << "-----------------Optimized Intermediate Code--------------------\n";
        node::Optimizer optimizer(mid);
        optimizer.run();
        const auto optimizedMid = optimizer.getRes();
        for(size_t i = 0; i < optimizedMid.size(); ++i){
            cout << i << ": " << optimizedMid[i] << endl;
        }
        cout << "---------------Intermediate Code Running Results----------------\n";
        const vector<quad::Quad> targetCode = parser.getRes();
        for(size_t i = 0; i < targetCode.size(); ++i){
            cout << i << ": " << targetCode[i] << endl;
        }

        cout << "----------Optimized Intermediate Code Running Results-----------\n";
        quad::QuadRunner runner(mid);
        runner.run();
        for(const auto mem = runner.getRes();
                const auto [token, res] : mem){
            cout << token << " = "
                 << std::right << std::setw(10) << res << " or "
                 << std::right << std::setw(10) << std::bit_cast<float>(res) << endl;
        }

        cout << "-------------------------Running Results------------------------\n";
        quad::QuadRunner optimizedRunner(optimizedMid);
        optimizedRunner.run();
        for(const auto mem = optimizedRunner.getRes();
                const auto [token, res] : mem){
            cout << token << " = "
                 << std::right << std::setw(10) << res << " or "
                 << std::right << std::setw(10) << std::bit_cast<float>(res) << endl;
        }

        cout << "-------------------Target Code Running Results------------------\n";
        target::TargetRuner targetRuner(targetCode);
        if(targetRuner.run() != 0){
            cout << "RUNTIME_ERR";
        }
        else{
            const int res = targetRuner.getFirst();
            cout << "ans" << " = "
                    << std::right << std::setw(10) << res << " or "
                    << std::right << std::setw(10) << std::bit_cast<float>(res) << endl;
        }

    }
    // cout << "Enter to close screen";
    // getchar();
    return 0;
}
