#include "../include/process.h"
#include "./httpd/mod_log_config.h"

void Process(std::string format, std::vector<std::string>& input, std::vector<Log>& output) {
    const char *err = NULL;
    std::vector<log_format_item * > res = parse_log_string(format.c_str(), &err);
    if (err != NULL) {
        std::cout << "error: " << err << std::endl;
        delete[] err;
        err = NULL;
        release(res);
        return;
    }
    
    // std::cout<< res.size() << std::endl;

    for (std::string inp : input) {
        params pr = {0, "", ' '};
        Log inp_l;
        for (int i=0; i<res.size(); i++) {
            log_format_item *it = res[i];
            if (i+1 < res.size() && res[i+1]->is_constant && res[i+1]->arg != NULL) {
                // 下一个为常量则记录终止符
                pr.ed = *(res[i+1]->arg);
            }
            else {
                pr.ed = ' ';
            }

            if (it->arg != NULL) {
                pr.arg = std::string(it->arg);
            }
            else {
                pr.arg = "";
            }

            std::string res_log = it->func(inp, pr);

            if (res_log != "") {
                std::cout << "res log: " << res_log << ";" << std::endl;
                if (it->t != NULL) {
                    Content c;
                    if (it->arg != NULL) {
                        c = {std::string(it->t)+","+std::string(it->arg), res_log};
                    }
                    else {
                        c = {std::string(it->t), res_log};
                    }
                    inp_l.contents.push_back(c);
                }
            }
            else {
                std::cout << "Error: Log content does not match the format string." << std::endl;
                break;
            }
        }
        output.push_back(inp_l);
    }

    release(res);
}