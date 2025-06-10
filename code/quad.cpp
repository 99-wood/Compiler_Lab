//
// Created by admin on 2025/6/10.
//

#include "quad.h"

#include <codecvt>

namespace quad{
    bool QuadRunner::isToken(const string &arg) {
        assert(!arg.empty());
        return arg[0] == '(';
    }

    bool QuadRunner::isEmpty(const string &arg) {
        assert(!arg.empty());
        return arg[0] == '_';
    }

    Token QuadRunner::getToken(const string &arg) {
        assert(isToken(arg));
        Token token{};
        std::stringstream ss;
        ss << arg;
        ss >> token;
        return token;
    }

    int QuadRunner::getInt(const string &arg) {
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
                currentArg = getInt(arg1);
            }
            else if(op == "MOV"){
                Token dest = getToken(arg2);
                if(isToken(arg1)){
                    Token src = getToken(arg1);
                    mem[dest] = mem[src];
                }
                else{
                    mem[dest] = getInt(arg1);
                }
            }
            else if(op == "RET"){
                if(isEmpty(arg1)) return {false, 0};
                return {true, mem[getToken(arg1)]};
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
                const int to = getInt(arg1);
                const auto tmp = currentArg;
                currentArg = to;
                const auto [haveReturnValue, returnValue] = execute(newMemDivice);
                currentArg = tmp;
                if(isEmpty(arg2)){
                    assert(!haveReturnValue);
                }
                else{
                    assert(haveReturnValue);
                    Token dest = getToken(result);
                    mem[dest] = returnValue;
                }
            }
            else if(op == "JZ"){
                const int to = getInt(arg2);
                if(isToken(arg1)){
                    if(const Token src = getToken(arg1); !mem[src]) currentArg = to;
                }
                else{
                    if(const int val = getInt(arg1); !val) currentArg = to;
                }
            }
            else if(op == "ADD" || op == "DEL" || op == "MUL" || op == "DIV" ||
                    op == "ADDF" || op == "DELF" || op == "MULF" || op == "DIVF" ||
                    op == "G" || op == "GE" || op == "L" || op == "LE" || op == "E" || op == "NE" ||
                    op == "GF" || op == "GEF" || op == "LF" || op == "LEF" || op == "EF" || op == "NEF" ||
                    op == "AND" || op == "OR"){
                const Token res = getToken(result);
                if(isToken(arg1)){
                    const Token x = getToken(arg1);
                    if(isToken(arg2)){
                        const Token y = getToken(arg2);
                        mem[res] = genBinaryOperation(op)(mem[x], mem[y]);
                    }
                    else{
                        const int y = getInt(arg2);
                        mem[res] = genBinaryOperation(op)(mem[x], y);
                    }
                }
                else{
                    const int x = getInt(arg1);
                    if(isToken(arg2)){
                        const Token y = getToken(arg2);
                        mem[res] = genBinaryOperation(op)(x, mem[y]);
                    }
                    else{
                        const int y = getInt(arg2);
                        mem[res] = genBinaryOperation(op)(x, y);
                    }
                }
            }
            else if(op == "NOT" ||
                    op == "I2I" || op == "I2F" || op == "I2C" || op == "I2B" ||
                    op == "F2I" || op == "F2F" || op == "F2C" || op == "F2B" ||
                    op == "C2I" || op == "C2F" || op == "C2C" || op == "C2B" ||
                    op == "B2I" || op == "B2F" || op == "B2C" || op == "B2B" ){
                const Token dest = getToken(arg2);
                if(isToken(arg1)){
                    const Token src = getToken(arg1);
                    mem[dest] = genUnaryOperation(op)(mem[src]);
                }
                else{
                    const int src = getInt(arg1);
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
        if(op == "DEL"){
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
        if(op == "DELF"){
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
