//
// Created by admin on 2025/6/11.
//

#ifndef TARGET_H
#define TARGET_H

namespace target{
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cassert>
    using std::string;
    using std::vector;

    class Combines {
    public:
        explicit Combines(const quad::Quad &quad) {
            std::stringstream ss;
            ss << quad;
            alter(ss.str());
            return;
        }

        int alter(string s);

        //Combines();
        string OpSymble;
        string Opnum1;
        string Opnum2;
        string Opresult;
    };

    struct Address {
        void assignAdr(const Address &add) {
            DS.assign(add.DS);
            ES.assign(add.ES);
            BX.assign(add.BX);
            offset.assign(add.offset);
            size = add.size;
        }

        string DS;
        string ES;
        string BX;
        string offset;
        int size;
    };


    Address parseAddressExpression(const string &s);

    void STACK_ALLOC(vector<uint8_t> &datas, int num);

    void STACK_FREE(vector<uint8_t> &datas, int num);

    void MOV(vector<Combines>::iterator &cp, vector<uint8_t> &datas, int32_t &DS, int32_t &ES, int32_t &BX);

    bool isaddress(const string &adrexp);

    // uint32_t getsomething(const string &adrexp, vector<uint8_t> &datas, int32_t &DS, int32_t &ES, int32_t &BX);

    //从datas取src
    // uint32_t getsomething(const string &adrexp, int32_t &DS, int32_t &ES, int32_t &BX); //从其他方式取src
    // int retpos(Address &add, int32_t &DS, int32_t &ES, int32_t &BX);

    // void pushsomething(const string &adrexp, vector<uint8_t> &datas, int32_t &DS, int32_t &ES, int32_t &BX,
                       // uint32_t src); //置入dst
    // void pushsomething(const string &adrexp, int32_t &DS, int32_t &ES, int32_t &BX, uint32_t src); //置入dst,dst不为内存
    std::function<uint32_t(uint32_t, uint32_t)> funselect(const string &op); //二元运算包装

    bool runcode(vector<Combines>::iterator &cp, vector<uint8_t> &datas, int32_t &DS, int32_t &ES, int32_t &BX,
                 vector<Combines>::iterator &head); //运行代码
    string classification(const string &opsymble); //将四元式操作大致分类
    std::function<int(int)> genUnaryOperation(const string &op); //一元运算


    class TargetRuner {
        vector<uint8_t> datas; // 数据段
        vector<Combines> codes; // 代码段
        int32_t DS = 0;
        int32_t ES = 0;
        int32_t BX = 0;
        uint32_t getsomething(const string &adrexp, vector<uint8_t> &dwatas, const int32_t &DS, const int32_t &ES, int32_t &BX) const {
            Address tem;
            tem.assignAdr(parseAddressExpression(adrexp));

            int pos = retpos(tem, DS, ES, BX);

            uint32_t result = 0;
            for(int i = tem.size - 1; i >= 0; i--){
                result *= 256;
                result += datas[pos + i];
            }

            return result;
        }

        int retpos(const Address &add, const int32_t &DS, const int32_t &ES, const int32_t &BX) const {
            int tem = 0;
            if(!add.BX.empty())tem += BX;
            if(!add.DS.empty())tem += DS;
            if(!add.ES.empty())tem += ES;
            if(!add.offset.empty())tem += atoi(add.offset.c_str());
            //cerr<<tem<<' '<<datas.size();
            assert(tem<datas.size());
            return tem;
        }

        void pushsomething(const string &adrexp, vector<uint8_t> &datas, const int32_t &DS, const int32_t &ES, const int32_t &BX,
                           uint32_t src) const {
            Address tem;
            tem.assignAdr(parseAddressExpression(adrexp));

            int pos = retpos(tem, DS, ES, BX);

            for(int i = 0; i <= tem.size - 1; i++){
                datas[pos + i] = src % 256;

                src /= 256;
            }
        }

        static uint32_t getsomething(const string &adrexp, const int32_t &DS, const int32_t &ES, const int32_t &BX) {
            uint32_t tem = 0;
            if(adrexp == "DS")tem = DS;
            else if(adrexp == "ES")tem = ES;
            else if(adrexp == "BX")tem = BX;
            else tem = atoi(adrexp.c_str());

            return tem;
        }

        static void pushsomething(const string &adrexp, int32_t &DS, int32_t &ES, int32_t &BX, const uint32_t src) {
            if(adrexp == "DS")DS = src;
            if(adrexp == "ES")ES = src;
            if(adrexp == "BX")BX = src;
        }


        static void STACK_ALLOC(vector<uint8_t> &datas, const int num) {
            for(int i = 0; i < num; i++){
                datas.push_back(0);
            }
            //cerr<<'*'<<num;
        }

        static void STACK_FREE(vector<uint8_t> &datas, const int num) {
            assert(num<=datas.size());
            for(int i = 0; i < num; i++){
                datas.pop_back();
            }
        }

        void MOV(vector<Combines>::const_iterator &cp, vector<uint8_t> &datas, int32_t &DS, int32_t &ES, int32_t &BX) {
            int src = 0;
            if(isaddress(cp->Opnum1)){
                src = getsomething(cp->Opnum1, datas, DS, ES, BX);
            }
            else{
                src = getsomething(cp->Opnum1, DS, ES, BX);
            }

            if(isaddress(cp->Opnum2)){
                pushsomething(cp->Opnum2, datas, DS, ES, BX, src);
            }
            else{
                pushsomething(cp->Opnum2, DS, ES, BX, src);
            }
        }
        bool runcode(vector<Combines>::const_iterator &cp, vector<uint8_t> &datas, int32_t &DS, int32_t &ES, int32_t &BX,
                 const vector<Combines>::const_iterator &head) {
        string opclass = classification(cp->OpSymble);
        if(opclass == "TWO"){
            //二元
            uint32_t a, b;
            //取src
            if(isaddress(cp->Opnum1)){
                a = getsomething(cp->Opnum1, datas, DS, ES, BX);
            }
            else{
                a = getsomething(cp->Opnum1, DS, ES, BX);
            }
            if(isaddress(cp->Opnum2)){
                b = getsomething(cp->Opnum2, datas, DS, ES, BX);
            }
            else{
                b = getsomething(cp->Opnum2, DS, ES, BX);
            }
            //调用计算
            auto opfun = funselect(cp->OpSymble);

            int src = opfun(a, b);

            //dst保存
            if(isaddress(cp->Opresult)){
                pushsomething(cp->Opresult, datas, DS, ES, BX, src);
            }
            else{
                pushsomething(cp->Opresult, DS, ES, BX, src);
            }
            ++cp;
            return true;
        }
        if(opclass == "TRANS"){
            //一元
            uint32_t a;
            if(isaddress(cp->Opnum1)){
                a = getsomething(cp->Opnum1, datas, DS, ES, BX);
            }
            else{
                a = getsomething(cp->Opnum1, DS, ES, BX);
            }

            auto opfun = genUnaryOperation(cp->OpSymble);
            const int src = opfun(a);

            //dst保存
            if(isaddress(cp->Opnum2)){
                pushsomething(cp->Opnum2, datas, DS, ES, BX, src);
            }
            else{
                pushsomething(cp->Opnum2, DS, ES, BX, src);
            }
            ++cp;
            return true;
        }
        if(opclass == "OTHER"){
            if(cp->OpSymble == "MOV"){
                MOV(cp, datas, DS, ES, BX);
                ++cp;
                return true;
            }
            if(cp->OpSymble == "STACK_ALLOC"){
                uint32_t a;
                if(isaddress(cp->Opnum1)){
                    a = getsomething(cp->Opnum1, datas, DS, ES, BX);
                }
                else{
                    a = getsomething(cp->Opnum1, DS, ES, BX);
                }
                //cerr<<'&'<<a;
                ES = datas.size();
                STACK_ALLOC(datas, a);
                ++cp;
                return true;
            }
            if(cp->OpSymble == "STACK_FREE"){
                uint32_t a;
                if(isaddress(cp->Opnum1)){
                    a = getsomething(cp->Opnum1, datas, DS, ES, BX);
                }
                else{
                    a = getsomething(cp->Opnum1, DS, ES, BX);
                }
                STACK_FREE(datas, a);
                ++cp;
                return true;
            }

            if(cp->OpSymble == "JMP"){
                //跳转
                uint32_t a;
                if(isaddress(cp->Opnum1)){
                    a = getsomething(cp->Opnum1, datas, DS, ES, BX);
                }
                else{
                    a = getsomething(cp->Opnum1, DS, ES, BX);
                }
                cp = head + a;
                return true;
            }
            if(cp->OpSymble == "JZ"){
                uint32_t a, b;
                if(isaddress(cp->Opnum1)){
                    a = getsomething(cp->Opnum1, datas, DS, ES, BX);
                }
                else{
                    a = getsomething(cp->Opnum1, DS, ES, BX);
                }
                if(isaddress(cp->Opnum2)){
                    b = getsomething(cp->Opnum2, datas, DS, ES, BX);
                }
                else{
                    b = getsomething(cp->Opnum2, DS, ES, BX);
                }

                if(!a){
                    cp = head + b;
                    return true;
                }
                else{
                    cp++;
                    return true;
                }
            }
            //报错
            if(cp->OpSymble == "ERR"){
                std::cout << "ERROR ON" << cp - head << std::endl;
                ++cp;
                return false;
            }

            assert(0);
        }
        assert(0);
    }

    public:
        explicit TargetRuner(const vector<quad::Quad> &input) {
            for(const auto &quad : input){
                Combines tmp(quad);
                codes.push_back(tmp);
            }
        }

        int getFirst() const {
            assert(datas.size() >= 4);
            int unitres = 0;
            for(int i = 3; i >= 0; i--){
                unitres *= 256;
                unitres += datas[i];
            }
            return unitres;
        }

        int run() {
            vector<Combines>::const_iterator CP = codes.begin();
            const vector<Combines>::const_iterator head = codes.begin();


            while(CP != codes.end()){
                if(runcode(CP, datas, DS, ES, BX, head)){
                }
                else{
                    return 1;
                }
            }
            return 0;
        }
    };

    inline Address parseAddressExpression(const string &s) {
        Address addr;
        addr.DS = "";
        addr.ES = "";
        addr.BX = "";
        addr.offset = "";
        addr.size = 0;

        // 1. 去除所有空格并检查格式
        string input = s;

        if(input.back() != ']')input.push_back(']');

        input.erase(remove_if(input.begin(), input.end(), ::isspace), input.end());

        if(input.empty() || input.front() != '[' || input.back() != ']')
            return addr; // 格式错误

        // 2. 提取括号内的内容
        string content = input.substr(1, input.size() - 2);

        // 3. 按最后一个冒号分割表达式和大小
        const size_t colon_pos = content.find_last_of(':');
        if(colon_pos == string::npos)
            return addr; // 格式错误

        string expr = content.substr(0, colon_pos);
        const string size_str = content.substr(colon_pos + 1);

        // 4. 处理大小
        try{
            addr.size = stoi(size_str);
        } catch(...){
            addr.size = 0; // 转换失败
        }

        // 5. 分割地址表达式
        vector<string> tokens;
        if(!expr.empty()){
            std::istringstream iss(expr);
            string token;
            while(getline(iss, token, '+')){
                if(!token.empty())
                    tokens.push_back(token);
            }
        }

        // 6. 分类处理各元素
        vector<string> offsetParts;

        for(const auto &t : tokens){
            if(t == "DS"){
                if(addr.DS.empty()) addr.DS = "DS";
            }
            else if(t == "ES"){
                if(addr.ES.empty()) addr.ES = "ES";
            }
            else if(t == "BX"){
                if(addr.BX.empty()) addr.BX = "BX";
            }
            else{
                offsetParts.push_back(t);
            }
        }

        // 7. 构建偏移量字符串
        for(size_t i = 0; i < offsetParts.size(); i++){
            if(i > 0) addr.offset += "+";
            addr.offset += offsetParts[i];
        }

        return addr;
    }

    inline string trim(const string &str) {
        size_t start = str.find_first_not_of(" ");
        if(start == string::npos) return "";
        size_t end = str.find_last_not_of(" ");
        return str.substr(start, end - start + 1);
    }

    inline int Combines::alter(string s) {
        // 1. 去除所有空格
        string input = s;
        input.erase(std::ranges::remove_if(input, [](unsigned char c) {
            return isspace(c);
        }).begin(), input.end());

        // 2. 查找四元式主体部分的起始位置
        size_t start_pos = input.find('(');
        if(start_pos == string::npos) return -1; // 找不到左括号

        // 3. 提取括号内的内容
        size_t end_pos = input.find(')', start_pos);
        if(end_pos == string::npos) return -1; // 找不到右括号
        string content = input.substr(start_pos + 1, end_pos - start_pos - 1);

        // 4. 分割逗号分隔的各个部分
        vector<string> tokens;
        std::istringstream iss(content);
        string token;
        while(getline(iss, token, ',')){
            if(!token.empty()){
                tokens.push_back(token);
            }
        }


        // 5. 验证令牌数量 (至少需要3个：操作符+两个操作数)
        if(tokens.size() < 3) return -1;

        // 6. 填充结构体成员
        this->OpSymble = tokens[0];
        this->Opnum1 = tokens[1];
        this->Opnum2 = tokens[2];

        // 7. 处理结果部分 (可能存在)
        if(tokens.size() >= 4){
            this->Opresult = tokens[3];
        }
        else{
            this->Opresult = ""; // 无结果部分
        }

        return 0; // 成功解析
    }

    inline bool isaddress(const string &adrexp) {
        if(adrexp[0] == '[' || adrexp[1] == '[') return true;
        return false;
    }


    inline std::function<uint32_t(uint32_t, uint32_t)> funselect(const string &op) {
        if(op == "ADD"){
            return [&](const uint32_t &x, const uint32_t &y) { return *(int *) (&x) + *(int *) (&y); };
        }
        if(op == "SUB"){
            return [&](const uint32_t &x, const uint32_t &y) { return x - y; };
        }
        if(op == "MUL"){
            return [&](const uint32_t &x, const uint32_t &y) { return x * y; };
        }
        if(op == "DIV"){
            return [&](const uint32_t &x, const uint32_t &y) { return x / y; };
        }
        if(op == "ADDF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y), c = a + b;
                int res = *(int *) (&c);
                return res;
            };
        }
        if(op == "SUBF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y), c = a - b;
                int res = *(int *) (&c);
                return res;
            };
        }
        if(op == "MULF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y), c = a * b;

                int res = *(int *) (&c);
                return res;
            };
        }
        if(op == "DIVF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y), c = a / b;
                int res = *(int *) (&c);
                return res;
            };
        }
        if(op == "G"){
            return [&](const uint32_t &x, const uint32_t &y) { return (int) x > (int) y ? 1 : 0; };
        }
        if(op == "GE"){
            return [&](const uint32_t &x, const uint32_t &y) { return (int) x >= (int) y ? 1 : 0; };
        }
        if(op == "L"){
            return [&](const uint32_t &x, const uint32_t &y) { return (int) x < (int) y ? 1 : 0; };
        }
        if(op == "LE"){
            return [&](const uint32_t &x, const uint32_t &y) { return (int) x <= (int) y ? 1 : 0; };
        }
        if(op == "E"){
            return [&](const uint32_t &x, const uint32_t &y) { return (int) x == (int) y ? 1 : 0; };
        }
        if(op == "NE"){
            return [&](const uint32_t &x, const uint32_t &y) { return (int) x != (int) y ? 1 : 0; };
        }
        if(op == "GF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y);
                int res = a > b ? 1 : 0;
                return res;
            };
        }
        if(op == "GEF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y);
                int res = a >= b ? 1 : 0;
                return res;
            };
        }
        if(op == "LF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y);
                int res = a < b ? 1 : 0;
                return res;
            };
        }
        if(op == "LEF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y);
                int res = a <= b ? 1 : 0;
                return res;
            };
        }
        if(op == "EF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y);
                int res = a == b ? 1 : 0;
                return res;
            };
        }
        if(op == "NEF"){
            return [&](const uint32_t &x, const uint32_t &y) {
                float a = *(float *) (&x), b = *(float *) (&y);
                int res = a != b ? 1 : 0;
                return res;
            };
        }
        if(op == "AND"){
            return [&](const uint32_t &x, const uint32_t &y) { return x && y; };
        }
        if(op == "OR"){
            return [&](const uint32_t &x, const uint32_t &y) { return x || y; };
        }
        return [&](const uint32_t &x, const uint32_t &y) { return 0; };
    }


    inline string classification(const string &opsymble) {
        string two = "ADD SUB MUL DIV ADDF SUBF MULF DIVF G GE L LE E NE GF GEF LF LEF EF NEF AND OR";
        if(two.find(opsymble) != string::npos)return "TWO";
        if(opsymble.find("2") != string::npos || opsymble == "NOT")return "TRANS";
        return "OTHER";
    }

    inline std::function<int(int)> genUnaryOperation(const string &op) {
        if(op == "NOT"){
            return [&](const int &x) { return x == 0; };
        }
        if(op == "I2F"){
            return [&](const int &x) {
                float tem = (float) x;

                return *(int *) (void *) (&tem);
            };
        }
        if(op == "C2F"){
            return [&](const int &x) {
                float tem = (float) x;
                return *(int *) (void *) (&tem);
            };
        }
        if(op == "B2F"){
            return [&](const int &x) {
                float tem = (float) x;
                return *(int *) (void *) (&tem);
            };
        }
        if(op == "I2C"){
            return [&](const int &x) { return static_cast<char>(x); };
        }
        if(op == "F2C"){
            return [&](const int &x) {
                float tem = *(float *) (&x);
                return static_cast<char>(tem);
            };
        }
        if(op == "B2C"){
            return [&](const int &x) { return x; };
        }
        if(op == "I2B"){
            return [&](const int &x) { return static_cast<bool>(x); };
        }
        if(op == "F2B"){
            return [&](const int &x) {
                float tem = *(float *) (&x);
                return static_cast<bool>(tem);
            };
        }
        if(op == "C2B"){
            return [&](const int &x) { return static_cast<bool>(x); };
        }
        if(op == "F2I"){
            return [&](const int &x) {
                float tem = *(float *) (&x);
                return static_cast<int>(tem);
            };
        }
        if(op == "B2I"){
            return [&](const int &x) { return x; };
        }
        if(op == "C2I"){
            return [&](const int &x) { return x; };
        }
        assert(0);
    }
}

#endif //TARGET_H
