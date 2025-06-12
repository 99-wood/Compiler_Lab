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
        for(const auto parseErr = parser.getErr(); const auto &s : parseErr) cout << s << endl;
    }
    else{
        cout << "---------------------------Target Code--------------------------\n";
        const vector<quad::Quad> targetCode = parser.getRes();
        for(size_t i = 0; i < targetCode.size(); ++i){
            cout << i << ": " << targetCode[i] << endl;
        }
        cout << "------------------------Intermediate Code------------------------\n";
        const vector<quad::Quad> mid = parser.getMid();
        for(size_t i = 0; i < mid.size(); ++i){
            cout << i << ": " << mid[i] << endl;
        }
        cout << "-------------------------Running Results-------------------------\n";
        quad::QuadRunner runner(mid);
        runner.run();
        for(const auto mem = runner.getRes();
            const auto [token, res] : mem){
            cout << token << " = "
                    << std::right << std::setw(10) << res << " or "
                    << std::right << std::setw(10) << std::bit_cast<float>(res) << endl;
        }
        cout << "-----------------Optimized Intermediate Code---------------------\n";
        node::Optimizer optimizer(mid);
        optimizer.run();
        const auto optimizedMid = optimizer.getRes();
        for(size_t i = 0; i < optimizedMid.size(); ++i){
            cout << i << ": " << optimizedMid[i] << endl;
        }
        cout << "-------------------------Running Results-------------------------\n";

        quad::QuadRunner optimizedRunner(optimizedMid);
        optimizedRunner.run();
        for(const auto mem = optimizedRunner.getRes();
            const auto [token, res] : mem){
            cout << token << " = "
                    << std::right << std::setw(10) << res << " or "
                    << std::right << std::setw(10) << std::bit_cast<float>(res) << endl;
        }
        cout << "-------------------Target Code Running Results-------------------\n";
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
    for(const auto parseWarn = parser.getWarn(); const auto &s : parseWarn){
        std::cout << "\033[33m" << s << endl << "\033[0m";
    }
    // cout << "Enter to close screen";
    // getchar();
    return 0;
}
