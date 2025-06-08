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

namespace parser{
    using std::string;
    using std::vector;
    using lexer::Token;
    using namespace symbol;

    struct Quad {
        string op, arg1, arg2, result;

        Quad(string op, string arg1, string arg2 = "", string res = "") : op(std::move(op)), arg1(std::move(arg1)),
                                                                          arg2(std::move(arg2)),
                                                                          result(std::move(res)) {
        }

        friend std::ostream &operator<<(std::ostream &os, const Quad &q) {
            return os << "(" << q.op
                   << ", " << (q.arg1.empty() ? "_" : q.arg1)
                   << ", " << (q.arg2.empty() ? "_" : q.arg2)
                   << ", " << (q.result.empty() ? "_" : q.result)
                   << ")";
        }
    };


    class Parser {
    private:
        vector<Token> str;
        vector<int> ci;
        vector<float> cf;
        vector<string> I;
        vector<Quad> ans;
        int firstQuad{};
        vector<string> err;
        vector<string> warn;
        bool hasParsed{};
        vector<Token>::const_iterator ptr;
        vector<SymbolTable> symbolTableStack;
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
            err.push_back(output);
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
            for(const auto &symbolTable : std::views::reverse(symbolTableStack)){
                if(auto it = symbolTable.findByToken(token); it != symbolTable.end() && it->kind == SymbolKind::FUN)
                    return true;
            }
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

        [[nodiscard]] static string offsetToString(const int off, const int size) {
            return "[DS +" + std::to_string(off) + ", " + std::to_string(size) + "]";
        }

        void push(const string &reg, int &off) {
            assert(reg == "BX" || reg == "DS" || reg == "ES");
            ans.emplace_back(
                "MOV",
                offsetToString(off, 4),
                reg);
            off += 4;
        }

        void pop(const string &reg, const int off) {
            assert(reg == "BX" || reg == "DS" || reg == "ES");
            ans.emplace_back(
                "MOV",
                reg,
                offsetToString(off, 4));
        }

        void movConst(const int dest, const int val) {
            ans.emplace_back("MOV", offsetToString(dest, 4), std::to_string(val));
        }

        void movConst(const int dest, const float val) {
            ans.emplace_back("MOV", offsetToString(dest, 4), std::to_string(std::bit_cast<int>(val)));
        }

        void movConst(const int dest, const char val) {
            ans.emplace_back("MOV", offsetToString(dest, 1), std::to_string(static_cast<int>(val)));
        }

        void movConst(const int dest, const bool val) {
            ans.emplace_back("MOV", offsetToString(dest, 1), std::to_string(static_cast<int>(val)));
        }

        void mov(const string &dest, const string &src, const int size) {
            assert(dest == "BX" || dest == "DS" || dest == "ES");
            assert(src == "BX" || src == "DS" || dest == "ES");
            if(dest == src) return;
            assert(size == 4);
            ans.emplace_back("MOV", dest, src);
        }

        void mov(const string &reg, const int &src, const int size) {
            assert(reg == "BX" || reg == "DS");
            ans.emplace_back(
                "MOV",
                reg,
                offsetToString(src, size));
        }

        void mov(const int &dest, const string &reg, const int size) {
            assert(reg == "BX" || reg == "DS");
            ans.emplace_back(
                "MOV",
                offsetToString(dest, size),
                reg);
        }

        void mov(const std::pair<int, int> &dest, const string &reg, const int size, int &off) {
            assert(reg == "BX" || reg == "DS");
            auto [dlv, doff] = dest;
            if(dlv == level) return mov(doff, reg, size);
            else{
                if(reg == "DS"){
                    const int tmp = off;
                    off += 4;
                    push("BX", off);
                    mov("BX", "DS", 4);
                    mov("DS", 12 + 4 * dlv, 4);
                    mov(doff, "BX", size);
                    mov("DS", "BX", 4);
                    pop("BX", tmp);
                }
                else{
                    const int tmp = off;
                    off += 4;
                    push("ES", off);
                    mov("ES", "DS", 4);
                    mov("DS", 12 + 4 * dlv, 4);
                    mov(doff, reg, size);
                    mov("DS", "ES", 4);
                    pop("ES", tmp);
                }
            }
        }

        // BX <- [地址]
        void mov(const string &reg, const std::pair<int, int> &src, const int size, int &off) {
            assert(reg == "BX");
            auto [slv, soff] = src;
            if(slv == level) return mov(reg, soff, size);
            else{
                int tmp = off;
                push("ES", off);
                mov("ES", "DS", 4);
                mov("DS", 12 + 4 * slv, 4);
                mov("BX", soff, size);
                mov("DS", "ES", 4);
                pop("ES", tmp);
            }
        }

        // [偏移量] <- [偏移量]
        void mov(const int dest, const int src, const int size) {
            ans.emplace_back("MOV", offsetToString(dest, size), offsetToString(src, size));
        }

        // [偏移量] <- [地址]
        void mov(const int dest, const std::pair<int, int> &src, const int size, int &off) {
            auto [slv, soff] = src;
            if(slv == level) return mov(dest, soff, size);
            else{
                mov("BX", src, size, off);
                mov(dest, "BX", size);
            }
        }

        // [偏移量] <- 临时变量
        void mov(const int dest, const TempSymbol &src, int &off) {
            if(src.kind == SymbolKind::CONST){
                if(src.type == &INT) return movConst(dest, std::get<int>(src.ptr));
                else if(src.type == &FLOAT) return movConst(dest, std::get<float>(src.ptr));
                else if(src.type == &CHAR) return movConst(dest, std::get<char>(src.ptr));
                else if(src.type == &BOOL) return movConst(dest, std::get<bool>(src.ptr));
                else throw std::runtime_error("Wrong type on get const val.");
            }
            else{
                assert(src.kind == SymbolKind::VAL || src.kind == SymbolKind::VAR);
                mov(dest, std::pair<int, int>(std::get<std::pair<int, int> >(src.ptr)), src.type->size(), off);
            }
        }

        // 临时变量 <- 临时变量，带类型类型检查，可选警告隐式类型转换
        void mov(const TempSymbol &dest, const TempSymbol &src, int &off) {
            // TODO
        }

        TempSymbol typeConversion(const TempSymbol &symbol, const SymbolType *type, int &off) {
            // TODO
        }

        TempSymbol toLocal(const TempSymbol &symbol, int &off) {
            // TODO
        }

        // 处理逻辑或运算
        TempSymbol handleLogicOr(const TempSymbol &x, const TempSymbol &y, int &off) {
            // TODO
        }

        // 处理逻辑与运算
        TempSymbol handleLogicAnd(const TempSymbol &x, const TempSymbol &y, int &off) {
            // TODO
        }

        // 处理逻辑非运算
        TempSymbol handleLogicNot(const TempSymbol &x, int &off) {
            // TODO
        }

        // 处理关系运算
        TempSymbol handleRelation(const TempSymbol &x, const TempSymbol &y, const Token &token, int &off) {
            // TODO
        }

        // 处理加减运算
        TempSymbol handleAddOrSub(const TempSymbol &x, const TempSymbol &y, const Token &token, int &off) {
            // TODO
        }

        // 处理乘除运算
        TempSymbol handleMulOrDiv(const TempSymbol &x, const TempSymbol &y, const Token &token, int &off) {
            // TODO
        }

        // 处理函数调用
        TempSymbol handleCallFunction(const Symbol &funSymbol, const vector<TempSymbol> &args, int &off, bool &isSuccess) {
            // TODO
        }


        // 处理 <初始化列表后缀'>
        bool parseInitListSuffixTail(int &off) {
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
            symbolTableStack[level].push_back(Symbol{name, res.type, res.kind, std::pair<int, int>(level, off)});
            const int tmp = off;
            off += static_cast<int>(res.type->size());
            mov(tmp, res, off);
            return parseInitListSuffixTail(off);
        }


        // 处理 <初始化列表后缀>
        bool parseInitListSuffix(int &off, const Token &lastToken) {
            assert(match(Token("=")));
            TempSymbol res{};
            if(!parseExpression(off, res)) return false;
            symbolTableStack[level].push_back(Symbol{lastToken, res.type, res.kind, std::pair<int, int>(level, off)});
            const int tmp = off;
            off += static_cast<int>(res.type->size());
            mov(tmp, res, off);
            return parseInitListSuffixTail(off);
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
                if(!isTypeToken(token)){
                    addErrNow("Not a type.");
                    return false;
                }
                const SymbolType *type = tokenToType(token);
                next();
                for(const Token &name : identList){
                    if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                        addErrNow("Multi define.");
                        return false;
                    }
                    symbolTableStack[level].emplace_back(name, type, SymbolKind::VAR, std::pair<int, int>(level, off));
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
        bool parseVarDefSuffix(int &off, const Token &lastToken) {
            if(expect(Token("="))) return parseInitListSuffix(off, lastToken);
            else if(expect(Token(":")) || expect(Token(","))){
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
            return parseVarDefSuffix(off, name);
        }

        // 处理 <常量定义语句>
        bool parseValDefStmt(int &off) {
            // TODO
        }

        // 处理 <if 语句>
        bool parseIfStmt(int &off, const Symbol &funSymbol) {
            assert(match(Token("if")));
            if(!match(Token("("))) return false;
            TempSymbol res;
            parseExpression(off, res);
            if(!match(Token(")"))) return false;
            // TODO
            ++level;
            int newOffset = level * 4 + 12;
            if(parseCodeBlock(newOffset, funSymbol)) return false;
            if(match(Token("else"))){
                // TODO
                if(!parseCodeBlock(newOffset, funSymbol)) return false;
            }
            else{
                // TODO
            }
            // TODO
            --level;
            return true;
        }

        // 处理 <while 语句>
        bool parseWhileStmt(int &off, const Symbol &funSymbol) {
            assert(match(Token("while")));
            if(!match(Token("("))) return false;
            TempSymbol res;
            parseExpression(off, res);
            if(!match(Token(")"))) return false;
            // TODO
            ++level;
            int newOffset = level * 4 + 12;
            if(parseCodeBlock(newOffset, funSymbol)) return false;
            // TODO
            --level;
            return true;
        }

        // 处理 <return 语句>
        bool parseReturnStmt(int &off, const Symbol &funSymbol) {
            assert(match(Token("return")));
            // TODO
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
            Symbol funSymbol = getIdentifier(token);
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
                if(expect(&isVarIdentifier) || expect(&isValIdentifier)){
                    res = TempSymbol(getIdentifier(peek()));
                    next();
                    return true;
                }
                else if(&isFuncIdentifier){
                    return parseFuncCallExpr(off, res);
                }
                else{
                    addErrNow("Undefined identifier.");
                    return false;
                }
            }
            else if(expect(lexer::TokenType::CI) || expect(lexer::TokenType::CF) || expect(lexer::TokenType::CC) ||
                    expect(lexer::TokenType::CB)){
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
                Token token = peek();
                next();
                TempSymbol rhs;
                if(!parseTerm(off, rhs)) return false;
                lhs = handleMulOrDiv(lhs, rhs, token, off);
            }
            res = lhs;
            return true;
        }

        // 处理 <算术表达式>
        bool parseArithExpr(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseTerm(off, lhs)) return false;
            while(expect(Token("+")) || expect(Token("-"))){
                Token token = peek();
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
            parseArithExpr(off, lhs);
            if(expect(Token(">")) || expect(Token("<")) || expect(Token("==")) || expect(Token(">=")) ||
               expect(Token("<=")) || expect(Token("!="))){
                Token token = peek();
                next();
                TempSymbol rhs;
                if(!parseArithExpr(off, rhs)) return false;
                res = handleRelation(lhs, rhs, token, off);
                return true;
            }
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
                res = handleLogicNot(rhs, off);
                return true;
            }
            else return parseRelExpr(off, res);
        }

        // 处理 <逻辑与表达式>
        bool parseLogicAndExpr(int &off, TempSymbol &res) {
            TempSymbol lhs;
            if(!parseLogicNotExpr(off, lhs)) return false;
            if(lhs.type != &BOOL){
                addImplicitTypeConversionWarn();
                lhs = typeConversion(lhs, &BOOL, off);
            }
            while(match(Token("&&"))){
                TempSymbol rhs;
                if(!parseLogicNotExpr(off, rhs)) return false;
                if(rhs.type != &BOOL){
                    addImplicitTypeConversionWarn();
                    rhs = typeConversion(rhs, &BOOL, off);
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
            if(lhs.type != &BOOL){
                addImplicitTypeConversionWarn();
                lhs = typeConversion(lhs, &BOOL, off);
            }
            while(match(Token("||"))){
                TempSymbol rhs;
                if(!parseLogicAndExpr(off, rhs)) return false;
                if(rhs.type != &BOOL){
                    addImplicitTypeConversionWarn();
                    rhs = typeConversion(rhs, &BOOL, off);
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
            if(expect(isVarIdentifier)){
                res = TempSymbol(getIdentifier(peek()));
                if(match(Token("="))){
                    TempSymbol src;
                    if(!parseExpression(off, src)) return false;
                    if(src.type != res.type){
                        addImplicitTypeConversionWarn();
                        src = typeConversion(src, res.type, off);
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
            if(expect(Token("{"))) return parseCodeBlock(off, funSymbol) && parseStmtList(off, funSymbol);
            return parseStmt(off, funSymbol) && parseStmtList(off, funSymbol);
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
                Token token = peek();
                const SymbolType *type = nullptr;
                if(symbolTableStack[level].findByToken(token) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                next();
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
                if(!match(Token(","))){
                    addErrNow("Expect ','.");
                    return false;
                }
                symbolTableStack[level].push_back(Symbol{
                    token, type, SymbolKind::VAL, std::pair<int, int>{level, off}
                });
                paramTable.push_back(Symbol{token, type, SymbolKind::VAL, std::pair<int, int>(level, off)});
                off += type->size();
                return parseParamList(paramTable, off);
            }
            else{
                addErrNow();
                return false;
            }
        }

        // 处理 <函数定义>
        bool parseFunctionDef() {
            ans.emplace_back("JMP", "0");
            const int JMP = static_cast<int>(ans.size()) - 1;
            assert(!symbolTableStack.empty());
            // SymbolTable &symbolTable = symbolTableStack.back();
            Token name{}; // 函数名
            const SymbolType *type; // 函数类型
            auto *funInfo = new FunInfo(level, 16, 16, new ParamInfo, static_cast<int>(ans.size())); // 函数信息表
            // funInfo->level = level;
            // funInfo->off = 16; // 返回值地址偏移量，返回地址基址，跳转指令地址，全局变量偏移量
            // funInfo->stackSize = 16;
            // funInfo->paramInfoPtr = new ParamInfo();
            // funInfo->entry = static_cast<int>(ans.size());
            assert(match(Token("fun")));
            if(expect(lexer::TokenType::I)){
                name = peek();
                if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                    addErrNow("Multi define function.");
                    return false;
                }
                assert(name.id <= static_cast<int>(I.size()));
                if(I[name.id - 1] == "main"){
                    firstQuad = static_cast<int>(ans.size());
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
            symbolTableStack.emplace_back();
            if(!parseParamList(*funInfo->paramInfoPtr, funInfo->stackSize)) return false;
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
            symbolTableStack[level - 1].emplace_back(name, type, SymbolKind::FUN, funInfo);
            parseCodeBlock(funInfo->stackSize, symbolTableStack[level - 1].back());
            symbolTableStack.pop_back();
            --level;
            assert(level + 1 == static_cast<int>(symbolTableStack.size()));
            if(type != &VOID){
                ans.emplace_back("ERR", std::to_string(ans.size()));
            }
            ans[JMP] = Quad("JMP", std::to_string(ans.size()));
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
                // TODO: var 识别
            }
            else if(*ptr == Token("val")){
                // TODO: val 识别
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
            symbolTableStack.emplace_back();
            ans.emplace_back("STACK_ALLOC", "0");
            const int STACK_ALLOC = static_cast<int>(ans.size()) - 1;
            level = 0;
            if(int off = 0; !parseDefStmtList(off)){
                return false;
            }
            else if(firstQuad == -1){
                addErr("Have no main function!");
                return false;
            }
            else{
                ans[STACK_ALLOC] = Quad("STACK_ALLOC", std::to_string(off));
                ans.emplace_back("JMP", std::to_string(firstQuad));
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
            ptr = str.begin();
            ans.clear();
            firstQuad = -1;
            err.clear();
            warn.clear();
            hasParsed = true;
            return parseProgram();
        }

        [[nodiscard]] vector<string> getErr() const {
            return err;
        }

        [[nodiscard]] vector<Quad> getRes() const {
            return ans;
        }
    };
}


#endif //PARSER_H
