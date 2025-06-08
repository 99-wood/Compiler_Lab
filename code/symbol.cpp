//
// Created by admin on 2025/6/7.
//

#include "symbol.h"

namespace symbol{
    bool isTypeToken(const Token &token) {
        return token == Token("Int") || token == Token("Float") || token == Token("Char") ||
               token == Token("Bool") || token == Token("Void");
    }

    const SymbolType *tokenToType(const Token &token) {
        if(token == Token("Int")) return &INT;
        if(token == Token("Float")) return &FLOAT;
        if(token == Token("Char")) return &CHAR;
        if(token == Token("Bool")) return &BOOL;
        if(token == Token("Void")) return &VOID;
        throw std::runtime_error("Wrong on Token to Type");
    }

    TVAL tokenToTVAL(const Token &tk) {
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

    TempSymbol::TempSymbol(const Symbol &symbol) : type(symbol.type), kind(symbol.kind), ptr(symbol.ptr) {
        if(symbol.kind == SymbolKind::FUN || symbol.kind == SymbolKind::PROCESS || symbol.kind == SymbolKind::TYPE)
            throw std::runtime_error("Cannot trans Symbol to TempSymbol.");
        // else if(kind == SymbolKind::VAL || kind == SymbolKind::VAR){
        //     ptr.vPtr = symbol.ptr.vPtr;
        // }
        // else{
        //     assert(kind == SymbolKind::CONST);
        //     if(type == &INT) /*ptr.CI = symbol.ptr.CI*/;
        //     else if(type == &FLOAT) /*ptr.CF = symbol.ptr.CF*/;
        //     else if(type == &CHAR) /*ptr.CC = symbol.ptr.CC*/;
        //     else if(type == &BOOL) /*ptr.CB = symbol.ptr.CB*/;
        //     else throw std::runtime_error("Wrong type");
        // }
    }

    int TempSymbol::getVal() const {
        if(kind != SymbolKind::CONST) throw std::runtime_error("not a const number.");
        if(type == &INT) return std::get<int>(ptr);
        else if(type == &FLOAT) return std::bit_cast<int>(std::get<float>(ptr));
        else if(type == &CHAR) return static_cast<int>(std::get<char>(ptr));
        else if(type == &BOOL) return static_cast<int>(std::get<bool>(ptr));
        else throw std::runtime_error("Wrong type on get const val.");
    }

    // Symbol::Symbol(const Token &token, const TempSymbol &symbol) : token(token), type(symbol.type), kind(symbol.kind),
    //                                                             ptr() {
    //     if(symbol.kind == SymbolKind::FUN || symbol.kind == SymbolKind::PROCESS || symbol.kind == SymbolKind::TYPE)
    //         throw std::runtime_error("Invade TempSymbol type");
    //     else if(kind == SymbolKind::VAL || kind == SymbolKind::VAR){
    //         ptr.vPtr = symbol.ptr.vPtr;
    //     }
    //     else{
    //         assert(kind == SymbolKind::CONST);
    //         if(type == &INT) ptr.CI = symbol.ptr.CI;
    //         else if(type == &FLOAT) ptr.CF = symbol.ptr.CF;
    //         else if(type == &CHAR) ptr.CC = symbol.ptr.CC;
    //         else if(type == &BOOL) ptr.CB = symbol.ptr.CB;
    //         else throw std::runtime_error("Wrong type");
    //     }
    // }
} // symbol
