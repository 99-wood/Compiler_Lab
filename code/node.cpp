#include "node.h"

#include <ranges>

namespace node{
    int Node::tim = 0;
    vector<Node *> Node::allNode;
    std::map<std::pair<std::pair<Node *, Node *>, string>, Node *> Node::to;
    std::map<Var, Node *> Node::belong;

    bool isConstVar(const Var &var) {
        return var.first == VarType::C;
    }

    void Node::add(const Var &var) {
        if(sv.contains(var)) return;
        ++tim;
        sv[var] = tim;
        s.insert(V(var, tim));
        belong[var] = this;
    }

    void Node::del(const Var &var) {
        assert(var.first == VarType::I);
        assert(sv.contains(var));
        s.erase(V(var, sv[var]));
        sv.erase(var);
        belong.erase(var);
        assert(!(s.empty() && !isBase));
    }

    vector<Var> Node::getC() const {
        if(!isConst) return {};
        assert(!s.empty() && s.begin()->first.first == VarType::C);
        return {s.begin()->first};
    }

    vector<Var> Node::getI() const {
        auto it = s.lower_bound(V(Var(VarType::I, 0), 0));
        vector<Var> ans;
        while(it != s.end() && it->first.first == VarType::I){
            ans.push_back(it->first);
            ++it;
        }
        return ans;
    }

    vector<Var> Node::getT() const {
        auto it = s.lower_bound(V(Var(VarType::T, 0), 0));
        vector<Var> ans;
        while(it != s.end() && it->first.first == VarType::T){
            ans.push_back(it->first);
            ++it;
        }
        return ans;
    }

    void Node::clear() {
        tim = 0;
        to.clear();
        belong.clear();
        for(const auto ndptr : allNode){
            delete ndptr;
        }
        allNode.clear();
    }

    int Node::getCnt() {
        return static_cast<int>(allNode.size());
    }

    Var Node::getFirst() const {
        assert(!(s.empty() && !isBase));
        if(isBase) return base;
        else return sv.begin()->first;
    }

    // bool Node::contains(const Var &v) {
    //     return sv.contains(v);
    // }

    Node &Node::getNode(const int id) {
        assert(id < allNode.size());
        return *allNode[id];
    }

    Node &Node::getNode(const Node &x, const string &op) {
        if(x.isConst){
            const auto arr = x.getC();
            assert(arr.size() == 1);
            const auto [type, val] = arr[0];
            return getNode(Var(VarType::C, genUnaryOperation(op)(val)));
        }
        else{
            if(const auto token = std::pair(std::pair(const_cast<Node *>(&x), static_cast<Node *>(nullptr)), op); to.
                contains(token)){
                return *to[token];
            }
            else{
                const auto ndptr = new Node(false, vector{const_cast<Node *>(&x)}, op);
                to[token] = ndptr;
                return *ndptr;
            }
        }
    }

    Node &Node::getNode(const Node &x, const string &op, const Node &y) {
        if(x.isConst && y.isConst){
            const auto arrx = x.getC();
            const auto arry = y.getC();
            assert(arrx.size() == 1);
            assert(arry.size() == 1);
            const auto [typex, valx] = arrx[0];
            const auto [typey, valy] = arry[0];
            return getNode(Var(VarType::C, genBinaryOperation(op)(valx, valy)));
        }
        else{
            if(const auto token = std::pair(std::pair(const_cast<Node *>(&x), const_cast<Node *>(&y)), op); to.
                contains(token)){
                return *to[token];
            }
            else{
                const auto ndptr = new Node(false, vector{const_cast<Node *>(&x), const_cast<Node *>(&y)}, op);
                to[token] = ndptr;
                return *ndptr;
            }
        }
    }

    Node &Node::getNode(const Var &v) {
        if(belong.contains(v)){
            return *belong[v];
        }
        if(isConstVar(v)){
            auto ndptr = new Node(true, v);
            return *ndptr;
        }
        else{
            auto ndptr = new Node(true, v);
            return *ndptr;
        }
    }

    void Optimizer::split() {
        vector<bool> tag(origin.size(), false);
        tag[0] = true;
        for(int i = 0; i < origin.size(); ++i){
            if(origin[i].op == "JMP"){
                const int to = quad::Quad::getInt(origin[i].arg1);
                tag[to] = true;
                if(i + 1 < origin.size()) tag[i + 1] = true;
            }
            if(origin[i].op == "JZ"){
                const int to = quad::Quad::getInt(origin[i].arg2);
                tag[to] = true;
                if(i + 1 < origin.size()) tag[i + 1] = true;
            }
            if(origin[i].op == "RET"){
                if(i + 1 < origin.size()) tag[i + 1] = true;
            }
            if(origin[i].op == "CALL"){
                const int to = quad::Quad::getInt(origin[i].arg1);
                tag[to] = true;
            }
        }
        for(int i = 0; i < origin.size(); ++i){
            if(tag[i]) basicBlocks.emplace_back();
            basicBlocks.back().push_back(origin[i]);
            belong[i] = static_cast<int>(basicBlocks.size()) - 1;
        }
    }

    void Optimizer::optimize(const vector<quad::Quad> &quads, vector<quad::Quad> &res) {
        assert(!quads.empty());
        Node::clear();
        bool canOptimaize = true;
        Var v(VarType::I, -1); // 最后的返回值，相当于自定义量
        for(const auto &[op, arg1, arg2, result] : quads){
            if(op == "CALL") canOptimaize = false;
            if(op == "JZ" || op == "RET" && !quad::Quad::isEmpty(arg1)){
                assert(v.second == -1);
                v = genVar(arg1);
            }
        }
        if(!canOptimaize){
            res = quads;
            return;
        }
        for(const auto &[op, arg1, arg2, result] : quads){
            assert(op != "CALL");
            if(op == "JZ" || op == "RET") continue;
            if(op == "JMP"){
            }
            else if(op == "MOV"){
                const Var src = genVar(arg1), dest = genVar(arg2);
                if(Node::belong.contains(dest)){
                    Node::getNode(dest).del(dest);
                }
                Node::getNode(src).add(dest);
            }
            else if(op == "RET"){
            }
            else if(op == "CALL"){
                assert(0);
            }
            else if(op == "JZ"){
            }
            else if(op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" ||
                    op == "ADDF" || op == "SUBF" || op == "MULF" || op == "DIVF" ||
                    op == "G" || op == "GE" || op == "L" || op == "LE" || op == "E" || op == "NE" ||
                    op == "GF" || op == "GEF" || op == "LF" || op == "LEF" || op == "EF" || op == "NEF" ||
                    op == "AND" || op == "OR"){
                const Var x = genVar(arg1), y = genVar(arg2), dest = genVar(result);
                if(Node::belong.contains(dest)){
                    Node::getNode(dest).del(dest);
                }
                Node::getNode(Node::getNode(x), op, Node::getNode(y)).add(dest);
            }
            else if(op == "NOT" ||
                    op == "I2I" || op == "I2F" || op == "I2C" || op == "I2B" ||
                    op == "F2I" || op == "F2F" || op == "F2C" || op == "F2B" ||
                    op == "C2I" || op == "C2F" || op == "C2C" || op == "C2B" ||
                    op == "B2I" || op == "B2F" || op == "B2C" || op == "B2B"){
                const Var src = genVar(arg1), dest = genVar(arg2);
                if(Node::belong.contains(dest)){
                    Node::getNode(dest).del(dest);
                }
                Node::getNode(Node::getNode(src), op).add(dest);
            }
            else if(op == "ERR"){
            }
            else
                assert(0);
        }
        vector<bool> tag(Node::getCnt());
        for(const Node *const nptr : Node::allNode | std::views::reverse){
            if(nptr->sv.contains(v) || !nptr->getI().empty() || tag[nptr->id]){
                tag[nptr->id] = true;
                for(const Node *const prevptr : nptr->prev){
                    tag[prevptr->id] = true;
                }
            }
        }
        for(const Node *const nptr : Node::allNode){
            if(!tag[nptr->id]) continue;
            auto I = nptr->getI();
            if(nptr->sv.contains(v) && std::ranges::find(I, v) == I.end()) I.push_back(v);
            if(I.empty()){
                if(nptr->isBase) continue;
                I.push_back(nptr->getFirst());
            }
            if(nptr->isBase){
                assert(nptr->prev.empty());
                if(*I.begin() == nptr->base) I.erase(I.begin());
                for(const auto &var : I){
                    res.emplace_back("MOV", toString(nptr->base), toString(var));
                }
            }
            else{
                const Var I0 = I[0];
                const auto ptr0 = I.begin();
                I.erase(ptr0);
                assert(!nptr->prev.empty());
                const string op = nptr->op;
                if(nptr->prev.size() == 1){
                    const Var x = nptr->prev[0]->getFirst();
                    res.emplace_back(op, toString(x), toString(I0));
                }
                else{
                    assert(nptr->prev.size() == 2);
                    const Var x = nptr->prev[0]->getFirst();
                    const Var y = nptr->prev[1]->getFirst();
                    res.emplace_back(op, toString(x), toString(y), toString(I0));
                }
                for(const Var &var : I){
                    res.emplace_back("MOV", toString(I0), toString(var));
                }
            }
        }
        if(quads.back().op == "JZ" || quads.back().op == "JMP" || quads.back().op == "RET" || quads.back().op == "ERR"){
            res.push_back(quads.back());
            if(res.back().op == "RET" && !quad::Quad::isEmpty(res.back().arg1)){
                if(Node::belong.contains(genVar(res.back().arg1))){
                    res.back().arg1 = toString(Node::belong[genVar(res.back().arg1)]->getFirst());
                }
            }
        }
    }
    // using std::cout;
    // using std::endl;
    void Optimizer::run() {
        split();
        optimizedQuad.resize(basicBlocks.size());
        start.resize(basicBlocks.size());
        for(int i = 0; i < basicBlocks.size(); ++i){
            optimize(basicBlocks[i], optimizedQuad[i]);
            start[i] = static_cast<int>(res.size());
            res.insert(res.end(), optimizedQuad[i].begin(), optimizedQuad[i].end());
        }
        for(auto& [op, arg1, arg2, result] : res){
            if(op == "JMP" || op == "CALL"){
                const int to = quad::Quad::getInt(arg1);
                arg1 = std::to_string(start[belong[to]]);
            }
            if(op == "JZ"){
                const int to = quad::Quad::getInt(arg2);
                arg2 = std::to_string(start[belong[to]]);
            }
        }
    }

    vector<quad::Quad> Optimizer::getRes() const {
        return res;
    }

    std::function<int(int, int)> Node::genBinaryOperation(const string &op) {
        if(op == "ADD"){
            return [&](const int &x, const int &y) { return x + y; };
        }
        if(op == "SUB"){
            return [&](const int &x, const int &y) { return x - y; };
        }
        if(op == "MUL"){
            return [&](const int &x, const int &y) { return x * y; };
        }
        if(op == "DIV"){
            return [&](const int &x, const int &y) {
                assert(y != 0);
                return x / y;
            };
        }

        if(op == "AND"){
            return [&](const int &x, const int &y) { return static_cast<bool>(x) && static_cast<bool>(y); };
        }
        if(op == "OR"){
            return [&](const int &x, const int &y) { return static_cast<bool>(x) || static_cast<bool>(y); };
        }

        if(op == "ADDF"){
            return [&](const int &x, const int &y) {
                return std::bit_cast<int>(std::bit_cast<float>(x) + std::bit_cast<float>(y));
            };
        }
        if(op == "SUBF"){
            return [&](const int &x, const int &y) {
                return std::bit_cast<int>(std::bit_cast<float>(x) - std::bit_cast<float>(y));
            };
        }
        if(op == "MULF"){
            return [&](const int &x, const int &y) {
                return std::bit_cast<int>(std::bit_cast<float>(x) * std::bit_cast<float>(y));
            };
        }
        if(op == "DIVF"){
            return [&](const int &x, const int &y) {
                return std::bit_cast<int>(std::bit_cast<float>(x) / std::bit_cast<float>(y));
            };
        }

        if(op == "G"){
            return [&](const int &x, const int &y) { return x > y; };
        }
        if(op == "GE"){
            return [&](const int &x, const int &y) { return x >= y; };
        }
        if(op == "L"){
            return [&](const int &x, const int &y) { return x < y; };
        }
        if(op == "LE"){
            return [&](const int &x, const int &y) { return x <= y; };
        }
        if(op == "E"){
            return [&](const int &x, const int &y) { return x == y; };
        }
        if(op == "NE"){
            return [&](const int &x, const int &y) { return x != y; };
        }

        if(op == "GF"){
            return [&](const int &x, const int &y) { return std::bit_cast<float>(x) > std::bit_cast<float>(y); };
        }
        if(op == "GEF"){
            return [&](const int &x, const int &y) { return std::bit_cast<float>(x) >= std::bit_cast<float>(y); };
        }
        if(op == "LF"){
            return [&](const int &x, const int &y) { return std::bit_cast<float>(x) < std::bit_cast<float>(y); };
        }
        if(op == "LEF"){
            return [&](const int &x, const int &y) { return std::bit_cast<float>(x) <= std::bit_cast<float>(y); };
        }
        if(op == "EF"){
            return [&](const int &x, const int &y) { return std::bit_cast<float>(x) == std::bit_cast<float>(y); };
        }
        if(op == "NEF"){
            return [&](const int &x, const int &y) { return std::bit_cast<float>(x) != std::bit_cast<float>(y); };
        }
        assert(0);
    }

    std::function<int(int)> Node::genUnaryOperation(const string &op) {
        if(op == "NOT"){
            return [&](const int &x) { return std::bit_cast<int>(x); };
        }
        if(op == "I2F"){
            return [&](const int &x) { return std::bit_cast<int>(static_cast<float>(x)); };
        }
        if(op == "C2F"){
            return [&](const int &x) { return std::bit_cast<int>(static_cast<float>(x)); };
        }
        if(op == "B2F"){
            return [&](const int &x) { return std::bit_cast<int>(static_cast<float>(x)); };
        }
        if(op == "F2I"){
            return [&](const int &x) { return static_cast<int>(std::bit_cast<float>(x)); };
        }
        if(op == "C2I"){
            return [&](const int &x) { return static_cast<int>(x); };
        }
        if(op == "B2I"){
            return [&](const int &x) { return static_cast<int>(x); };
        }
        if(op == "I2C"){
            return [&](const int &x) { return static_cast<char>(x); };
        }
        if(op == "F2C"){
            return [&](const int &x) { return static_cast<char>(std::bit_cast<float>(x)); };
        }
        if(op == "B2C"){
            return [&](const int &x) { return static_cast<char>(x); };
        }
        if(op == "I2B"){
            return [&](const int &x) { return static_cast<bool>(x); };
        }
        if(op == "F2B"){
            return [&](const int &x) { return static_cast<bool>(std::bit_cast<float>(x)); };
        }
        if(op == "C2B"){
            return [&](const int &x) { return static_cast<bool>(x); };
        }
        assert(0);
    }
}
