//
// Created by admin on 2025/6/3.
//

#ifndef LEXER_H
#define LEXER_H

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
namespace lexer{
    using std::vector;
    using std::string;
    const vector<string> K = {
          "Int"     /*1 */
        , "Float"   /*2 */
        , "Char"    /*3 */
        , "Bool"    /*4 */
        , "Void"    /*5 */
        , "fun"     /*6 */
        , "var"     /*7 */
        , "val"     /*8 */
        , "if"      /*9 */
        , "else"    /*10*/
        , "while"   /*11*/
        , "return"  /*12*/
    };
    const vector<string> P = {
          "("   /*1 */
        , ")"   /*2 */
        , "{"   /*3 */
        , "}"   /*4 */
        , "["   /*5 */
        , "]"   /*6 */
        , "="   /*7 */
        , "||"  /*8 */
        , "&&"  /*9 */
        , "!"   /*10*/
        , "="   /*11*/
        , "=="  /*12*/
        , "!="  /*13*/
        , ">"   /*14*/
        , ">="  /*15*/
        , "<"   /*16*/
        , "<="  /*17*/
        , "+"   /*18*/
        , "-"   /*19*/
        , "*"   /*20*/
        , "/"   /*21*/
        , "'"   /*22*/
        , "\\"  /*23*/
        , ","   /*24*/
        , ";"   /*25*/
        , ":"   /*26*/
    };
    enum class TokenType {
        K, P, I, CI, CF, CC, CB, T
    };

    inline std::ostream& operator<< (std::ostream& os, const TokenType& tt) {
        if(tt == TokenType::K) return os << "K";
        if(tt == TokenType::P) return os << "P";
        if(tt == TokenType::I) return os << "I";
        if(tt == TokenType::CI) return os << "CI";
        if(tt == TokenType::CF) return os << "CF";
        if(tt == TokenType::CC) return os << "CC";
        if(tt == TokenType::CB) return os << "CB";
        if(tt == TokenType::T) return os << "T";
        throw std::runtime_error("Invalid TokenType value");
    }
    struct Token {
        TokenType type;
        int id;
        Token() = default;
        Token(const TokenType& type, const int& id) : type(type), id(id){}
        explicit Token(const string& str) {
            if(const auto pos = std::ranges::find(P, str); pos != P.end()){
                type = TokenType::P;
                id = static_cast<int>(pos - P.begin()) + 1;
            }
            else if(const auto pos = std::ranges::find(K, str); pos != K.end()){
                type = TokenType::K;
                id = static_cast<int>(pos - K.begin()) + 1;
            }
            else throw std::runtime_error("Wrong on construct Token by string. Not found this string in K");
        }
        bool operator==(const Token& other) const {
            return type == other.type && id == other.id;
        }
        friend std::ostream& operator<< (std::ostream& os, const Token& token) {
            return os << "(" << token.type << ", " << token.id << ")";
        }
        [[nodiscard]] string toString() const {
            std::stringstream ss;
            ss << "(" << type << ", " << id << ")";
            return ss.str();
        }
    };

    inline bool scan(const string& str, vector<Token>& ans
                     , vector<string>& I, vector<int>& ci, vector<float>& cf, vector<string>& err) {

        int state = 0;
        int line = 1, column = 1;
        int num;
        string now;
        auto addInt = [&]() {
            if(const auto pos = std::ranges::find(ci, num); pos == ci.end()){
                ci.push_back(num);
                ans.emplace_back(TokenType::CI, static_cast<int>(ci.size()));
            }
            else{
                ans.emplace_back(TokenType::CI, static_cast<int>(pos - ci.begin() + 1));
            }
        };
        auto addFloat = [&]() {
            float num;
            std::stringstream ss;
            ss << now;
            ss >> num;
            auto pos = std::find(cf.begin(), cf.end(), num);
            if(pos == cf.end()){
                cf.push_back(num);
                ans.emplace_back(TokenType::CF, static_cast<int>(cf.size()));
            }
            else{
                ans.emplace_back(TokenType::CF, static_cast<int>(pos - cf.begin() + 1));
            }
        };
        auto addErr = [&](const int type, const string& arg = "") {std::stringstream ss;
            ss << "Wrong on ";
            ss << line;
            ss << " line ";
            ss << column;
            ss << " column.";
            if(type == 0){ //非法符号
                ss << " Unexpected char ";
                ss << arg;
                ss << ".";
            }
            else if(type == 1){ //非法结束符
                if(arg.empty()) ss << " Cannot end program with identifier or number.";
                else{
                    ss << " Cannot end program with ";
                    ss << arg;
                    ss << ".";
                }
            }
            else if(type == 2){ //非法标识符
                ss << " Illegal identifiers starting with numbers.";
            }
            else if(type == 3){ //与期望不符
                ss << " Expected \'";
                ss << arg;
                ss << "\' before this.";
            }
            else if(type == 4){ // 非法小数
                ss << " Illegal decimals.";
            }
            string output;
            std::getline(ss, output);
            err.push_back(output);
        };
        // auto addP = [&]() {
        //     auto pos = std::find(P.begin(), P.end(), now);
        //     if(pos == P.end()){
        //         addErr(0, now);
        //         return false;
        //     }
        //     ans.emplace_back(TokenType::P, int(pos - P.begin() + 1));
        // };

// 这里报错需要让整个函数返回而不是仅仅让 addP 返回，所以没办法了用来宏的写法。
#define addP() do { \
    auto pos = std::find(P.begin(), P.end(), now); \
    if (pos == P.end()) { \
        addErr(0, now); \
        return false; \
    } \
    ans.emplace_back(TokenType::P, int(pos - P.begin() + 1)); \
} while (0)

        auto addChar = [&]() {
            assert(now.size() == 1);
            ans.emplace_back(TokenType::CC, static_cast<int>(now[0]));
        };
        auto ptr = str.begin();
        while(true){
            if(state == 0){
                now.clear();
                num = 0;
                if(ptr == str.end()) return true;
                const char c = *ptr;
                ++ptr;
                if(isspace(c)){

                }
                else if(isalpha(c)){
                    now.clear();
                    now.push_back(c);
                    state = 1;
                }
                else if(isdigit(c)){
                    now.clear();
                    now.push_back(c);
                    num = c - '0';
                    state = (c == '0') ? 2 : 5;
                }
                else if(c == '='){
                    now.push_back('=');
                    state = 11;
                }
                else if(c == '>'){
                    now.push_back('>');
                    state = 12;
                }
                else if(c == '<'){
                    now.push_back('<');
                    state = 13;
                }
                else if(c == '|'){
                    now.push_back('|');
                    state = 14;
                }
                else if(c == '&'){
                    now.push_back('&');
                    state = 15;
                }
                else if(c == '!'){
                    now.push_back('!');
                    state = 16;
                }
                else if(c == '\''){
                    now.push_back('\'');
                    state = 17;
                }
                else{
                    now.push_back(c);
                    addP();
                }
                if(c == '\n'){
                    ++line;
                    column = 1;
                }
                else{
                    ++column;
                }
            }
            else if(state == 1){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isalpha(*ptr) || isdigit(*ptr)){
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else{
                    state = 0;
                    auto pos = std::find(K.begin(), K.end(), now);
                    if(pos != K.end()){
                        ans.emplace_back(TokenType::K, static_cast<int>(pos - K.begin() + 1));
                    }
                    else{
                        pos = std::find(I.begin(), I.end(), now);
                        if(pos == I.end()){
                            I.push_back(now);
                            ans.emplace_back(TokenType::I, static_cast<int>(I.size()));
                        }
                        else{
                            ans.emplace_back(TokenType::I, static_cast<int>(pos - I.begin() + 1));
                        }
                    }
                }
            }
            else if(state == 2){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(*ptr == 'x'){
                    state = 3;
                    ++ptr;
                    ++column;
                }
                else if(*ptr == 'e'){
                    state = 8;
                    now.push_back('e');
                    ++ptr;
                    ++column;
                }
                else if(*ptr == '.'){
                    state = 6;
                    now.push_back('.');
                    ++ptr;
                    ++column;
                }
                else if(isdigit(*ptr)){
                    state = 5;
                    now.push_back(*ptr);
                    num = num * 10 + (*ptr - '0');
                    ++ptr;
                    ++column;
                }
                else if(isalpha(*ptr)){
                    addErr(2);
                    return false;
                }
                else{
                    state = 0;
                    addInt();
                }
            }
            else if(state == 3){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr) || ('a' <= *ptr && *ptr <= 'f')){
                    state = 4;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else{
                    err.emplace_back("Illegal hexadecimal numbers.");
                    return false;
                }
            }
            else if(state == 4){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr) || ('a' <= *ptr && *ptr <= 'f')){
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else if(isalpha(*ptr)){
                    err.emplace_back("Illegal hexadecimal numbers.");
                    return false;
                }
                else {
                    state = 0;
                    addInt();
                }
            }
            else if(state == 5){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(*ptr == 'e'){
                    state = 8;
                    now.push_back('e');
                    ++ptr;
                    ++column;
                }
                else if(*ptr == '.'){
                    state = 6;
                    now.push_back('.');
                    ++ptr;
                    ++column;
                }
                else if(isdigit(*ptr)){
                    state = 5;
                    now.push_back(*ptr);
                    num = num * 10 + (*ptr - '0');
                    ++ptr;
                    ++column;
                }
                else if(isalpha(*ptr)){
                    addErr(2);
                    return false;
                }
                else{
                    state = 0;
                    addInt();
                }
            }
            else if(state == 6){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr)){
                    state = 7;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else{
                    addErr(4);
                    return false;
                }
            }
            else if(state == 7){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr)){
                    state = 7;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else if(*ptr == 'e'){
                    state = 8;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else if(isalpha(*ptr)){
                    addErr(4);
                    return false;
                }
                else{
                    state = 0;
                    addFloat();
                }
            }
            else if(state == 8){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr)){
                    state = 10;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else if(*ptr == '+' || *ptr == '-'){
                    state = 9;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else{
                    addErr(4);
                    return false;
                }
            }
            else if(state == 9){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr)){
                    state = 10;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else{
                    addErr(4);
                    return false;
                }
            }
            else if(state == 10){
                if(ptr == str.end()){
                    addErr(1);
                    return false;
                }
                else if(isdigit(*ptr)){
                    state = 10;
                    now.push_back(*ptr);
                    ++ptr;
                    ++column;
                }
                else if(isalpha(*ptr)){
                    addErr(4);
                    return false;
                }
                else{
                    state = 0;
                    addFloat();
                }
            }
            else if(state == 11 || state == 12 || state == 13 || state == 16){
                if(ptr == str.end()){
                    addErr(1, now);
                    return false;
                }
                else if(*ptr == '='){
                    state = 0;
                    now.push_back('=');
                    addP();
                    ++ptr;
                    ++column;
                }
                else{
                    state = 0;
                    addP();
                }
            }
            else if(state == 14){
                if(ptr == str.end()){
                    addErr(1, now);
                    return false;
                }
                else if(*ptr == '|'){
                    state = 0;
                    now.push_back('|');
                    addP();
                    ++ptr;
                    ++column;
                }
                else{
                    addErr(0, now);
                }
            }
            else if(state == 15){
                if(ptr == str.end()){
                    addErr(1, now);
                    return false;
                }
                else if(*ptr == '&'){
                    state = 0;
                    now.push_back('&');
                    addP();
                    ++ptr;
                    ++column;
                }
                else{
                    addErr(0, now);
                }
            }
            else if(state == 17){
                if(ptr == str.end()){
                    addErr(1, now);
                    return false;
                }
                else if(*ptr == '\\'){
                    state = 18;
                    now.push_back('\\');
                    ++ptr;
                    ++column;
                }
                else{
                    state = 19;
                    now.clear();
                    now.push_back(*ptr);
                    addChar();
                    ++ptr;
                    ++column;
                }
            }
            else if(state == 18){
                if(ptr == str.end()){
                    addErr(1, now);
                    return false;
                }
                else if(*ptr == 'n'){
                    state = 19;
                    now.clear();
                    now.push_back('\n');
                    addChar();
                    ++ptr;
                    ++column;
                }
                else if(*ptr == '\\'){
                    state = 19;
                    now.clear();
                    now.push_back('\\');
                    addChar();
                    ++ptr;
                    ++column;
                }
                else{
                    now = "\\";
                    now.push_back(*ptr);
                    addErr(0, now);
                    return false;
                }
            }
            else if(state == 19){
                if(ptr == str.end()){
                    addErr(1, now);
                    return false;
                }
                else if(*ptr == '\''){
                    state = 0;
                    ++ptr;
                    ++column;
                }
                else{
                    addErr(3, "\'");
                    return false;
                }
            }
        }
    }
}

#endif //LEXER_H
