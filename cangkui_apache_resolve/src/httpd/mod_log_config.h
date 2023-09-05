#ifndef ILOGTAIL_MY_MOD_LOG_H
#define ILOGTAIL_MY_MOD_LOG_H 1

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <cstring>

struct params {
    int pos; // 当前解析日志字符串的位置
    std::string arg; // 额外的参数
    char ed; // 当前段终止符（如果存在）
};

typedef std::function<std::string(std::string &, params &)> func_type;

// 解析得到的每个描述符
typedef struct {
    func_type func; // 每个item对应的处理函数
    char *arg; // 保存参数
    char *t; // format描述符类型
    int condition_sense;
    // int want_orig;
    bool is_constant; // 该处解析是否为常量解析
    std::vector<int> conditions; // 保存状态码
} log_format_item;

/****************************
 * 处理函数（处理format字符串）
 */
std::vector<log_format_item * > parse_log_string(const char *s, const char **err);
void release(std::vector<log_format_item * > &r);

/****************************
 * 处理函数（处理日志内容）
 */
std::string constant_item(std::string &srcstr, params &pr);
std::string pr_host_ipv4(std::string &srcstr, params &pr);
std::string pr_digit(std::string &srcstr, params &pr);
std::string pr_digit_clf(std::string &srcstr, params &pr);
std::string pr_status_code(std::string &srcstr, params &pr);
std::string pr_req(std::string &srcstr, params &pr);
std::string pr_req_protocal(std::string &srcstr, params &pr);
std::string pr_identifier(std::string &srcstr, params &pr);
std::string pr_time(std::string &srcstr, params &pr);
std::string pr_agent(std::string &srcstr, params &pr);
std::string pr_x(std::string &srcstr, params &pr);
std::string pr_info(std::string &srcstr, params &pr);

/****************************
 * 注册函数：构造哈希映射
 */
void processor_register_config(std::unordered_map<std::string, func_type> &m);

#endif // ILOGTAIL_MY_MOD_LOG_H
