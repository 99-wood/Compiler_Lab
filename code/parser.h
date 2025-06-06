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
        friend std::ostream& operator<< (std::ostream& os, const Quad& q) {
            return os << "(" << q.op
                      << ", " << (q.arg1.empty() ? "_" : q.arg1)
                      << ", " << (q.arg2.empty() ? "_" : q.arg2)
                      << ", " << (q.result.empty() ? "_" : q.result);
        }
    };

    enum class TVAL {
        Int, Float, Char, Bool, Void, Array, Struct
    };
    TVAL tokenToTVAL(const Token& tk) {
        if(tk.type != lexer::TokenType::K){
            throw std::runtime_error("Error on trans token to TVAL. Not a type.");
        }
        switch(tk.id){
            case 1 : return TVAL::Int;
            case 2 : return TVAL::Float;
            case 3 : return TVAL::Char;
            case 4 : return TVAL::Bool;
            case 5 : return TVAL::Void;
            default: throw std::runtime_error("Error on trans token to TVAL. Not a type.");
        }
    }
    struct SymbolType {
        TVAL type;
        union TypeInfoPtr{
            void* nullPtr;
        }ptr;
    };

    static const SymbolType INT_PTR{TVAL::Int, {nullptr}};
    static const SymbolType FLOAT_PTR{TVAL::Float, {nullptr}};
    static const SymbolType CHAR_PTR{TVAL::Char, {nullptr}};
    static const SymbolType BOOL_PTR{TVAL::Bool, {nullptr}};
    static const SymbolType VOID_PTR{TVAL::Void, {nullptr}};

    using SymbolTypeTable = vector<SymbolType>;
    SymbolTypeTable symbolTypeTable{INT_PTR, FLOAT_PTR, CHAR_PTR, BOOL_PTR, VOID_PTR};

    enum class SymbolKind {
        FUN, CONST, TYPE, VAR, VAL
    };

    struct Symbol {
        Token token;
        SymbolType* type;
        SymbolKind* kind;
        union SymbolInfoPtr {

        }ptr;
    };

    class SymbolTable : public vector<Symbol>{
    public:
        SymbolTable::iterator findByToken(const Token& tk){
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
        vector<Quad> ans;
        int firstQuad{};
        vector<string> err;
        bool hasParsed{};
        vector<Token>::const_iterator ptr;

        enum class ErrType {
            OTHER
        };

        void addErrNow(const string& arg = "") {
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
        void addErr(const string& arg) {
            err.push_back("[Wrong] " + arg);
            return;
        }

        // 处理 <函数定义>
        bool parseFunctionDef(SymbolTable& symbolTable){
            assert(ptr != str.end() && *ptr == Token("fun"));
            ++ptr;
            if(ptr != str.end() && ptr->type == lexer::TokenType::I){
                if(symbolTable.findByToken(*ptr) != symbolTable.end()){
                    addErrNow("multidef");
                    return false;
                }
            }
            else{
                addErrNow();
                return false;
            }
            
        }

        // 处理 <定义语句>
        bool parseDefinitionStmt(SymbolTable& symbolTable) {
            if(ptr == str.end()){
                addErrNow("Expect Token.");
                return false;
            }
            if(*ptr == Token("fun")){
                return parseFunctionDef(symbolTable);
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
        bool parseDefStmtListTail(SymbolTable& symbolTable) {
            if(ptr == str.end()) return true;
            if(parseDefinitionStmt(symbolTable)) return false;
            return parseDefStmtListTail(symbolTable);
        }

        // 处理 <定义语句列表>
        bool parseDefStmtList(SymbolTable& symbolTable) {
            if(parseDefinitionStmt(symbolTable)) return false;
            return parseDefStmtListTail(symbolTable);
        }

        // 处理 <程序>
        bool parseProgram() {
            SymbolTable globalSymbolTable;
            if(bool res = parseDefStmtList(globalSymbolTable); !res){
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
        void changeStr(const vector<Token>& newStr) {
            str = newStr;
            hasParsed = false;
            return;
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
