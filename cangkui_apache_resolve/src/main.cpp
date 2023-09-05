/*
 * @Author: cangkui 1843361355@qq.com
 * @Date: 2023-07-29 14:34:27
 * @Description:    问题收集：
 *                  1. url不能包含空格、双引号等符号，否则解析异常
 *                  2. %{VARNAME}i 变量只支持Referer和UA
 *                  3. %{VARNAME}e、%{VARNAME}^ti 等还不支持
 */
#include <iostream>
#include "../include/process.h"
#include "./httpd/mod_log_config.h"

int main(int argc, char** argv) {
    std::vector<std::string> input = {
        "10.1.1.95 - e800 [18/Mar/2005:12:21:42 +0800] \"GET /stats/awstats.pl?config=e800 HTTP/1.1\" 200 899 \"http://10.1.1.1/pv/\" \"Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_3 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8J2 Safari/6533.18.5\""
    };
    std::vector<Log> output;
    
    std::string format = "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"";
    Process(format, input, output);
    for (Log l : output) {
        for (Content c : l.contents) {
            std::cout << c.key << ": " << c.value << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}