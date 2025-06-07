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
        bool hasParsed{};
        vector<Token>::const_iterator ptr;
        vector<SymbolTable> symbolTableStack;
        int level{};

        enum class ErrType {
            OTHER
        };

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

        [[nodiscard]] bool expect(const Token &tk) const {
            if(ptr == str.end()) return false;
            return *ptr == tk;
        }

        [[nodiscard]] bool expect(const lexer::TokenType &tp) const {
            if(ptr == str.end()) return false;
            return ptr->type == tp;
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

        [[nodiscard]] bool empty() const {
            return ptr == str.end();
        }

        void next() {
            assert(ptr != str.end());
            ++ptr;
        }
        void mov(const int off, const TempSymbol &symbol) {
            if(symbol.kind == SymbolKind::CONST){
                string res;
                if(symbol.type == &INT) res = std::to_string(std::get<int>(symbol.ptr));
                else if(symbol.type == &FLOAT) res = std::to_string(std::bit_cast<int>(std::get<float>(symbol.ptr)));
                else if(symbol.type == &CHAR) res = std::to_string(static_cast<int>(std::get<char>(symbol.ptr)));
                else if(symbol.type == &VOID) res = std::to_string(static_cast<int>(std::get<bool>(symbol.ptr)));
                else throw std::runtime_error("Wrong type on mov.");
                ans.emplace_back(
                    "MOV",
                    "[DS +" + std::to_string(off) + ", " + std::to_string(symbol.type->size()) + "]",
                    res);
            }
            else{
                assert(symbol.kind == SymbolKind::VAL || symbol.kind == SymbolKind::VAR);
                ans.emplace_back(
                    "MOV",
                    "[DS +" + std::to_string(off) + ", " + std::to_string(symbol.type->size()) + "]",
                    "[DS +" + std::to_string(std::get<int>(symbol.ptr)) + ", " + std::to_string(symbol.type->size()) + "]");
            }
        }
        void mov(const string& reg, const TempSymbol &symbol) {
            assert(reg == "BX" || reg == "DS");
            if(symbol.kind == SymbolKind::CONST){
                string res;
                if(symbol.type == &INT) res = std::to_string(std::get<int>(symbol.ptr));
                else if(symbol.type == &FLOAT) res = std::to_string(std::bit_cast<int>(std::get<float>(symbol.ptr)));
                else if(symbol.type == &CHAR) res = std::to_string(static_cast<int>(std::get<char>(symbol.ptr)));
                else if(symbol.type == &VOID) res = std::to_string(static_cast<int>(std::get<bool>(symbol.ptr)));
                else throw std::runtime_error("Wrong type on mov.");
                ans.emplace_back("MOV", reg, res);
            }
            else{
                assert(symbol.kind == SymbolKind::VAL || symbol.kind == SymbolKind::VAR);
                ans.emplace_back("MOV", reg, "[DS +" + std::to_string(std::get<int>(symbol.ptr)) + ", " + std::to_string(symbol.type->size()) + "]");
            }
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
            if(!parseExprStmt(off, res)) return false;
            mov(off, res); // 初始化
            off += static_cast<int>(res.type->size());
            symbolTableStack[level].push_back(Symbol{name, res.type, res.kind, off});
            return parseInitListSuffixTail(off);
        }


        // 处理 <初始化列表后缀>
        bool parseInitListSuffix(int &off, const Token &lastToken) {
            assert(match(Token("=")));
            TempSymbol res{};
            if(!parseExprStmt(off, res)) return false;
            mov(off, res); // 初始化
            off += static_cast<int>(res.type->size());
            symbolTableStack[level].push_back(Symbol{lastToken, res.type, res.kind, off});
            return parseInitListSuffixTail(off);
        }

        // 处理 <标识符列表后缀>
        bool parseIdentListSuffix(int &off, vector<Token>& identList) {
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
                const SymbolType* type = tokenToType(token);
                next();
                for(const Token& name : identList){
                    if(symbolTableStack[level].findByToken(name) != symbolTableStack[level].end()){
                        addErrNow("Multi define.");
                        return false;
                    }
                    symbolTableStack[level].emplace_back(name, type, SymbolKind::VAR, off);
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
        bool parseIfStmt(int &off) {
            // TODO
        }

        // 处理 <while 语句>
        bool parseWhileStmt(int &off) {
            // TODO
        }

        // 处理 <return 语句>
        bool parseReturnStmt(int &off) {
            // TODO
        }

        // 处理 <表达式语句>
        bool parseExprStmt(int &off, TempSymbol &res) {
            // TODO
        }

        // 处理 <语句>
        bool parseStmt(int &off) {
            if(expect(Token("var"))) return parseVarDefStmt(off);
            if(expect(Token("val"))) return parseValDefStmt(off);
            if(expect(Token("if"))) return parseIfStmt(off);
            if(expect(Token("while"))) return parseWhileStmt(off);
            if(expect(Token("return"))) return parseReturnStmt(off);
            TempSymbol res{};
            return parseExprStmt(off, res);
        }

        // 处理 <语句列表>
        bool parseStmtList(int &off) {
            if(expect(Token("}"))) return true;
            if(expect(Token("{"))) return parseCodeBlock(off) && parseStmtList(off);
            return parseStmt(off) && parseStmtList(off);
        }

        // 处理 <代码块>
        bool parseCodeBlock(int &off) {
            if(!match(Token("{"))){
                addErr("Expect '{'.");
                return false;
            }
            if(!parseStmtList(off)) return false;
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
                symbolTableStack[level].push_back(Symbol{token, type, SymbolKind::VAL, off});
                paramTable.push_back(Symbol{token, type, SymbolKind::VAL, off});
                if(type == &INT || type == &FLOAT){
                    off += 4;
                }
                else{
                    assert(type == &CHAR || type == &BOOL);
                    off += 1;
                }
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
                assert(name.id <= I.size());
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
            --level;
            symbolTableStack[level].emplace_back(name, type, SymbolKind::FUN, funInfo);
            parseCodeBlock(funInfo->stackSize);
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
