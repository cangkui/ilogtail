#include "mod_log_config.h"

std::unordered_set<std::string> method{"GET","POST","PUT","DELETE","PATCH","HEAD","TRACE","OPTIONS","CONNECT"};
// 包括 渲染引擎标识与版本号标识
std::unordered_set<std::string> ag_eng{"AppleWebKit","Gecko","Blink","Trident","Presto","EdgeHTML","KHTML","Edg"};
std::unordered_set<std::string> ag_ver{"Version","Mobile","Chrome","Firefox","Safari","Edge",};


std::string constant_item(std::string &srcstr, params &pr)
{
    // params.arg 此处为具体的固定字符串
    int st = pr.pos;
    if (pr.pos + pr.arg.length() <= srcstr.length()) {
        std::string hd = srcstr.substr(st, pr.arg.length());
        if (hd == pr.arg) {
            pr.pos += pr.arg.length();
            return hd;
        }
        else
            return "";
    }
    return "";
}

std::string pr_host_ipv4(std::string &srcstr, params &pr)
{
    // 校验组成形式为a.b.c.d 不校验0<=a,b,c,d<=255
    if (pr.pos >= srcstr.length())
        return "";
    
    int num_cnt = 0, 
        dot_cnt = 0,
        st = pr.pos;
    while (pr.pos < srcstr.length()) {
        char c = srcstr[pr.pos];
        if (isdigit(c)) {
            while (pr.pos < srcstr.length() && isdigit(srcstr[pr.pos])) {
                pr.pos++;
            }
            num_cnt++;
        }
        else if (c == '.') {
            dot_cnt++;
            pr.pos++;
        }
        else break;
    }

    if (num_cnt == 4 && dot_cnt == 3)
        return srcstr.substr(st, pr.pos - st);
    return "";
}

std::string pr_digit(std::string &srcstr, params &pr)
{
    if (pr.pos >= srcstr.length())
        return "";
    
    int st = pr.pos;
    while(pr.pos < srcstr.length() && (isdigit(srcstr[pr.pos]) || srcstr[pr.pos] == '.')) {
        pr.pos++;
    }

    return srcstr.substr(st, pr.pos - st);
}

std::string pr_digit_clf(std::string &srcstr, params &pr)
{
    // CLF 规范的描述符的日志内容可以用'-'代替
    if (pr.pos >= srcstr.length())
        return "";
    
    if (srcstr[pr.pos] == '-') {
        pr.pos++;
        if (pr.pos >= srcstr.length() || srcstr[pr.pos] == pr.ed)
            return "-";
        return "";
    }
    
    int st = pr.pos;
    while(pr.pos < srcstr.length() && (isdigit(srcstr[pr.pos]) || srcstr[pr.pos] == '.')) {
        pr.pos++;
    }

    return srcstr.substr(st, pr.pos - st);
}

std::string pr_status_code(std::string &srcstr, params &pr)
{
    if (pr.pos >= srcstr.length())
        return "";
    
    int st = pr.pos;
    while (pr.pos < srcstr.length() && isdigit(srcstr[pr.pos])) {
        pr.pos++;
    }
    if (pr.pos - st == 3)
        return srcstr.substr(st, 3);
    return "";
}

std::string pr_url(std::string &srcstr, params &pr)
{
    if (pr.pos >= srcstr.length())
        return "";
    
    int st = pr.pos;
    // TODO
    return "";
}

std::string pr_req(std::string &srcstr, params &pr) 
{
    // 三部分：请求方法 请求url 请求协议，三者空格隔开
    if (pr.pos >= srcstr.length())
        return "";

    int st = pr.pos;
    // 请求方法
    while(pr.pos < srcstr.length() && isupper(srcstr[pr.pos])) {
        pr.pos++;
    }
    if (pr.pos >= srcstr.length()) {
        if (!method.count(srcstr.substr(st, pr.pos - st)))
            return "";
        return srcstr.substr(st, pr.pos - st);
    }
    if (srcstr[pr.pos] != ' ')
        return "";
    pr.pos++;

    // 请求地址：不能包含空格
    while(pr.pos < srcstr.length() && srcstr[pr.pos] != ' ')
        pr.pos++;
    if (pr.pos >= srcstr.length()) 
        return srcstr.substr(st, pr.pos - st);
    pr.pos++;
    
    // 请求协议
    while (pr.pos < srcstr.length() && (
                isupper(srcstr[pr.pos]) || 
                isdigit(srcstr[pr.pos]) || 
                srcstr[pr.pos] == '/' || 
                srcstr[pr.pos] == '.'
            )
        )
        pr.pos++;
    return srcstr.substr(st, pr.pos - st);
}

std::string pr_req_protocal(std::string &srcstr, params &pr) 
{
    if (pr.pos >= srcstr.length())
        return "";
    
    int st = pr.pos;
    while (pr.pos < srcstr.length() && (
                isupper(srcstr[pr.pos]) || 
                isdigit(srcstr[pr.pos]) || 
                srcstr[pr.pos] == '/' || 
                srcstr[pr.pos] == '.'
            )
        )
        pr.pos++;
    return srcstr.substr(st, pr.pos - st);
}

std::string pr_identifier(std::string &srcstr, params &pr) 
{
    // 标识符用于识别%u等无基本特征的描述符日志内容，该部分的日志内容不能包含空格
    if (pr.pos >= srcstr.length())
        return "";

    int st = pr.pos;
    while(pr.pos < srcstr.length() && srcstr[pr.pos] != pr.ed) {
        pr.pos++;
    }
    return srcstr.substr(st, pr.pos - st);
}

std::string pr_time(std::string &srcstr, params &pr)
{
    if (pr.pos >= srcstr.length())
        return "";
    
    if (pr.arg.empty()) {
        // 默认情况下时间由'['和']'包围
        if (srcstr[pr.pos] != '[')
            return "";
        int st = pr.pos;
        while (pr.pos < srcstr.length() && srcstr[pr.pos] != ']') {
            pr.pos++;
        }
        if (pr.pos >= srcstr.length())
            return srcstr.substr(st, pr.pos - st);
        else
            pr.pos++;
        return srcstr.substr(st, pr.pos - st);
    }
    // %{format}t 的解析比较麻烦暂未实现
    else return "";
}

bool is_agent(std::string &s, int &pos) {
    // 判断字符串首部子串是否符合渲染引擎标识
    int st = pos;
    if (pos < s.length() && s[pos] != '(' && s[pos] != '[') {
        while (pos < s.length() && s[pos] != '/' && s[pos] != ' ') {
            pos++;
        }
        if (ag_eng.count(s.substr(st, pos - st)) || ag_ver.count(s.substr(st, pos - st))) {
            // sample: AppleWebKit/537.36+
            if (pos >= s.length() || s[pos] == ' ') {
                return true;
            }
            else if (s[pos] == '/') {
                pos++;
                // 识别对应的版本号字符串  sample: 537.36+  or  4J86
                while (pos < s.length() && (isdigit(s[pos]) || isupper(s[pos]) || s[pos] == '.' || s[pos] == '+')) {
                    pos++;
                }
                return true;
            }
            else {
                pos = st;
                return false;
            }
        }
        else {
            pos = st;
            return false;
        }
    }
    else if (pos < s.length() && s[pos] == '(') {
        // sample: (KHTML, like Gecko)
        while (pos < s.length() && s[pos] != ')') pos++;
        if (pos < s.length()) pos++;
        return true;
    }
    // 暂时不考虑某些浏览器UA末尾自带的部分
    // else if (pos < s.length() && s[pos] == '[') {
    //     // sample: [FB_IAB/FB4A;FBAV/349.0.0.35.118;]
    //     while (pos < s.length() && s[pos] != ']') pos++;
    //     if (pos < s.length()) pos++;
    //     return true;
    // }
    else return false;
}

std::string pr_agent(std::string &srcstr, params &pr)
{
    if (pr.pos >= srcstr.length())
        return "";
    
    // 解析%{User-Agent}i的公共部分. 该部分内部以空格隔开，因此内部元素的值不能包含空格
    int st = pr.pos;
    // sample: Mozilla/5.0  or  Opera/4.3
    while (pr.pos < srcstr.length() && srcstr[pr.pos] != ' ') {
        pr.pos++;
    }
    if (pr.pos >= srcstr.length()) return srcstr.substr(st, pr.pos - st);
    if (srcstr[pr.pos] == ' ') pr.pos++; // a space is skipped

    // sample: (Windows NT 10.0; Win64; x64)
    if (pr.pos >= srcstr.length()) return srcstr.substr(st, pr.pos - st);
    if (srcstr[pr.pos] != '(') return "";
    while (pr.pos < srcstr.length() && srcstr[pr.pos] != ')') pr.pos++;
    pr.pos++;
    if (pr.pos >= srcstr.length()) return srcstr.substr(st, pr.pos - st);
    if (srcstr[pr.pos] == ' ') pr.pos++; // a space is skipped
    if (pr.pos >= srcstr.length()) return srcstr.substr(st, pr.pos - st);
    if (srcstr[pr.pos] == ' ') pr.pos++; // a space is skipped

    // 解析渲染引擎与版本号部分
    while (pr.pos < srcstr.length() && is_agent(srcstr, pr.pos)) {
        if (srcstr[pr.pos] == ' ') pr.pos++;
    }
    return srcstr.substr(st, pr.pos - st);
}


std::string pr_x(std::string &srcstr, params &pr)
{
    if (pr.pos >= srcstr.length())
        return "";
    
    int st = pr.pos;
    if (srcstr[pr.pos] != pr.ed) pr.pos++;
    return srcstr.substr(st, pr.pos - st);
}

std::string pr_info(std::string &srcstr, params &pr)
{
    // 目前只支持Referer与UA
    if (pr.pos >= srcstr.length())
        return "";
    
    int st = pr.pos;
    if (pr.arg == "Referer") 
        return pr_identifier(srcstr, pr);
    else if (pr.arg == "User-Agent")
        return pr_agent(srcstr, pr);
    else
        return "";
}
