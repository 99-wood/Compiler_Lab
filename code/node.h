//
// Created by admin on 2025/6/11.
//

#ifndef NODE_H
#define NODE_H
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cassert>
#include <iostream>
#include <map>
#include <functional>

#include "quad.h"

namespace node{
    using std::vector;
    using std::string;

    enum class VarType {
        C, I, T
    };

    inline std::ostream &operator<<(std::ostream &os, const VarType &vt) {
        if(vt == VarType::C) return os << 'C';
        if(vt == VarType::I) return os << 'I';
        if(vt == VarType::T) return os << 'T';
        assert(0);
    }

    using Var = std::pair<VarType, int>;

    inline string toString(const Var &var) {
        if(var.first == VarType::C) return std::to_string(var.second);
        std::stringstream ss;
        ss << "(" << var.first << ", " << var.second << ")";
        return ss.str();
    }

    inline Var genVar(const string &str) {
        assert(!quad::Quad::isEmpty(str));
        if(quad::Quad::isToken(str)){
            Var v;
            const auto [type, id] = quad::Quad::getToken(str);
            v.second = id;
            if(type == lexer::TokenType::I) v.first = VarType::I;
            else{
                assert(type == lexer::TokenType::T);
                v.first = VarType::T;
            }
            return v;
        }
        else{
            return {VarType::C, quad::Quad::getInt(str)};
        }
    }


    class Node {
        friend class Optimizer;
        using V = std::pair<Var, int>;

        class Cmparator {
        public:
            bool operator()(const V &x, const V &y) const {
                assert(!(x.first.first == VarType::C && y.first.first == VarType::C));
                return x.first.first == y.first.first ? x.second < y.second : x.first.first < y.first.first;
            }
        };

        static int tim;
        static std::map<Var, Node *> belong;
        static vector<Node *> allNode;
        static std::map<std::pair<std::pair<Node *, Node *>, string>, Node *> to;
        std::set<V, Cmparator> s;
        std::map<Var, int> sv;

        explicit Node(const bool is, const vector<Node*> &pre, string op) : id(static_cast<int>(allNode.size())), isBase(is), isConst(false),
                                       base(Var(VarType::T, -1)), prev(pre), op(std::move(op)){
            assert(!is);
            allNode.push_back(this);
        }

        explicit Node(const bool is, const Var &v) : id(static_cast<int>(allNode.size())), isBase(is), isConst(v.first == VarType::C),
                                                     base(v), prev(){
            assert(is);
            allNode.push_back(this);
            add(v);
        }

        static std::function<int(int, int)> genBinaryOperation(const string &op);

        static std::function<int(int)> genUnaryOperation(const string &op);

    public:
        Node() = delete;

        int id;
        const bool isBase;
        const bool isConst;
        const Var base;
        const vector<Node*> prev;
        const string op;

        void add(const Var &var);

        void del(const Var &var);

        [[nodiscard]] vector<Var> getC() const; // 获取改节点所有常数，最多一个
        [[nodiscard]] vector<Var> getI() const; // 获取改节点所有变量
        [[nodiscard]] vector<Var> getT() const; // 获取改节点所有临时变量
        static void clear();

        static int getCnt(); // 获取节点个数
        [[nodiscard]] Var getFirst() const;
        // bool contains(const Var &v);
        static Node &getNode(int id); // 通过 id 获取对应节点 id 从 0 开始
        static Node &getNode(const Node &x, const string &op); // 一元运算生成 node
        static Node &getNode(const Node &x, const string &op, const Node &y); // 二元运算生成 node
        static Node &getNode(const Var &v); // 变量生成 node
        // static bool contain(const Var &v);
    };

    class Optimizer {
        const vector<quad::Quad> origin;
        vector<quad::Quad> res;
        vector<vector<quad::Quad> > basicBlocks;
        vector<vector<quad::Quad> > optimizedQuad;
        vector<int> start;
        vector<int> belong;

        void split();

        static void optimize(const vector<quad::Quad> &quads, vector<quad::Quad> &res);

    public:
        explicit Optimizer(const vector<quad::Quad> &code) : origin(code), res(), belong(code.size(), 0) {
        }

        void run();
        [[nodiscard]] vector<quad::Quad> getRes() const;
    };
}
#endif //NODE_H
