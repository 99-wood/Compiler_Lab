//
// Created by admin on 2025/6/4.
//

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <utility>
#include <vector>
#include "lexer.h"
#include "symbol.h"
#include "quad.h"

namespace parser{
    using std::string;
    using std::vector;
    using lexer::Token;
    using quad::Quad;
    using namespace symbol;


    class Parser {
    private:
        vector<Token> str;
        vector<int> ci;
        vector<float> cf;
        vector<string> I;
        vector<Quad> ans;
        vector<Quad> mid;
        int mainPos{};
        vector<string> err;
        vector<string> warn;
        bool hasParsed{};
        vector<Token>::const_iterator ptr;
        vector<SymbolTable> symbolTableStack;
        vector<std::pair<int, Token> > allocQuat;
        vector<std::pair<int, Token> > freeQuat;
        vector<Symbol> global;

        int level{};

        enum class ErrType {
            OTHER
        };

        void addWarnNow(const string &arg = "") {
            std::stringstream ss;
            ss << "[Warning] Warn on ";
            ss << ptr - str.begin() + 1;
            ss << " token.";

            ss << " " + arg;
            string output;
            std::getline(ss, output);
            warn.push_back(output);
        }

        void addImplicitTypeConversionWarn() {
            addWarnNow("Implicit type conversion.");
        }

        void addErrNow(const string &arg = "") {
            std::stringstream ss;
            ss << "[Wrong] Wrong on ";
            ss << ptr - str.begin() + 1;
            ss << " token.";

            ss << " " + arg;
            string output;
            std::getline(ss, output);
            err.push_back(output);
        }

        void addErr(const string &arg) {
            err.push_back("[Wrong] " + arg);
        }

        [[nodiscard]] Token peek() const {
            assert(ptr != str.end());
            return *ptr;
        }

        [[nodiscard]] Token advance() {
            assert(ptr != str.end());
            const Token res = *ptr;
            ++ptr;
            return res;
        }

        [[nodiscard]] bool expect(const Token &tk) const {
            if(ptr == str.end()) return false;
            return *ptr == tk;
        }

        [[nodiscard]] bool expect(const lexer::TokenType &tp) const {
            if(ptr == str.end()) return false;
            return ptr->type == tp;
        }

        [[nodiscard]] bool expect(bool (Parser::*f)(const Token &) const) {
            if(ptr == str.end()) return false;
            return (this->*f)(*ptr);
        }

        [[nodiscard]] bool match(const Token &tk) {
            if(ptr == str.end()) return false;
            if(*ptr != tk) return false;
            else{
                ++ptr;
                return true;
            }
        }

        [[nodiscard]] bool match(const lexer::TokenType &tp) {
            if(ptr == str.end()) return false;
            if(ptr->type != tp) return false;
            else{
                ++ptr;
                return true;
            }
        }

        [[nodiscard]] bool match(bool (Parser::*f)(const Token &) const) {
            if(ptr == str.end()) return false;
            if(!(this->*f)(*ptr)) return false;
            ++ptr;
            return true;
        }

        static const SymbolType *tokenToType(const Token &token) {
            if(token == Token("Int")) return &INT;
            if(token == Token("Float")) return &FLOAT;
            if(token == Token("Char")) return &CHAR;
            if(token == Token("Bool")) return &BOOL;
            if(token == Token("Void")) return &VOID;
            throw std::runtime_error("Wrong on Token to Type");
        }

        // 是否为变量标识符
        [[nodiscard]] bool isVarIdentifier(const Token &token) const {
            for(const auto &symbolTable : std::views::reverse(symbolTableStack)){
                if(auto it = symbolTable.findByToken(token); it != symbolTable.end() && it->kind == SymbolKind::VAR)
                    return true;
            }
            return false;
        }

        // 是否为常量标识符
        [[nodiscard]] bool isValIdentifier(const Token &token) const {
            for(const auto &symbolTable : std::views::reverse(symbolTableStack)){
                if(auto it = symbolTable.findByToken(token); it != symbolTable.end() && it->kind == SymbolKind::VAL)
                    return true;
            }
            return false;
        }

        // 是否为函数标识符
        [[nodiscard]] bool isFuncIdentifier(const Token &token) const {
            if(const auto it = symbolTableStack[0].findByToken(token);
                it != symbolTableStack[0].end() && it->kind == SymbolKind::FUN)
                return true;
            return false;
        }

        // 获取标识符信息
        [[nodiscard]] Symbol getIdentifier(const Token &token) const {
            for(const auto &symbolTable : symbolTableStack | std::views::reverse){
                if(auto it = symbolTable.findByToken(token); it != symbolTable.end()) return *it;
            }
            throw std::runtime_error("cannot find the token");
        }

        [[nodiscard]] bool empty() const {
            return ptr == str.end();
        }

        void next() {
            assert(ptr != str.end());
            ++ptr;
        }

        void prev() {
            assert(ptr != str.begin());
            --ptr;
        }

        int cntTempSymbol, cntSymbol;

        TempSymbol newTempSymbol(const SymbolType *type, int &off) {
            TempSymbol res{
                Token{lexer::TokenType::T, cntTempSymbol++}, type, SymbolKind::VAR, std::pair<int, int>(level, off)
            };
            off += type->size();
            return res;
        }

        static TempSymbol newTempSymbol(const Symbol &symbol) {
            TempSymbol res{symbol};
            res.token.id = symbol.id;
            return res;
        }

        [[nodiscard]] static string offsetToString(const int off, const int size) {
            return "[DS + " + std::to_string(off) + ": " + std::to_string(size) + "]";
        }

        static string addrToString(const string &sreg, const string &breg, const int off, const int size) {
            assert(sreg == "DS" || sreg == "ES");
            assert(breg == "BX");
            if(off == 0){
                return "[" + sreg + " + " + breg + ": " + std::to_string(size) + "]";
            }
            else{
                return "[" + sreg + " + " + breg + " + " + std::to_string(off) + ": " + std::to_string(size) + "]";
            }
        }

        static string addrToString(const string &sreg, const int off, const int size) {
            assert(sreg == "DS" || sreg == "ES");
            if(off == 0){
                return "[" + sreg + ": " + std::to_string(size) + "]";
            }
            else{
                return "[" + sreg + " + " + std::to_string(off) + ": " + std::to_string(size) + "]";
            }
        }

        [[nodiscard]] int push(const string &reg, int &off) {
            assert(reg == "BX" || reg == "DS" || reg == "ES");
            TempSymbol res = newTempSymbol(&INT, off);
            ans.emplace_back(
                "MOV",
                reg,
                offsetToString(std::get<std::pair<int, int> >(res.ptr).second, 4));
            return std::get<std::pair<int, int> >(res.ptr).second;
        }

        void pop(const string &reg, const int off) {
            assert(reg == "BX" || reg == "DS" || reg == "ES");
            ans.emplace_back(
                "MOV",
                offsetToString(off, 4),
                reg);
        }

        void mov(const string &dest, const string &src, const int size) {
            assert(dest == "BX" || dest == "DS" || dest == "ES");
            assert(src == "BX" || src == "DS" || dest == "ES");
            if(dest == src) return;
            assert(size == 4);
            ans.emplace_back("MOV", src, dest);
        }

        void mov(const string &reg, const int &src, const int size) {
            assert(reg == "BX" || reg == "DS" || reg == "ES");
            ans.emplace_back(
                "MOV",
                offsetToString(src, size),
                reg);
        }

        void mov(const int &dest, const string &reg, const int size) {
            assert(reg == "BX" || reg == "DS");
            ans.emplace_back(
                "MOV",
                reg,
                offsetToString(dest, size));
        }

        //        void mov(const std::pair<int, int> &dest, const string &reg, const int size, int &off) {
        //            assert(reg == "BX" || reg == "DS");
        //            auto [dlv, doff] = dest;
        //            if(dlv == level) return mov(doff, reg, size);
        //            else{
        //                if(reg == "DS"){
        //                    const int tmp = off;
        //                    off += 4;
        //                    push("BX", off);
        //                    mov("BX", "DS", 4);
        //                    mov("DS", 12 + 4 * dlv, 4);
        //                    mov(doff, "BX", size);
        //                    mov("DS", "BX", 4);
        //                    pop("BX", tmp);
        //                }
        //                else{
        //                    const int tmp = off;
        //                    off += 4;
        //                    push("ES", off);
        //                    mov("ES", "DS", 4);
        //                    mov("DS", 12 + 4 * dlv, 4);
        //                    mov(doff, reg, size);
        //                    mov("DS", "ES", 4);
        //                    pop("ES", tmp);
        //                }
        //            }
        //        }

        // BX <- [地址]
        void mov(const string &reg, const std::pair<int, int> &src, const int size, int &off) {
            assert(reg == "BX");
            auto [slv, soff] = src;
            if(slv == level) return mov(reg, soff, size);
            else{
                const int tmp = push("ES", off);
                mov("ES", "DS", 4);
                mov("DS", 12 + 4 * slv, 4);
                mov("BX", soff, size);
                mov("DS", "ES", 4);
                pop("ES", tmp);
            }
        }

        // [偏移量] <- [偏移量]
        void mov(const int dest, const int src, const int size) {
            ans.emplace_back("MOV", offsetToString(src, size), offsetToString(dest, size));
        }

        // [偏移量] <- [地址]
        void mov(const int dest, const std::pair<int, int> &src, const int size, int &off) {
            if(const auto [slv, soff] = src; slv == level) return mov(dest, soff, size);
            else{
                mov("BX", src, size, off);
                mov(dest, "BX", size);
            }
        }

        // [偏移量] <- 临时变量
        //        void mov(const int dest, const TempSymbol &src, int &off) {
        //            if(src.type == &VOID) throw std::runtime_error("Cannot move Void.");
        //            if(src.kind == SymbolKind::CONST){
        //                ans.emplace_back("MOV",
        //                                 std::to_string(src.getVal()),
        //                                 offsetToString(dest, src.type->size()));
        //            }
        //            else{
        //                assert(src.kind == SymbolKind::VAL || src.kind == SymbolKind::VAR);
        //                mov(dest, std::pair<int, int>(std::get<std::pair<int, int> >(src.ptr)), src.type->size(), off);
        //            }
        //        }

        // [ES:BX] <- [DS:OFF]
        void mov(const string &ES, const string &BX, const string &DS, const int off, const int size) {
            assert(ES == "ES");
            assert(BX == "BX");
            assert(DS == "DS");
            ans.emplace_back("MOV",
                             "[DS + " + std::to_string(off) + ": " + std::to_string(size) + "]",
                             "[ES + BX: " + std::to_string(size) + "]");
        }

        // 跳转到 BX
        void jmp(const string &reg) {
            assert(reg == "BX");
            ans.emplace_back("JMP", "BX");
            return;
        }

        // 临时变量 <- 临时变量，带类型类型检查，可警告隐式类型转换，影响 ES
        void mov(const TempSymbol &dest, TempSymbol src, int &off) {
            assert(dest.type != &VOID);
            assert(src.type != &VOID);
            assert(dest.kind == SymbolKind::VAR || dest.kind == SymbolKind::VAL);
            if(src.type != dest.type){
                addImplicitTypeConversionWarn();
                src = typeConversion(src, dest.type, off);
            }
            const auto [lv, of] = std::get<std::pair<int, int> >(dest.ptr);
            if(src.kind == SymbolKind::CONST){
                // const int tmp = push("ES", off);
                if(lv < level) mov("ES", 12 + lv * 4, 4);
                else mov("ES", "DS", 4);
                ans.emplace_back("MOV",
                                 std::to_string(src.getVal()),
                                 addrToString("ES", of, dest.type->size()));
                mid.emplace_back("MOV",
                                 std::to_string(src.getVal()),
                                 dest.token.toString());
                // pop("ES", tmp);
                return;
            }
            else{
                src = toLocal(src, off);
                // TODO: 优化传值
                // const int tmp = push("ES", off);
                if(lv < level) mov("ES", 12 + lv * 4, 4);
                else mov("ES", "DS", 4);
                ans.emplace_back("MOV",
                                 offsetToString(std::get<std::pair<int, int> >(src.ptr).second, src.type->size()),
                                 addrToString("ES", of, src.type->size()));
                mid.emplace_back("MOV",
                                 src.token.toString(),
                                 dest.token.toString());
                // pop("ES", tmp);
                return;
            }
        }

        // 类型转换
        TempSymbol typeConversion(const TempSymbol &symbol, const SymbolType *type, int &off) {
            assert(
                symbol.kind == SymbolKind::CONST || symbol.kind == SymbolKind::VAL ||
                symbol.kind == SymbolKind::VAR);
            if(symbol.type == type) return symbol;
            if(symbol.type == &VOID){
                throw std::runtime_error("Cannot trans Void.");
            }
            if(type == &VOID){
                throw std::runtime_error("Cannot trans to Void.");
            }
            if(symbol.kind == SymbolKind::CONST){
                TempSymbol res;
                res.type = type;
                res.kind = SymbolKind::CONST;
                if(type == &INT){
                    if(symbol.type == &INT) res.ptr = static_cast<int>(std::get<int>(symbol.ptr));
                    else if(symbol.type == &FLOAT) res.ptr = static_cast<int>(std::get<float>(symbol.ptr));
                    else if(symbol.type == &CHAR) res.ptr = static_cast<int>(std::get<char>(symbol.ptr));
                    else if(symbol.type == &BOOL) res.ptr = static_cast<int>(std::get<bool>(symbol.ptr));
                    else throw std::runtime_error("Wrong type.");
                }
                else if(type == &FLOAT){
                    if(symbol.type == &INT) res.ptr = static_cast<float>(std::get<int>(symbol.ptr));
                    else if(symbol.type == &FLOAT) res.ptr = static_cast<float>(std::get<float>(symbol.ptr));
                    else if(symbol.type == &CHAR) res.ptr = static_cast<float>(std::get<char>(symbol.ptr));
                    else if(symbol.type == &BOOL) res.ptr = static_cast<float>(std::get<bool>(symbol.ptr));
                    else throw std::runtime_error("Wrong type.");
                }
                else if(type == &CHAR){
                    if(symbol.type == &INT) res.ptr = static_cast<char>(std::get<int>(symbol.ptr));
                    else if(symbol.type == &FLOAT) res.ptr = static_cast<char>(std::get<float>(symbol.ptr));
                    else if(symbol.type == &CHAR) res.ptr = static_cast<char>(std::get<char>(symbol.ptr));
                    else if(symbol.type == &BOOL) res.ptr = static_cast<char>(std::get<bool>(symbol.ptr));
                    else throw std::runtime_error("Wrong type.");
                }
                else if(type == &BOOL){
                    if(symbol.type == &INT) res.ptr = static_cast<bool>(std::get<int>(symbol.ptr));
                    else if(symbol.type == &FLOAT) res.ptr = static_cast<bool>(std::get<float>(symbol.ptr));
                    else if(symbol.type == &CHAR) res.ptr = static_cast<bool>(std::get<char>(symbol.ptr));
                    else if(symbol.type == &BOOL) res.ptr = static_cast<bool>(std::get<bool>(symbol.ptr));
                    else throw std::runtime_error("Wrong type.");
                }
                else throw std::runtime_error("Wrong type.");
                return res;
            }
            else{
                TempSymbol res = newTempSymbol(type, off);
                //                res.type = type;
                //                res.kind = SymbolKind::VAR;
                //                res.ptr = std::pair<int, int>(level, off);
                auto [lv, of] = std::get<std::pair<int, int> >(symbol.ptr);
                // TODO: 优化传值
                // const int tmpES = push("ES", off);
                if(lv < level) mov("ES", 12 + lv * 4, 4);
                else mov("ES", "DS", 4);
                string s;
                s.push_back(symbol.type->toString()[0]);
                s.push_back('2');
                s.push_back(type->toString()[0]);
                //                int tmp = off;
                //                off += type->size();
                ans.emplace_back(s,
                                 addrToString("ES", of, res.type->size()),
                                 offsetToString(std::get<std::pair<int, int> >(res.ptr).second, type->size()));
                mid.emplace_back(s,
                                 symbol.token.toString(),
                                 res.token.toString());
                // pop("ES", tmpES);
                return res;
            }
        }

        // 移动到当前段
        TempSymbol toLocal(const TempSymbol &symbol, int &off) {
            assert(
                symbol.kind == SymbolKind::CONST || symbol.kind == SymbolKind::VAL || symbol.kind == SymbolKind::VAR);
            if(symbol.type == &VOID){
                throw std::runtime_error("Cannot trans Void to local.");
            }
            if(symbol.kind == SymbolKind::CONST){
                TempSymbol res = newTempSymbol(symbol.type, off);
                //                const int tmp = off;
                //                off += symbol.type->size();
                ans.emplace_back("MOV",
                                 std::to_string(symbol.getVal()),
                                 offsetToString(std::get<std::pair<int, int> >(res.ptr).second, symbol.type->size()));
                mid.emplace_back("MOV",
                                 std::to_string(symbol.getVal()),
                                 res.token.toString());
                return res;
            }
            else{
                if(std::get<std::pair<int, int> >(symbol.ptr).first == level) return symbol; // 如果就在当前段直接返回
                TempSymbol res = newTempSymbol(symbol.type, off);
                //                const int tmp = off;
                //                off += symbol.type->size();
                auto [lv, of] = std::get<std::pair<int, int> >(symbol.ptr);
                // const int tmpES = push("ES", off);
                mov("ES", 12 + lv * 4, 4);
                ans.emplace_back("MOV",
                                 addrToString("ES", of, symbol.type->size()),
                                 offsetToString(std::get<std::pair<int, int> >(res.ptr).second, symbol.type->size()));
                mid.emplace_back("MOV",
                                 symbol.token.toString(),
                                 res.token.toString());
                // pop("ES", tmpES);
                return res;
            }
        }

        // 处理逻辑或运算
        TempSymbol handleLogicOr(TempSymbol x, TempSymbol y, int &off) {
            assert(x.type != &VOID);
            assert(y.type != &VOID);
            if(x.type != &BOOL) addImplicitTypeConversionWarn();
            if(y.type != &BOOL) addImplicitTypeConversionWarn();
            x = typeConversion(x, &BOOL, off);
            y = typeConversion(y, &BOOL, off);
            if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                return {
                    Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                    std::get<bool>(x.ptr) || std::get<bool>(y.ptr)
                };
            }
            else{
                x = toLocal(x, off);
                y = toLocal(y, off);
                TempSymbol res = newTempSymbol(&BOOL, off);
                //                const int tmp = off;
                //                off += BOOL.size();
                ans.emplace_back("OR",
                                 offsetToString(std::get<std::pair<int, int> >(x.ptr).second, BOOL.size()),
                                 offsetToString(std::get<std::pair<int, int> >(y.ptr).second, BOOL.size()),
                                 offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                mid.emplace_back("OR",
                                 x.token.toString(),
                                 y.token.toString(),
                                 res.token.toString());
                return res;
            }
        }

        // 处理逻辑与运算
        TempSymbol handleLogicAnd(TempSymbol x, TempSymbol y, int &off) {
            assert(x.type != &VOID);
            assert(y.type != &VOID);
            if(x.type != &BOOL) addImplicitTypeConversionWarn();
            if(y.type != &BOOL) addImplicitTypeConversionWarn();
            x = typeConversion(x, &BOOL, off);
            y = typeConversion(y, &BOOL, off);
            if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                return {
                    Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                    std::get<bool>(x.ptr) && std::get<bool>(y.ptr)
                };
            }
            else{
                x = toLocal(x, off);
                y = toLocal(y, off);
                TempSymbol res = newTempSymbol(&BOOL, off);
                //                const int tmp = off;
                //                off += BOOL.size();
                ans.emplace_back("AND",
                                 offsetToString(std::get<std::pair<int, int> >(x.ptr).second, BOOL.size()),
                                 offsetToString(std::get<std::pair<int, int> >(y.ptr).second, BOOL.size()),
                                 offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                mid.emplace_back("AND",
                                 x.token.toString(),
                                 y.token.toString(),
                                 res.token.toString());
                return res;
            }
        }

        // 处理逻辑非运算
        TempSymbol handleLogicNot(const TempSymbol &x, int &off) {
            // TODO
            assert(0);
        }

        // 处理关系运算
        TempSymbol handleRelation(TempSymbol x, TempSymbol y, const Token &token, int &off) {
            assert(x.type != &VOID);
            assert(y.type != &VOID);
            if(x.type != y.type) addImplicitTypeConversionWarn();
            assert(token == Token(">") ||
                token == Token(">=") ||
                token == Token("<") ||
                token == Token("<=") ||
                token == Token("==") ||
                token == Token("!="));
            if(x.type == &FLOAT || y.type == &FLOAT){
                x = typeConversion(x, &FLOAT, off);
                y = typeConversion(y, &FLOAT, off);
                if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                    if(token == Token(">"))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<float>(x.ptr) > std::get<float>(y.ptr)
                        };
                    else if(token == Token(">="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<float>(x.ptr) >= std::get<float>(y.ptr)
                        };
                    else if(token == Token("<"))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<float>(x.ptr) < std::get<float>(y.ptr)
                        };
                    else if(token == Token("<="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<float>(x.ptr) <= std::get<float>(y.ptr)
                        };
                    else if(token == Token("=="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<float>(x.ptr) == std::get<float>(y.ptr)
                        };
                    else if(token == Token("!="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<float>(x.ptr) != std::get<float>(y.ptr)
                        };
                    else
                        assert(0);
                }
                else{
                    x = toLocal(x, off);
                    y = toLocal(y, off);
                    TempSymbol res = newTempSymbol(&BOOL, off);
                    //                    const int tmp = off;
                    //                    off += BOOL.size();
                    if(token == Token(">")){
                        ans.emplace_back("GF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("GF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token(">=")){
                        ans.emplace_back("GEF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("GEF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("<")){
                        ans.emplace_back("LF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("LF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("<=")){
                        ans.emplace_back("LEF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("LEF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("==")){
                        ans.emplace_back("EF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("EF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("!=")){
                        ans.emplace_back("NEF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("NEF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else
                        assert(0);
                    return res;
                }
            }
            else{
                if(x.type != &INT || y.type != &INT) addImplicitTypeConversionWarn();
                x = typeConversion(x, &INT, off);
                y = typeConversion(y, &INT, off);
                if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                    if(token == Token(">"))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<int>(x.ptr) > std::get<int>(y.ptr)
                        };
                    else if(token == Token(">="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<int>(x.ptr) >= std::get<int>(y.ptr)
                        };
                    else if(token == Token("<"))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<int>(x.ptr) < std::get<int>(y.ptr)
                        };
                    else if(token == Token("<="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<int>(x.ptr) <= std::get<int>(y.ptr)
                        };
                    else if(token == Token("=="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<int>(x.ptr) == std::get<int>(y.ptr)
                        };
                    else if(token == Token("!="))
                        return {
                            Token{lexer::TokenType::T, -1}, &BOOL, SymbolKind::CONST,
                            std::get<int>(x.ptr) != std::get<int>(y.ptr)
                        };
                    else
                        assert(0);
                }
                else{
                    x = toLocal(x, off);
                    y = toLocal(y, off);
                    TempSymbol res = newTempSymbol(&BOOL, off);
                    //                    const int tmp = off;
                    //                    off += BOOL.size();
                    if(token == Token(">")){
                        ans.emplace_back("G",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("G",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token(">=")){
                        ans.emplace_back("GE",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("GE",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("<")){
                        ans.emplace_back("L",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("L",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("<=")){
                        ans.emplace_back("LE",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("LE",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("==")){
                        ans.emplace_back("E",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("E",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else if(token == Token("!=")){
                        ans.emplace_back("NE",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, BOOL.size()));
                        mid.emplace_back("NE",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else
                        assert(0);
                    return res;
                }
            }
        }

        // 处理加减运算
        TempSymbol handleAddOrSub(TempSymbol x, TempSymbol y, const Token &token, int &off) {
            assert(x.type != &VOID);
            assert(y.type != &VOID);
            if(x.type != y.type) addImplicitTypeConversionWarn();
            assert(token == Token("+") || token == Token("-"));
            bool isAdd = (token == Token("+"));
            if(x.type == &FLOAT || y.type == &FLOAT){
                x = typeConversion(x, &FLOAT, off);
                y = typeConversion(y, &FLOAT, off);
                if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                    if(isAdd)
                        return {
                            Token{lexer::TokenType::T, -1}, &FLOAT, SymbolKind::CONST,
                            std::get<float>(x.ptr) + std::get<float>(y.ptr)
                        };
                    else
                        return {
                            Token{lexer::TokenType::T, -1}, &FLOAT, SymbolKind::CONST,
                            std::get<float>(x.ptr) - std::get<float>(y.ptr)
                        };
                }
                else{
                    x = toLocal(x, off);
                    y = toLocal(y, off);
                    TempSymbol res = newTempSymbol(&FLOAT, off);
                    //                    const int tmp = off;
                    //                    off += FLOAT.size();
                    if(isAdd){
                        ans.emplace_back("ADDF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, FLOAT.size()));
                        mid.emplace_back("ADDF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else{
                        ans.emplace_back("SUBF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, FLOAT.size()));
                        mid.emplace_back("SUBF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    return res;
                }
            }
            else{
                if(x.type != &INT || y.type != &INT) addImplicitTypeConversionWarn();
                x = typeConversion(x, &INT, off);
                y = typeConversion(y, &INT, off);
                if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                    if(isAdd)
                        return {
                            Token{lexer::TokenType::T, -1}, &INT, SymbolKind::CONST,
                            std::get<int>(x.ptr) + std::get<int>(y.ptr)
                        };
                    else
                        return {
                            Token{lexer::TokenType::T, -1}, &INT, SymbolKind::CONST,
                            std::get<int>(x.ptr) - std::get<int>(y.ptr)
                        };
                }
                else{
                    x = toLocal(x, off);
                    y = toLocal(y, off);
                    TempSymbol res = newTempSymbol(&INT, off);
                    //                    const int tmp = off;
                    //                    off += INT.size();
                    if(isAdd){
                        ans.emplace_back("ADD",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, INT.size()));
                        mid.emplace_back("ADD",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else{
                        ans.emplace_back("SUB",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, INT.size()));
                        mid.emplace_back("SUB",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    return res;
                }
            }
        }

        // 处理乘除运算
        TempSymbol handleMulOrDiv(TempSymbol x, TempSymbol y, const Token &token, int &off, bool &isSuccess) {
            isSuccess = true;
            assert(x.type != &VOID);
            assert(y.type != &VOID);
            if(x.type != y.type) addImplicitTypeConversionWarn();
            assert(token == Token("*") || token == Token("/"));
            bool isMul = (token == Token("*"));
            if(x.type == &FLOAT || y.type == &FLOAT){
                x = typeConversion(x, &FLOAT, off);
                y = typeConversion(y, &FLOAT, off);
                if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                    if(isMul)
                        return {
                            Token{lexer::TokenType::T, -1}, &FLOAT, SymbolKind::CONST,
                            std::get<float>(x.ptr) * std::get<float>(y.ptr)
                        };
                    else
                        return {
                            Token{lexer::TokenType::T, -1}, &FLOAT, SymbolKind::CONST,
                            std::get<float>(x.ptr) / std::get<float>(y.ptr)
                        };
                }
                else{
                    x = toLocal(x, off);
                    y = toLocal(y, off);
                    TempSymbol res = newTempSymbol(&FLOAT, off);
                    //                    const int tmp = off;
                    //                    off += FLOAT.size();
                    if(isMul){
                        ans.emplace_back("MULF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, FLOAT.size()));
                        mid.emplace_back("MULF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else{
                        ans.emplace_back("DIVF",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, FLOAT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, FLOAT.size()));
                        mid.emplace_back("DIVF",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    return res;
                }
            }
            else{
                if(x.type != &INT || y.type != &INT) addImplicitTypeConversionWarn();
                x = typeConversion(x, &INT, off);
                y = typeConversion(y, &INT, off);
                if(x.kind == SymbolKind::CONST && y.kind == SymbolKind::CONST){
                    if(isMul)
                        return {
                            Token{lexer::TokenType::T, -1}, &INT, SymbolKind::CONST,
                            std::get<int>(x.ptr) * std::get<int>(y.ptr)
                        };
                    else{
                        if(std::get<int>(y.ptr) == 0){
                            isSuccess = false;
                            return {};
                        }
                        return {
                            Token{lexer::TokenType::T, -1}, &INT, SymbolKind::CONST,
                            std::get<int>(x.ptr) / std::get<int>(y.ptr)
                        };
                    }
                }
                else{
                    x = toLocal(x, off);
                    y = toLocal(y, off);
                    TempSymbol res = newTempSymbol(&INT, off);
                    //                    const int tmp = off;
                    //                    off += INT.size();
                    if(isMul){
                        ans.emplace_back("MUL",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, INT.size()));
                        mid.emplace_back("MUL",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    else{
                        ans.emplace_back("DIV",
                                         offsetToString(std::get<std::pair<int, int> >(x.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(y.ptr).second, INT.size()),
                                         offsetToString(std::get<std::pair<int, int> >(res.ptr).second, INT.size()));
                        mid.emplace_back("DIV",
                                         x.token.toString(),
                                         y.token.toString(),
                                         res.token.toString());
                    }
                    return res;
                }
            }
        }

        // 处理函数调用，影响 ES
        TempSymbol handleCallFunction(const Symbol &funSymbol, const vector<TempSymbol> &args, int &off,
                                      bool &isSuccess) {
            isSuccess = true;
            assert(funSymbol.kind == SymbolKind::FUN);
            const FunInfo *funInfo = std::get<FunInfo *>(funSymbol.ptr);
            const ParamInfo &paramInfo = *funInfo->paramInfoPtr;
            if(args.size() != paramInfo.size()){
                addErrNow("Function parameter mismatch");
                isSuccess = false;
                return {};
            }
            const int STACK_ALLOC = static_cast<int>(ans.size());
            ans.emplace_back("STACK_ALLOC", "?");
            allocQuat.emplace_back(STACK_ALLOC, funSymbol.token); // 存储分配空间语句以备反填
            const TempSymbol res = funSymbol.type == &VOID
                                       ? TempSymbol{Token{lexer::TokenType::T, -1}, &VOID, SymbolKind::CONST, 0}
                                       : newTempSymbol(funSymbol.type, off);
            Quad CALL{
                "CALL",
                std::to_string(std::get<FunInfo *>(funSymbol.ptr)->entryM),
                "",
                funSymbol.type == &VOID ? "_" : res.token.toString()
            };
            //            const int retOff = funSymbol.type == &VOID ? 0 : off;
            //            off += funSymbol.type->size();
            ans.emplace_back("MOV", "DS", addrToString("ES", 0, 4)); // 返回地址基址压栈
            if(funSymbol.type == &VOID) ans.emplace_back("MOV", std::to_string(-1), addrToString("ES", 4, 4));
                // 返回地址偏移量压栈
            else
                ans.emplace_back("MOV", std::to_string(std::get<std::pair<int, int> >(res.ptr).second),
                                 addrToString("ES", 4, 4)); // 返回地址偏移量压栈
            const int JMP_ARG = static_cast<int>(ans.size());
            ans.emplace_back("MOV", "?", addrToString("ES", 8, 4)); // 跳转语句压栈
            ans.emplace_back("MOV", offsetToString(12, 4), addrToString("ES", 12, 4)); // 全局 display 压栈
            int tmpES = push("ES", off);
            CALL.arg2 += std::to_string(static_cast<int>(args.size())) + " ";
            for(int i = 0, newOff = 16; i < static_cast<int>(args.size()); newOff += args[i].type->size(), ++i){
                if(args[i].type != paramInfo[i].type){
                    addErrNow("Function parameter mismatch");
                    isSuccess = false;
                    return {};
                }
                TempSymbol arg = toLocal(args[i], off);
                CALL.arg2 += arg.token.toString() + " : " + TempSymbol{paramInfo[i]}.token.toString();
                pop("ES", tmpES);
                ans.emplace_back(
                    "MOV",
                    offsetToString(std::get<std::pair<int, int> >(arg.ptr).second, args[i].type->size()),
                    addrToString("ES", newOff, args[i].type->size()));
            }
            mid.push_back(CALL);
            pop("ES", tmpES);
            ans.emplace_back("MOV", "ES", "DS"); // 段更换
            ans.emplace_back("JMP", std::to_string(funInfo->entry)); // 跳转执行
            ans[JMP_ARG] = Quad("MOV", std::to_string(ans.size()), addrToString("ES", 8, 4)); //反填压栈跳转;
            const int STACK_FREE = static_cast<int>(ans.size());
            ans.emplace_back("STACK_FREE", "?");
            freeQuat.emplace_back(STACK_FREE, funSymbol.token); // 存储释放空间语句以备反填
            return res;
        }


        // 处理 <初始化列表后缀'>
        bool parseInitListSuffixTail(int &off, const SymbolKind &kind) {
            if(expect(Token(";"))){
                next();
                return true;
            }
            if(!expect(Token(","))){
                addErrNow();
                return false;
            }
            next();
            Token name{};
            TempSymbol res{};
            // SymbolTable &symbolTable = symbolTableStack.back();
            if(expect(lexer::TokenType::I)){
                name = peek();
                if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                next();
            }
            else{
                addErrNow();
                return false;
            }
            if(!match(Token("="))) return false;
            if(!parseExpression(off, res)) return false;
            const Symbol var{name, res.type, kind, std::pair<int, int>(level, off), ++cntSymbol};
            symbolTableStack[level].push_back(var);
            //            const int tmp = off;
            off += static_cast<int>(res.type->size());
            mov(TempSymbol(var), res, off);
            return parseInitListSuffixTail(off, kind);
        }


        // 处理 <初始化列表后缀>
        bool parseInitListSuffix(int &off, const Token &lastToken, const SymbolKind &kind) {
            assert(match(Token("=")));
            TempSymbol res{};
            if(!parseExpression(off, res)) return false;
            const Symbol var = {lastToken, res.type, kind, std::pair<int, int>(level, off), ++cntSymbol};
            symbolTableStack[level].push_back(var);
            //            const int tmp = off;
            off += static_cast<int>(res.type->size());
            mov(TempSymbol(var), res, off);
            return parseInitListSuffixTail(off, kind);
        }

        // 处理 <标识符列表后缀>
        bool parseIdentListSuffix(int &off, vector<Token> &identList) {
            if(expect(Token(":"))){
                next();
                if(!expect(lexer::TokenType::K)){
                    addErrNow("Undefined type.");
                    return false;
                }
                const Token token = peek();
                next();
                if(!isTypeToken(token)){
                    addErrNow("Not a type.");
                    return false;
                }
                const SymbolType *type = tokenToType(token);
                for(const Token &name : identList){
                    if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                        addErrNow("Multi define.");
                        return false;
                    }
                    symbolTableStack[level].emplace_back(name, type, SymbolKind::VAR, std::pair<int, int>(level, off), ++cntSymbol);
                    off += static_cast<int>(type->size());
                }
                if(!match(Token(";"))){
                    addErrNow("Expect a ';'.");
                    return false;
                }
                return true;
            }
            if(expect(Token(","))){
                next();
                if(!expect(lexer::TokenType::I)){
                    addErrNow();
                    return false;
                }
                const Token name = peek();
                if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                identList.push_back(name);
                next();
                return parseIdentListSuffix(off, identList);
            }
            addErrNow();
            return false;
        }

        // 处理 <变量定义后缀>
        bool parseVarDefSuffix(int &off, const Token &lastToken, const SymbolKind &kind) {
            if(expect(Token("="))) return parseInitListSuffix(off, lastToken, kind);
            else if(expect(Token(":")) || expect(Token(","))){
                if(kind == SymbolKind::VAL){
                    addErrNow("Val must be initialized.");
                    return false;
                }
                vector<Token> identList{lastToken};
                return parseIdentListSuffix(off, identList);
            }
            else{
                addErrNow();
                return false;
            }
        }

        // 处理 <变量定义语句>
        bool parseVarDefStmt(int &off) {
            assert(match(Token("var")));
            Token name{};
            // SymbolTable &symbolTable = symbolTableStack.back();
            if(expect(lexer::TokenType::I)){
                name = peek();
                if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                next();
            }
            else{
                addErrNow();
                return false;
            }
            return parseVarDefSuffix(off, name, SymbolKind::VAR);
        }

        // 处理 <常量定义语句>
        bool parseValDefStmt(int &off) {
            assert(match(Token("val")));
            Token name{};
            // SymbolTable &symbolTable = symbolTableStack.back();
            if(expect(lexer::TokenType::I)){
                name = peek();
                if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                next();
            }
            else{
                addErrNow();
                return false;
            }
            return parseVarDefSuffix(off, name, SymbolKind::VAL);
        }

        void genCodeGlockCall(int &STACK_ALLOC) {
            STACK_ALLOC = static_cast<int>(ans.size());
            ans.emplace_back("STACK_ALLOC", "?");
            ans.emplace_back("MOV", offsetToString(0, 4), addrToString("ES", 0, 4)); // 返回地址基址压栈
            ans.emplace_back("MOV", offsetToString(4, 4), addrToString("ES", 4, 4)); // 返回地址偏移量压栈
            ans.emplace_back("MOV", offsetToString(8, 4), addrToString("ES", 8, 4)); // 跳转语句压栈
            for(int lv = 0; lv < level; ++lv){
                ans.emplace_back("MOV", offsetToString(12 + lv * 4, 4), addrToString("ES", 12 + lv * 4, 4));
                // display 压栈
            }
            ans.emplace_back("MOV", "DS", addrToString("ES", 12 + level * 4, 4)); // 当前 display 压栈
            ans.emplace_back("MOV", "ES", "DS"); // 段更换
            return;
        }

        // 处理 <if 语句>
        bool parseIfStmt(int &off, const Symbol &funSymbol) {
            assert(match(Token("if")));
            if(!match(Token("("))) return false;
            TempSymbol res;
            parseExpression(off, res);
            if(!match(Token(")"))) return false;
            if(res.type == &VOID){
                addErrNow("Unable to determine Void");
                return false;
            }
            if(res.type != &BOOL){
                addImplicitTypeConversionWarn();
                res = typeConversion(res, &BOOL, off);
            }
            const int JZ = static_cast<int>(ans.size());
            const int JZM = static_cast<int>(mid.size());
            if(res.kind == SymbolKind::CONST){
                addWarnNow("If statement contains constants.");
                ans.emplace_back("JZ", std::to_string(res.getVal()), "?");
                mid.emplace_back("JZ", std::to_string(res.getVal()), "?");
            }
            else{
                res = toLocal(res, off);
                ans.emplace_back("JZ", offsetToString(std::get<std::pair<int, int> >(res.ptr).second, 1), "?");
                mid.emplace_back("JZ", res.token.toString(), "?");
            }
            int ifBlockOffset = (level + 1) * 4 + 12;
            int IF_STACK_ALLOC; // 反填语句记录
            genCodeGlockCall(IF_STACK_ALLOC); // 生成块调用代码
            ++level;
            symbolTableStack.emplace_back(global); //构造符号表
            if(!parseCodeBlock(ifBlockOffset, funSymbol)) return false;
            symbolTableStack.pop_back();
            ans.emplace_back("MOV", offsetToString(12 + (level - 1) * 4, 4), "DS"); // 段更换回来
            --level;
            ans.emplace_back("STACK_FREE", std::to_string(ifBlockOffset));
            assert(ans[IF_STACK_ALLOC].op == "STACK_ALLOC");
            ans[IF_STACK_ALLOC] = Quad{"STACK_ALLOC", std::to_string(ifBlockOffset)};
            if(match(Token("else"))){
                const int JMP = static_cast<int>(ans.size());
                const int JMPM = static_cast<int>(mid.size());
                ans.emplace_back("JMP", "?");
                mid.emplace_back("JMP", "?");
                ans[JZ].arg2 = std::to_string(ans.size());
                mid[JZM].arg2 = std::to_string(mid.size());
                int elseBlockOffset = (level + 1) * 4 + 12;
                int ELSE_STACK_ALLOC;
                genCodeGlockCall(ELSE_STACK_ALLOC);
                ++level;
                symbolTableStack.emplace_back(global);
                if(!parseCodeBlock(elseBlockOffset, funSymbol)) return false;
                symbolTableStack.pop_back();
                ans.emplace_back("MOV", offsetToString(12 + (level - 1) * 4, 4), "DS"); // 段更换回来
                --level;
                ans.emplace_back("STACK_FREE", std::to_string(elseBlockOffset));
                assert(ans[ELSE_STACK_ALLOC].op == "STACK_ALLOC");
                ans[ELSE_STACK_ALLOC] = Quad{"STACK_ALLOC", std::to_string(elseBlockOffset)};
                ans[JMP].arg1 = std::to_string(ans.size());
                mid[JMPM].arg1 = std::to_string(mid.size());
            }
            else{
                ans[JZ].arg2 = std::to_string(ans.size());
                mid[JZM].arg2 = std::to_string(mid.size());
            }
            return true;
        }

        // 处理 <while 语句>
        bool parseWhileStmt(int &off, const Symbol &funSymbol) {
            assert(match(Token("while")));
            if(!match(Token("("))) return false;
            const int WH = ans.size();
            const int WHM = mid.size();
            TempSymbol res;
            parseExpression(off, res);
            if(!match(Token(")"))) return false;
            if(res.type == &VOID){
                addErrNow("Unable to determine Void");
                return false;
            }
            if(res.type != &BOOL){
                addImplicitTypeConversionWarn();
                res = typeConversion(res, &BOOL, off);
            }
            const int JZ = static_cast<int>(ans.size());
            const int JZM = static_cast<int>(mid.size());
            if(res.kind == SymbolKind::CONST){
                addWarnNow("If statement contains constants.");
                ans.emplace_back("JZ", std::to_string(res.getVal()), "?");
                mid.emplace_back("JZ", std::to_string(res.getVal()), "?");
            }
            else{
                res = toLocal(res, off);
                ans.emplace_back("JZ", offsetToString(std::get<std::pair<int, int> >(res.ptr).second, 1), "?");
                mid.emplace_back("JZ", res.token.toString(), "?");
            }
            int BlockOffset = (level + 1) * 4 + 12;
            int STACK_ALLOC; // 反填语句记录
            genCodeGlockCall(STACK_ALLOC); // 生成块调用代码
            ++level;
            symbolTableStack.emplace_back(global); //构造符号表
            if(!parseCodeBlock(BlockOffset, funSymbol)) return false;
            symbolTableStack.pop_back();
            ans.emplace_back("MOV", offsetToString(12 + (level - 1) * 4, 4), "DS"); // 段更换回来
            --level;
            ans.emplace_back("STACK_FREE", std::to_string(BlockOffset));
            assert(ans[STACK_ALLOC].op == "STACK_ALLOC");
            ans[STACK_ALLOC] = Quad{"STACK_ALLOC", std::to_string(BlockOffset)};
            ans.emplace_back("JMP", std::to_string(WH));
            mid.emplace_back("JMP", std::to_string(WHM));
            ans[JZ].arg2 = std::to_string(ans.size());
            mid[JZM].arg2 = std::to_string(mid.size());
            return true;
        }

        // 处理 <return 语句>
        bool parseReturnStmt(int &off, const Symbol &funSymbol) {
            assert(match(Token("return")));
            if(match(Token(";"))){
                if(funSymbol.type != &VOID){
                    addErrNow("The function needs to return a value.");
                    return false;
                }
                mov("BX", 8, 4);
                mov("DS", 0, 4);
                // TODO: 释放空间
                jmp("BX");
                mid.emplace_back("RET", "_");
                return true;
            }
            else{
                TempSymbol res;
                if(!parseExpression(off, res)) return false;
                if(!match(Token(";"))) return false;
                if(funSymbol.type == &VOID){
                    addErrNow("The Void function don't needs to return a value.");
                    return false;
                }
                // TODO: 优化传值
                res = toLocal(res, off);
                mid.emplace_back("RET", res.token.toString());
                mov("ES", 0, 4);
                mov("BX", 4, 4);
                mov("ES", "BX", "DS", std::get<std::pair<int, int> >(res.ptr).second, res.type->size());
                mov("BX", 8, 4);
                mov("DS", 0, 4);
                // TODO: 释放空间
                jmp("BX");
            }
            return true;
        }


        // 处理 <类型转换表达式>
        bool parseTypeCastExpr(int &off, TempSymbol &res) {
            const Token token = peek();
            next();
            assert(isTypeToken(token));
            if(!match(Token("("))) return false;
            if(!parseExpression(off, res)) return false;
            if(!match(Token(")"))) return false;
            res = typeConversion(res, tokenToType(token), off);
            return true;
        }

        // 处理 <实参列表>
        bool parseArgList(int &off, vector<TempSymbol> &args) {
            if(expect(Token(")"))) return true;
            TempSymbol arg;
            if(!parseExpression(off, arg)) return false;
            args.push_back(arg);
            if(expect(Token(")"))) return true;
            if(!match(Token(","))){
                addErrNow();
                return false;
            }
            return parseArgList(off, args);
        }

        // 处理 <函数调用表达式>
        bool parseFuncCallExpr(int &off, TempSymbol &res) {
            const Token token = peek();
            next();
            assert(isFuncIdentifier(token));
            const Symbol funSymbol = getIdentifier(token);
            if(!match(Token("("))){
                addErrNow();
                return false;
            }
            vector<TempSymbol> args;
            if(!parseArgList(off, args)) return false;
            if(!match(Token(")"))){
                addErrNow();
                return false;
            }
            bool isSuccess;
            res = handleCallFunction(funSymbol, args, off, isSuccess);
            if(!isSuccess) return false;
            return true;
        }

        // 处理 <原子表达式>
        bool parseAtomExpr(int &off, TempSymbol &res) {
            if(match(Token("("))){
                if(!parseExpression(off, res)) return false;
                return match(Token(")"));
            }
            else if(expect(Token("Int")) || expect(Token("Float")) || expect(Token("Char")) ||
                    expect(Token("Bool")) || expect(Token("Void"))){
                return parseTypeCastExpr(off, res);
            }
            else if(expect(lexer::TokenType::I)){
                if(expect(&Parser::isVarIdentifier) || expect(&Parser::isValIdentifier)){
                    res = TempSymbol(getIdentifier(peek()));
                    next();
                    return true;
                }
                else if(expect(&Parser::isFuncIdentifier)){
                    return parseFuncCallExpr(off, res);
                }
                else{
                    addErrNow("Undefined identifier.");
                    return false;
                }
            }
            else if(expect(lexer::TokenType::CI) || expect(lexer::TokenType::CF) || expect(lexer::TokenType::CC) ||
                    expect(lexer::TokenType::CB)){
                const Token token = peek();
                next();
                res.kind = SymbolKind::CONST;
                if(token.type == lexer::TokenType::CI){
                    res.type = &INT;
                    assert(token.id - 1 <= static_cast<int>(ci.size()));
                    res.ptr = ci[token.id - 1];
                }
                else if(token.type == lexer::TokenType::CF){
                    res.type = &FLOAT;
                    assert(token.id - 1 <= static_cast<int>(cf.size()));
                    res.ptr = cf[token.id - 1];
                }
                else if(token.type == lexer::TokenType::CC){
                    res.type = &CHAR;
                    res.ptr = static_cast<char>(token.id);
                }
                else if(token.type == lexer::TokenType::CB){
                    res.type = &BOOL;
                    res.ptr = (token.id != 0);
                }
                else
                    assert(0);
                return true;
            }
            else{
                addErrNow();
                return false;
            }
        }

        // 处理 <项>
        bool parseTerm(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseAtomExpr(off, lhs)) return false;
            while(expect(Token("*")) || expect(Token("/"))){
                const Token token = peek();
                next();
                TempSymbol rhs;
                if(!parseTerm(off, rhs)) return false;
                bool isSuccess;
                lhs = handleMulOrDiv(lhs, rhs, token, off, isSuccess);
                if(!isSuccess) return false;
            }
            res = lhs;
            return true;
        }

        // 处理 <算术表达式>
        bool parseArithExpr(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseTerm(off, lhs)) return false;
            while(expect(Token("+")) || expect(Token("-"))){
                const Token token = peek();
                next();
                TempSymbol rhs;
                if(!parseTerm(off, rhs)) return false;
                lhs = handleAddOrSub(lhs, rhs, token, off);
            }
            res = lhs;
            return true;
        }

        // 处理 <关系表达式>
        bool parseRelExpr(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseArithExpr(off, lhs)) return false;
            if(expect(Token(">")) || expect(Token("<")) || expect(Token("==")) || expect(Token(">=")) ||
               expect(Token("<=")) || expect(Token("!="))){
                const Token token = peek();
                next();
                TempSymbol rhs;
                if(!parseArithExpr(off, rhs)) return false;
                if(lhs.type == &VOID || rhs.type == &VOID){
                    addErrNow("cannot compare Void.");
                    return false;
                }
                res = handleRelation(lhs, rhs, token, off);
                return true;
            }
            res = lhs;
            return true;
        }

        // 处理 <逻辑非表达式>
        bool parseLogicNotExpr(int &off, TempSymbol &res) {
            if(match(Token("!"))){
                TempSymbol rhs;
                if(!parseRelExpr(off, rhs)) return false;
                if(rhs.type != &BOOL){
                    addImplicitTypeConversionWarn();
                    rhs = typeConversion(rhs, &BOOL, off);
                }
                if(rhs.type == &VOID){
                    addErrNow("cannot ! Void.");
                    return false;
                }
                res = handleLogicNot(rhs, off);
                return true;
            }
            else return parseRelExpr(off, res);
        }

        // 处理 <逻辑与表达式>
        bool parseLogicAndExpr(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseLogicNotExpr(off, lhs)) return false;
            while(match(Token("&&"))){
                TempSymbol rhs;
                if(!parseLogicNotExpr(off, rhs)) return false;
                if(lhs.type == &VOID || rhs.type == &VOID){
                    addErrNow("cannot && Void.");
                    return false;
                }
                lhs = handleLogicAnd(lhs, rhs, off);
            }
            res = lhs;
            return true;
        }

        // 处理 <逻辑或表达式>
        bool parseLogicOrExpr(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseLogicAndExpr(off, lhs)) return false;
            while(match(Token("||"))){
                TempSymbol rhs;
                if(!parseLogicAndExpr(off, rhs)) return false;
                if(lhs.type == &VOID || rhs.type == &VOID){
                    addErrNow("cannot || Void.");
                    return false;
                }
                lhs = handleLogicOr(lhs, rhs, off);
            }
            res = lhs;
            return true;
        }

        // 处理 <逻辑表达式>
        bool parseLogicExpr(int &off, TempSymbol &res) {
            return parseLogicOrExpr(off, res);
        }

        // 处理 <赋值表达式>
        bool parseAssignExpr(int &off, TempSymbol &res) {
            if(expect(&Parser::isVarIdentifier)){
                res = TempSymbol(getIdentifier(peek()));
                next();
                assert(res.type != &VOID);
                if(match(Token("="))){
                    TempSymbol src;
                    if(!parseExpression(off, src)) return false;
                    if(src.type != res.type){
                        addImplicitTypeConversionWarn();
                        src = typeConversion(src, res.type, off);
                    }
                    if(src.type == &VOID){
                        addErrNow("Cannot assign var with Void.");
                        return false;
                    }
                    mov(res, src, off);
                    return true;
                }
                else{
                    prev();
                }
            }
            return parseLogicExpr(off, res);
        }

        // 处理 <表达式>
        bool parseExpression(int &off, TempSymbol &res) {
            return parseAssignExpr(off, res);
        }

        // 处理 <表达式语句>
        bool parseExprStmt(int &off, TempSymbol &res) {
            if(!parseExpression(off, res)) return false;
            return match(Token(";"));
        }

        // 处理 <语句>
        bool parseStmt(int &off, const Symbol &funSymbol) {
            if(expect(Token("var"))) return parseVarDefStmt(off);
            if(expect(Token("val"))) return parseValDefStmt(off);
            if(expect(Token("if"))) return parseIfStmt(off, funSymbol);
            if(expect(Token("while"))) return parseWhileStmt(off, funSymbol);
            if(expect(Token("return"))) return parseReturnStmt(off, funSymbol);
            TempSymbol res{};
            return parseExprStmt(off, res);
        }

        // 处理 <语句列表>
        bool parseStmtList(int &off, const Symbol &funSymbol) {
            if(expect(Token("}"))) return true;
            if(expect(Token("{"))){
                if(!parseCodeBlock(off, funSymbol)) return false;
                return parseStmtList(off, funSymbol);
            }
            else{
                if(!parseStmt(off, funSymbol)) return false;
                return parseStmtList(off, funSymbol);
            }
        }

        // 处理 <代码块>
        bool parseCodeBlock(int &off, const Symbol &funSymbol) {
            if(!match(Token("{"))){
                addErr("Expect '{'.");
                return false;
            }
            if(!parseStmtList(off, funSymbol)) return false;
            if(!match(Token("}"))){
                addErr("Expect '}'.");
                return false;
            }
            return true;
        }

        // 处理 <参数列表>
        bool parseParamList(ParamInfo &paramTable, int &off) {
            if(expect(Token(")"))){
                return true;
            }
            else if(expect(lexer::TokenType::I)){
                // SymbolTable &symbolTable = symbolTableStack.back();
                const Token token = peek();
                next();
                const SymbolType *type = nullptr;
                if(symbolTableStack[level].findByToken(token) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                if(!match(Token(":"))){
                    addErrNow("Expect ':'.");
                    return false;
                }
                if(expect(lexer::TokenType::K)){
                    if(const Token tk = peek(); tk == Token("Int")) type = &INT;
                    else if(tk == Token("Float")) type = &FLOAT;
                    else if(tk == Token("Char")) type = &CHAR;
                    else if(tk == Token("Bool")) type = &BOOL;
                    else if(tk == Token("Void")){
                        addErrNow("Param cannot be Void.");
                        return false;
                    }
                    else{
                        addErrNow("Undefined type.");
                        return false;
                    }
                    next();
                }
                else{
                    addErrNow("Expect type.");
                    return false;
                }
                symbolTableStack[level].push_back(Symbol{
                    token, type, SymbolKind::VAL, std::pair<int, int>{level, off}, ++cntSymbol
                });
                paramTable.emplace_back(token, type, SymbolKind::VAL, std::pair<int, int>(level, off), cntSymbol);
                off += type->size();
                if(match(Token(","))) parseParamList(paramTable, off);
                else if(expect(Token(")"))) return true;
                else{
                    addErrNow();
                    return false;
                }
            }
            else{
                addErrNow();
                return false;
            }
        }

        // 处理 <函数定义>
        bool parseFunctionDef() {
            const int JMP = static_cast<int>(ans.size());
            const int JMPM = static_cast<int>(mid.size());
            ans.emplace_back("JMP", "?");
            mid.emplace_back("JMP", "?");
            assert(!symbolTableStack.empty());
            Token name{}; // 函数名
            const SymbolType *type; // 函数类型
            auto *funInfo = new FunInfo(level, 16, 16, new ParamInfo, static_cast<int>(ans.size()),
                                        static_cast<int>(mid.size())); // 函数信息表
            // funInfo->level = level;
            // funInfo->off = 16; // 返回地址基址，返回值地址偏移量，跳转指令地址，全局变量偏移量
            // funInfo->stackSize = 16;
            // funInfo->paramInfoPtr = new ParamInfo();
            // funInfo->entry = static_cast<int>(ans.size());
            assert(match(Token("fun")));
            bool isMain = false;
            if(expect(lexer::TokenType::I)){
                name = peek();
                if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                assert(name.id <= static_cast<int>(I.size()));
                if(I[name.id - 1] == "main"){
                    isMain = true;
                }
                next();
            }
            else{
                addErrNow();
                return false;
            }
            if(!match(Token("("))){
                addErrNow("Expect '('.");
                return false;
            }
            ++level;
            symbolTableStack.emplace_back(global);
            if(!parseParamList(*funInfo->paramInfoPtr, funInfo->stackSize)) return false;
            if(isMain && !funInfo->paramInfoPtr->empty()){
                addErrNow("main's param must be empty");
                return false;
            }
            if(!match(Token(")"))){
                addErrNow("Expect ')'.");
                return false;
            }
            if(!match(Token(":"))){
                addErrNow("Expect ':'.");
                return false;
            }
            if(expect(lexer::TokenType::K)){
                if(const Token token = peek(); token == Token("Int")) type = &INT;
                else if(peek() == Token("Float")) type = &FLOAT;
                else if(peek() == Token("Char")) type = &CHAR;
                else if(peek() == Token("Bool")) type = &BOOL;
                else if(peek() == Token("Void")) type = &VOID;
                else{
                    addErrNow("Undefined type.");
                    return false;
                }
                next();
            }
            else{
                addErrNow("Expect type.");
                return false;
            }
            const int funPos = static_cast<int>(symbolTableStack[level - 1].size());
            const Symbol funSymbol{name, type, SymbolKind::FUN, funInfo, 0};
            symbolTableStack[level - 1].push_back(funSymbol);
            parseCodeBlock(funInfo->stackSize, funSymbol);
            symbolTableStack.pop_back();
            --level;
            assert(level + 1 == static_cast<int>(symbolTableStack.size()));
            if(type != &VOID){
                ans.emplace_back("ERR", std::to_string(ans.size()));
                mid.emplace_back("ERR", std::to_string(mid.size()));
            }
            else{
                mov("BX", 8, 4);
                mov("DS", 0, 4);
                jmp("BX");
                mid.emplace_back("RET", "_");
            }
            ans[JMP] = Quad("JMP", std::to_string(ans.size()));
            mid[JMPM] = Quad("JMP", std::to_string(mid.size()));
            if(isMain) mainPos = funPos;
            return true;
        }

        // 处理 <定义语句>
        bool parseDefinitionStmt(int &off) {
            if(ptr == str.end()){
                addErrNow("Expect Token.");
                return false;
            }
            if(*ptr == Token("fun")){
                return parseFunctionDef();
            }
            else if(*ptr == Token("var")){
                return parseVarDefStmt(off);
            }
            else if(*ptr == Token("val")){
                return parseValDefStmt(off);
            }
            else{
                addErrNow("Unexpected token.");
                return false;
            }
        }

        // 处理 <定义语句列表'>
        bool parseDefStmtListTail(int &off) {
            if(ptr == str.end()) return true;
            if(!parseDefinitionStmt(off)) return false;
            return parseDefStmtListTail(off);
        }

        // 处理 <定义语句列表>
        bool parseDefStmtList(int &off) {
            if(!parseDefinitionStmt(off)) return false;
            return parseDefStmtListTail(off);
        }

        // 处理 <程序>
        bool parseProgram() {
            symbolTableStack.emplace_back(global);
            const int STACK_ALLOC = static_cast<int>(ans.size());
            ans.emplace_back("STACK_ALLOC", "0");
            ans.emplace_back("MOV", "0", "[0: 4]");
            ans.emplace_back("MOV", "0", "[4: 4]");
            ans.emplace_back("MOV", "0", "[8: 4]");
            ans.emplace_back("MOV", "0", "[12: 4]");
            level = 0;
            if(int off = 16; !parseDefStmtList(off)){
                return false;
            }
            else if(mainPos == -1){
                addErr("Have no main function!");
                return false;
            }
            else{
                bool isSuccess;
                const TempSymbol res = handleCallFunction(symbolTableStack[0][mainPos], {}, off, isSuccess);
                if(res.type != &VOID)
                    ans.emplace_back(
                        "MOV", offsetToString(std::get<std::pair<int, int> >(res.ptr).second, res.type->size()), "[0: 4]");
                ans[STACK_ALLOC] = Quad("STACK_ALLOC", std::to_string(off));
                // 反填
                for(const auto &[id, funName] : allocQuat){
                    assert(ans[id].op == "STACK_ALLOC");
                    const auto it = symbolTableStack[0].findByToken(funName);
                    assert(it != symbolTableStack[0].end());
                    ans[id] = {"STACK_ALLOC", std::to_string(std::get<FunInfo *>(it->ptr)->stackSize)};
                }
                for(const auto &[id, funName] : freeQuat){
                    assert(ans[id].op == "STACK_FREE");
                    const auto it = symbolTableStack[0].findByToken(funName);
                    assert(it != symbolTableStack[0].end());
                    ans[id] = {"STACK_FREE", std::to_string(std::get<FunInfo *>(it->ptr)->stackSize)};
                }
                return true;
            }
        }

    public:
        Parser() = default;

        void clean() {
            str.clear();
            hasParsed = false;
        }

        void changeStr(const vector<Token> &newStr, const vector<int> &newCI, const vector<float> &newCF,
                       const vector<string> &newI) {
            str = newStr;
            ci = newCI;
            cf = newCF;
            I = newI;
            hasParsed = false;
        }

        bool run() {
            cntSymbol = 0;
            cntTempSymbol = 0;
            ptr = str.begin();
            ans.clear();
            mainPos = -1;
            err.clear();
            warn.clear();
            hasParsed = true;
            freeQuat.clear();
            allocQuat.clear();
            global.clear();
            return parseProgram();
        }

        [[nodiscard]] vector<string> getErr() const {
            return err;
        }

        [[nodiscard]] vector<string> getWarn() const {
            return warn;
        }

        [[nodiscard]] vector<Quad> getRes() const {
            return ans;
        }

        [[nodiscard]] vector<Quad> getMid() const {
            return mid;
        }
        [[nodiscard]] vector<Symbol> getGlobal() const {
            return global;
        }
    };
}


#endif //PARSER_H
