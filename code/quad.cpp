//
// Created by admin on 2025/6/10.
//

#include "quad.h"

#include <codecvt>

namespace quad{
    bool Quad::isToken(const string &arg) {
        assert(!arg.empty());
        return arg[0] == '(';
    }

    bool Quad::isEmpty(const string &arg) {
        assert(!arg.empty());
        return arg[0] == '_';
    }

    Token Quad::getToken(const string &arg) {
        assert(isToken(arg));
        Token token{};
        std::stringstream ss;
        ss << arg;
        ss >> token;
        return token;
    }

    int Quad::getInt(const string &arg) {
        assert(!isToken(arg) && !isEmpty(arg));
        int val;
        std::stringstream ss;
        ss << arg;
        ss >> val;
        return val;
    }

    std::pair<bool, int> QuadRunner::execute(MemDevice &mem) {
        while(currentArg < args.size()){
            const auto &[op, arg1, arg2, result] = args[currentArg];
            ++currentArg;
            if(op == "JMP"){
                currentArg = Quad::getInt(arg1);
            }
            else if(op == "MOV"){
                Token dest = Quad::getToken(arg2);
                if(Quad::isToken(arg1)){
                    Token src = Quad::getToken(arg1);
                    mem[dest] = mem[src];
                }
                else{
                    mem[dest] = Quad::getInt(arg1);
                }
            }
            else if(op == "RET"){
                if(Quad::isEmpty(arg1)) return {false, 0};
                return {true, Quad::isToken(arg1) ? mem[Quad::getToken(arg1)] : Quad::getInt(arg1)};
            }
            else if(op == "CALL"){
                std::map<Token, int, cmp> newMem;
                MemDevice newMemDivice(mem.global, newMem);
                std::stringstream ss;
                ss << arg2;
                int n;
                ss >> n;
                for(int i = 1; i <= n; ++i){
                    Token src{};
                    Token dest{};
                    ss >> src;
                    char c;
                    ss >> c;
                    assert(c == ':');
                    ss >> dest;
                    assert(!global.contains(src));
                    newMem[dest] = mem[src];
                }
                const int to = Quad::getInt(arg1);
                const auto tmp = currentArg;
                currentArg = to;
                const auto [haveReturnValue, returnValue] = execute(newMemDivice);
                currentArg = tmp;
                if(Quad::isEmpty(result)){
                    assert(!haveReturnValue);
                }
                else{
                    assert(haveReturnValue);
                    Token dest = Quad::getToken(result);
                    mem[dest] = returnValue;
                }
            }
            else if(op == "JZ"){
                const int to = Quad::getInt(arg2);
                if(Quad::isToken(arg1)){
                    if(const Token src = Quad::getToken(arg1); !mem[src]) currentArg = to;
                }
                else{
                    if(const int val = Quad::getInt(arg1); !val) currentArg = to;
                }
            }
            else if(op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" ||
                    op == "ADDF" || op == "SUBF" || op == "MULF" || op == "DIVF" ||
                    op == "G" || op == "GE" || op == "L" || op == "LE" || op == "E" || op == "NE" ||
                    op == "GF" || op == "GEF" || op == "LF" || op == "LEF" || op == "EF" || op == "NEF" ||
                    op == "AND" || op == "OR"){
                const Token res = Quad::getToken(result);
                if(Quad::isToken(arg1)){
                    const Token x = Quad::getToken(arg1);
                    if(Quad::isToken(arg2)){
                        const Token y = Quad::getToken(arg2);
                        mem[res] = genBinaryOperation(op)(mem[x], mem[y]);
                    }
                    else{
                        const int y = Quad::getInt(arg2);
                        mem[res] = genBinaryOperation(op)(mem[x], y);
                    }
                }
                else{
                    const int x = Quad::getInt(arg1);
                    if(Quad::isToken(arg2)){
                        const Token y = Quad::getToken(arg2);
                        mem[res] = genBinaryOperation(op)(x, mem[y]);
                    }
                    else{
                        const int y = Quad::getInt(arg2);
                        mem[res] = genBinaryOperation(op)(x, y);
                    }
                }
            }
            else if(op == "NOT" ||
                    op == "I2I" || op == "I2F" || op == "I2C" || op == "I2B" ||
                    op == "F2I" || op == "F2F" || op == "F2C" || op == "F2B" ||
                    op == "C2I" || op == "C2F" || op == "C2C" || op == "C2B" ||
                    op == "B2I" || op == "B2F" || op == "B2C" || op == "B2B" ){
                const Token dest = Quad::getToken(arg2);
                if(Quad::isToken(arg1)){
                    const Token src = Quad::getToken(arg1);
                    mem[dest] = genUnaryOperation(op)(mem[src]);
                }
                else{
                    const int src = Quad::getInt(arg1);
                    mem[dest] = genUnaryOperation(op)(src);
                }
            }
            else if(op == "ERR"){
                throw std::runtime_error("ERR on " + std::to_string(currentArg) + ".");
            }
            else assert(0);
        }
        return {false, 0};
    }

    std::function<int(int, int)> QuadRunner::genBinaryOperation(const string &op) {
        if(op == "ADD"){
            return [&](const int &x, const int &y){return x + y;};
        }
        if(op == "SUB"){
            return [&](const int &x, const int &y){return x - y;};
        }
        if(op == "MUL"){
            return [&](const int &x, const int &y){return x * y;};
        }
        if(op == "DIV"){
            return [&](const int &x, const int &y){assert(y != 0); return x / y;};
        }

        if(op == "AND"){
            return [&](const int &x, const int &y){return static_cast<bool>(x) && static_cast<bool>(y);};
        }
        if(op == "OR"){
            return [&](const int &x, const int &y){return static_cast<bool>(x) || static_cast<bool>(y);};
        }

        if(op == "ADDF"){
            return [&](const int &x, const int &y){return std::bit_cast<int>(std::bit_cast<float>(x) + std::bit_cast<float>(y));};
        }
        if(op == "SUBF"){
            return [&](const int &x, const int &y){return std::bit_cast<int>(std::bit_cast<float>(x) - std::bit_cast<float>(y));};
        }
        if(op == "MULF"){
            return [&](const int &x, const int &y){return std::bit_cast<int>(std::bit_cast<float>(x) * std::bit_cast<float>(y));};
        }
        if(op == "DIVF"){
            return [&](const int &x, const int &y){return std::bit_cast<int>(std::bit_cast<float>(x) / std::bit_cast<float>(y));};
        }

        if(op == "G"){
            return [&](const int &x, const int &y){return x > y;};
        }
        if(op == "GE"){
            return [&](const int &x, const int &y){return x >= y;};
        }
        if(op == "L"){
            return [&](const int &x, const int &y){return x < y;};
        }
        if(op == "LE"){
            return [&](const int &x, const int &y){return x <= y;};
        }
        if(op == "E"){
            return [&](const int &x, const int &y){return x == y;};
        }
        if(op == "NE"){
            return [&](const int &x, const int &y){return x != y;};
        }

        if(op == "GF"){
            return [&](const int &x, const int &y){return std::bit_cast<float>(x) > std::bit_cast<float>(y);};
        }
        if(op == "GEF"){
            return [&](const int &x, const int &y){return std::bit_cast<float>(x) >= std::bit_cast<float>(y);};
        }
        if(op == "LF"){
            return [&](const int &x, const int &y){return std::bit_cast<float>(x) < std::bit_cast<float>(y);};
        }
        if(op == "LEF"){
            return [&](const int &x, const int &y){return std::bit_cast<float>(x) <= std::bit_cast<float>(y);};
        }
        if(op == "EF"){
            return [&](const int &x, const int &y){return std::bit_cast<float>(x) == std::bit_cast<float>(y);};
        }
        if(op == "NEF"){
            return [&](const int &x, const int &y){return std::bit_cast<float>(x) != std::bit_cast<float>(y);};
        }
        assert(0);
    }

    std::function<int(int)> QuadRunner::genUnaryOperation(const string &op) {
        if(op == "NOT"){
            return [&](const int &x){return std::bit_cast<int>(x);};
        }
        if(op == "I2F"){
            return [&](const int &x){return std::bit_cast<int>(static_cast<float>(x));};
        }
        if(op == "C2F"){
            return [&](const int &x){return std::bit_cast<int>(static_cast<float>(x));};
        }
        if(op == "B2F"){
            return [&](const int &x){return std::bit_cast<int>(static_cast<float>(x));};
        }
        if(op == "F2I"){
            return [&](const int &x){return static_cast<int>(std::bit_cast<float>(x));};
        }
        if(op == "C2I"){
            return [&](const int &x){return static_cast<int>(x);};
        }
        if(op == "B2I"){
            return [&](const int &x){return static_cast<int>(x);};
        }
        if(op == "I2C"){
            return [&](const int &x){return static_cast<char>(x);};
        }
        if(op == "F2C"){
            return [&](const int &x){return static_cast<char>(std::bit_cast<float>(x));};
        }
        if(op == "B2C"){
            return [&](const int &x){return static_cast<char>(x);};
        }
        if(op == "I2B"){
            return [&](const int &x){return static_cast<bool>(x);};
        }
        if(op == "F2B"){
            return [&](const int &x){return static_cast<bool>(std::bit_cast<float>(x));};
        }
        if(op == "C2B"){
            return [&](const int &x){return static_cast<bool>(x);};
        }
        assert(0);
    }

    void QuadRunner::run() {
        global.clear();
        currentArg = 0;
        MemDevice memDevice(global, global);
        execute(memDevice);
        finish = true;
    }

    std::map<Token, int, QuadRunner::cmp> QuadRunner::getRes() const {
        if(!finish) throw std::runtime_error("Have not run or finish.");
        return global;
    }
} // quad
