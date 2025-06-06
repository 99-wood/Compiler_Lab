//
// Created by admin on 2025/6/4.
//

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "lexer.h"

namespace parser{
    using std::string;
    using std::vector;
    using lexer::Token;

    struct Quad {
        string op, arg1, arg2, result;

        friend std::ostream &operator<<(std::ostream &os, const Quad &q) {
            return os << "(" << q.op
                   << ", " << (q.arg1.empty() ? "_" : q.arg1)
                   << ", " << (q.arg2.empty() ? "_" : q.arg2)
                   << ", " << (q.result.empty() ? "_" : q.result);
        }
    };

    enum class TVAL {
        Int, Float, Char, Bool, Void, Array, Struct
    };

    inline TVAL tokenToTVAL(const Token &tk) {
        if(tk.type != lexer::TokenType::K){
            throw std::runtime_error("Error on trans token to TVAL. Not a type.");
        }
        switch(tk.id){
            case 1: return TVAL::Int;
            case 2: return TVAL::Float;
            case 3: return TVAL::Char;
            case 4: return TVAL::Bool;
            case 5: return TVAL::Void;
            default: throw std::runtime_error("Error on trans token to TVAL. Not a type.");
        }
    }

    struct SymbolType {
        TVAL type;

        union TypeInfoPtr {
            void *nullPtr;
        } ptr;
    };

    constexpr SymbolType INT{TVAL::Int, {nullptr}};
    constexpr SymbolType FLOAT{TVAL::Float, {nullptr}};
    constexpr SymbolType CHAR{TVAL::Char, {nullptr}};
    constexpr SymbolType BOOL{TVAL::Bool, {nullptr}};
    constexpr SymbolType VOID{TVAL::Void, {nullptr}};

    using SymbolTypeTable = vector<SymbolType>;
    inline SymbolTypeTable symbolTypeTable{INT, FLOAT, CHAR, BOOL, VOID};

    enum class SymbolKind {
        FUN, PROCESS, CONST, TYPE, VAR, VAL
    };

    struct Symbol;

    using ParamInfo = vector<Symbol>;

    struct FunInfo {
        int level, off, entry;
        ParamInfo *paramInfoPtr;

        FunInfo() = default;

        FunInfo(const int level, const int off, const int paramNum, ParamInfo *paramInfoPtr,
                const int entry) : level(level), off(off),
                                   entry(entry), paramInfoPtr(paramInfoPtr) {
        }
    };

    struct Symbol {
        const Token token;
        const SymbolType *type;
        const SymbolKind kind;

        union SymbolInfoPtr {
            int vPtr;
            FunInfo *funPtr;
            int CI;
            float CF;
        } ptr;
    };

    class SymbolTable : public vector<Symbol> {
    public:
        iterator findByToken(const Token &tk) {
            auto pos = this->begin();
            while(pos != this->end()){
                if(pos->token == tk) return pos;
                ++pos;
            }
            return pos;
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
            ss << "[Wrong] Wrong on";
            ss << ptr - str.begin() + 1;
            ss << " token.";

            ss << " " + arg;
            string output;
            std::getline(ss, output);
            err.push_back(output);
            return;
        }

        void addErr(const string &arg) {
            err.push_back("[Wrong] " + arg);
            return;
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

        ParamInfo *parseParamList(ParamInfo &paramTable) {
        }

        // 处理 <函数定义>
        bool parseFunctionDef() {
            SymbolTable &symbolTable = symbolTableStack.back();
            Token name{};
            const SymbolType *type;
            auto *funInfo = new FunInfo;
            funInfo->level = level;
            funInfo->off = 12;
            funInfo->paramInfoPtr = new ParamInfo();
            funInfo->entry(ans.size());
            assert(match(Token("fun")));
            if(!empty() && expect(lexer::TokenType::I)){
                name = peek();
                if(symbolTable.findByToken(name) != symbolTable.end()){
                    addErrNow("Multi define function.");
                    return false;
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
            if(!parseParamList(*funInfo->paramInfoPtr)) return false;
            --level;
            if(!match(Token(")"))){
                addErrNow("Expect ')'.");
                return false;
            }
            if(expect(lexer::TokenType::K)){
                switch(peek()){
                    case Token("Int"): type = &INT;
                        break;
                    case Token("Float"): type = &FLOAT;
                        break;
                    case Token("Char"): type = &CHAR;
                        break;
                    case Token("Bool"): type = &BOOL;
                        break;
                    case Token("Void"): type = &VOID;
                        break;
                    default:
                        addErrNow("Undefined type.");
                        return false;
                }
                next();
            }
            else{
                addErrNow("Expect type.");
                return false;
            }
            symbolTable.push_back(Symbol{name, type, SymbolKind::FUN, {.funPtr = funInfo}});
        }

        // 处理 <定义语句>
        bool parseDefinitionStmt() {
            if(ptr == str.end()){
                addErrNow("Expect Token.");
                return false;
            }
            if(*ptr == Token("fun")){
                return parseFunctionDef();
            }
            else if(*ptr == Token("var")){
            }
            else if(*ptr == Token("val")){
            }
            else{
                addErrNow("Unexpected token.");
                return false;
            }
        }

        // 处理 <定义语句列表'>
        bool parseDefStmtListTail() {
            if(ptr == str.end()) return true;
            if(parseDefinitionStmt()) return false;
            return parseDefStmtListTail();
        }

        // 处理 <定义语句列表>
        bool parseDefStmtList() {
            if(parseDefinitionStmt()) return false;
            return parseDefStmtListTail();
        }

        // 处理 <程序>
        bool parseProgram() {
            symbolTableStack.clear();
            symbolTableStack.emplace_back();
            level = 0;
            if(!parseDefStmtList()){
                return false;
            }
            else if(firstQuad == -1){
                addErr("Have no main function!");
                return false;
            }
            else return true;
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
    };
}


#endif //PARSER_H
