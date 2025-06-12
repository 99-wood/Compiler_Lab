//
// Created by admin on 2025/6/10.
//

#ifndef QUAD_H
#define QUAD_H
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include "lexer.h"

namespace quad{
    using std::string;
    using std::vector;
    using lexer::Token;

    class Quad {
    public:
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


        static bool isToken(const string &arg);

        static bool isEmpty(const string &arg);

        static Token getToken(const string &arg);

        static int getInt(const string &arg);

    };


    class QuadRunner {
        class cmp {
        public:
            bool operator()(const Token &lhs, const Token &rhs) const {
                return lhs.type == rhs.type ? lhs.id < rhs.id : lhs.type < rhs.type;
            }
        };

        struct MemDevice {
            std::map<Token, int, cmp> &global;
            std::map<Token, int, cmp> &mem;
            explicit MemDevice(std::map<Token, int, cmp> &global, std::map<Token, int, cmp> &mem) : global(global), mem(mem) {
            }

            int &operator[](const Token &token) {
                if(global.contains(token)){
                    return global[token];
                }
                else{
                    return mem[token];
                }
            }
        };

        const vector<Quad> args;
        std::map<Token, int, cmp> global;
        size_t currentArg{};
        bool finish{false};
        std::pair<bool, int> execute(MemDevice &mem);

        static std::function<int(int, int)> genBinaryOperation(const string &op);

        static std::function<int(int)> genUnaryOperation(const string &op);

    public:
        explicit QuadRunner(const vector<Quad> &args) : args(args), global() {
        }

        void run();

        [[nodiscard]] std::map<Token, int, cmp> getRes() const;
    };
} // quad

#endif //QUAD_H
