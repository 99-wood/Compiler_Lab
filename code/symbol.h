//
// Created by admin on 2025/6/7.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include <string>
#include <utility>
#include <vector>
#include <variant>
#include "lexer.h"

namespace symbol{
    using std::string;
    using std::vector;
    using lexer::Token;

    enum class TVAL {
        Int, Float, Char, Bool, Void, Array, Struct
    };

    struct SymbolType {
        TVAL type;

        union TypeInfoPtr {
            void *nullPtr;
        } ptr;

        [[nodiscard]] int size() const {
            switch(type){
                case TVAL::Int: return 4;
                    break;
                case TVAL::Float: return 4;
                    break;
                case TVAL::Char: return 1;
                    break;
                case TVAL::Bool: return 1;
                    break;
                case TVAL::Void: return 0;
                    break;
                default: throw std::runtime_error("Undefined type.");
            }
        }

        [[nodiscard]] string toString() const {
            switch(type){
                case TVAL::Int: return "Int";
                    break;
                case TVAL::Float: return "Float";
                    break;
                case TVAL::Char: return "Char";
                    break;
                case TVAL::Bool: return "Bool";
                    break;
                case TVAL::Void: return "Void";
                    break;
                default: throw std::runtime_error("Undefined type.");
            }
        }
    };

    constexpr SymbolType INT{TVAL::Int, {nullptr}};
    constexpr SymbolType FLOAT{TVAL::Float, {nullptr}};
    constexpr SymbolType CHAR{TVAL::Char, {nullptr}};
    constexpr SymbolType BOOL{TVAL::Bool, {nullptr}};
    constexpr SymbolType VOID{TVAL::Void, {nullptr}};

    //    using SymbolTypeTable = vector<SymbolType>;

    //    inline SymbolTypeTable{INT, FLOAT, CHAR, BOOL, VOID};

    bool isTypeToken(const Token &token);

    //    const SymbolType* tokenToType(const Token &token);

    enum class SymbolKind {
        FUN, PROCESS, CONST, TYPE, VAR, VAL
    };

    inline std::ostream& operator<< (std::ostream& os, const SymbolKind &kind) {
        if(kind == SymbolKind::FUN) return os << "FUN";
        if(kind == SymbolKind::PROCESS) return os << "PROCESS";
        if(kind == SymbolKind::CONST) return os << "CONST";
        if(kind == SymbolKind::TYPE) return os << "TYPE";
        if(kind == SymbolKind::VAR) return os << "VAR";
        if(kind == SymbolKind::VAL) return os << "VAL";
        assert(0);
    }

    struct Symbol;

    using ParamInfo = vector<Symbol>;

    struct FunInfo {
        int level, off, stackSize, entry, entryM;
        ParamInfo *paramInfoPtr;

        FunInfo() = default;

        FunInfo(const int level, const int off, const int stackSize, ParamInfo *paramInfoPtr,
                const int entry, const int entryM) : level(level), off(off), stackSize(stackSize), entry(entry),
                                                     entryM(entryM),
                                                     paramInfoPtr(paramInfoPtr) {
        }
    };

    using SymbolInfoPtr = std::variant<
        std::pair<int, int>, // v
        FunInfo *, // funPtr
        int, // 用于 CI
        float, // CF
        char, // CC
        bool // CB
    >;

    struct TempSymbol {
        Token token{};
        const SymbolType *type{};
        SymbolKind kind{};

        SymbolInfoPtr ptr{};

        TempSymbol() = default;

        explicit TempSymbol(const Symbol &symbol);

        TempSymbol(const Token &token, const SymbolType *type, const SymbolKind &kind, SymbolInfoPtr ptr);

        [[nodiscard]] int getVal() const;
    };

    struct Symbol {
        const Token token;
        const SymbolType *type;
        const SymbolKind kind;
        SymbolInfoPtr ptr;
        int id;

        Symbol() = delete;

        Symbol(const Token &token, const SymbolType *type, const SymbolKind &kind, SymbolInfoPtr ptr,
               const int id) : token(token), type(type), kind(kind), ptr(std::move(ptr)), id(id) {
        }

        // union SymbolInfoPtr {
        //     int vPtr;
        //     FunInfo *funPtr;
        //     int CI;
        //     float CF;
        //     char CC;
        //     bool CB;
        // } ptr;

        // Symbol(const Token &token, const TempSymbol &tSymbol);
    };

    class SymbolTable : private vector<Symbol> {
        vector<Symbol> &global;
    public:
        using std::vector<Symbol>::emplace_back;
        using std::vector<Symbol>::begin;
        using std::vector<Symbol>::end;
        using std::vector<Symbol>::size;
        using std::vector<Symbol>::operator[];
        void push_back(const Symbol& sym) {
            const Symbol copy = sym;
             std::vector<Symbol>::push_back(copy);
            global.push_back(copy); // 插入一份到 global
        }

        template <typename... Args>
        void emplace_back(Args&&... args) {
            const Symbol sym(std::forward<Args>(args)...);
            std::vector<Symbol>::push_back(sym);
            global.push_back(sym); // 插入一份到 global
        }

        explicit SymbolTable(std::vector<Symbol>& g) : global(g) {}

        [[nodiscard]] iterator findByToken(const Token &tk) {
            auto pos = this->begin();
            while(pos != this->end()){
                if(pos->token == tk) return pos;
                ++pos;
            }
            return pos;
        }

        [[nodiscard]] const_iterator findByToken(const Token &tk) const {
            auto pos = this->begin();
            while(pos != this->end()){
                if(pos->token == tk) return pos;
                ++pos;
            }
            return pos;
        }


    };
} // symbol

#endif //SYMBOL_H
