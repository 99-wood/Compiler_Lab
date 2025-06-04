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

    static const SymbolType INT_PTR{TVAL::Int, nullptr};
    static const SymbolType FLOAT_PTR{TVAL::Float, nullptr};
    static const SymbolType CHAR_PTR{TVAL::Char, nullptr};
    static const SymbolType BOOL_PTR{TVAL::Bool, nullptr};
    static const SymbolType VOID_PTR{TVAL::Void, nullptr};

    using SymbolTypeTable = vector<SymbolType>;

    enum class SymbolKind {
        FUN, CONST, TYPE, VAR, VAL
    };

    struct Symbol {
        Token name;
        SymbolType* type;
        SymbolKind* kind;
        union SymbolInfoPtr {

        }ptr;
    };
    using SymbolTable = vector<Symbol>;

    class Parser {
    private:
        vector<Token> str;
        vector<Quad> ans;
        int firstQuad;
        vector<string> err;
        bool hasParsed;
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

        // 处理 <定义语句>
        bool parseDefinitionStmt() {
            if(ptr == str.end()){
                addErr("Expect Token.");
                return false;
            }
            if(*ptr == Token("fun")){

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
            if(parseDefStmtList() == false){
                return false;
            }
            else if(firstQuad == -1){
                addErr("Have no main function!");
                return false;
            }
            else return parseDefStmtList();
        }

    public:
        Parser() = default;
        void clean() {
            str.clear();
            hasParsed = false;
        }
        void changeStr(const vector<Token>& str) {
            this->str = str;
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
