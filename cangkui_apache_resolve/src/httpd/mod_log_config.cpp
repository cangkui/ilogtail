#include "mod_log_config.h"

// 描述符->处理函数 的哈希映射
std::unordered_map<std::string, func_type> log_hash;

/*****************************************************************
 * utils: To get a word from the source string starting from 
 * index 0 up to the position of a stop character.
 */
char *getword(const char **line, char stop)
{
    const char *pos = *line;
    int len;
    char *res;

    while ((*pos != stop) && *pos) {
        ++pos;
    }

    len = pos - *line;
    // res = apr_pstrmemdup(atrans, *line, len);
    if (*line == NULL) res = NULL;
    else {
        res = new char[len+1];
        memcpy(res, *line, len);
        res[len] = '\0';
    }

    if (stop) {
        while (*pos == stop) {
            ++pos;
        }
    }
    *line = pos;

    return res;
}

func_type hash_func_get(std::unordered_map<std::string, func_type> &hash_map, const char *key, int klen)
{
    std::string hkey(key, key + klen);
    return hash_map[hkey];
}


/*****************************************************************
 * Parsing the log format string
 */

char *parse_log_misc_string(log_format_item *it, const char **sa)
{
    const char *s;
    char *d;

    it->func = constant_item;
    // it->conditions = NULL;
    it->is_constant = true;

    s = *sa;
    while (*s && *s != '%') {
        s++;
    }
    /*
     * This might allocate a few chars extra if there's a backslash
     * escape in the format string.
     */
    // it->arg = apr_palloc(p, s - *sa + 1);
    it->arg = new char[s - *sa + 1];

    d = it->arg;
    s = *sa;
    while (*s && *s != '%') {
        if (*s != '\\') {
            *d++ = *s++;
        }
        else {
            s++;
            switch (*s) {
            case '\\':
                *d++ = '\\';
                s++;
                break;
            case 'r':
                *d++ = '\r';
                s++;
                break;
            case 'n':
                *d++ = '\n';
                s++;
                break;
            case 't':
                *d++ = '\t';
                s++;
                break;
            default:
                /* copy verbatim */
                *d++ = '\\';
                /*
                 * Allow the loop to deal with this *s in the normal
                 * fashion so that it handles end of string etc.
                 * properly.
                 */
                break;
            }
        }
    }
    *d = '\0';

    *sa = s;
    return NULL;
}

char *parse_log_item(log_format_item *it, const char **sa)
{
    const char *s = *sa;
    it->t = NULL;
    // ap_log_handler *handler = NULL;
    func_type handler = NULL;

    if (*s != '%') {
        // 如果fmt本次解析的是常规字符而非规范的描述符
        return parse_log_misc_string(it, sa);
    }

    // 是描述符的话，看下一位（描述符均以%开头）
    ++s;
    it->condition_sense = 0;
    // it->conditions = NULL;

    if (*s == '%') {
        // 解析描述符： %%
        it->arg = new char[2];
        it->arg[0] = '%', it->arg[1] = '\0';
        it->func = constant_item;
        it->is_constant = true;
        *sa = ++s;
        return NULL;
    }

    // it->want_orig = -1;
    // it->arg = "";               /* For safety's sake... */
    // it->arg = new char[1];
    // it->arg[0] = '\0';
    it->arg = NULL;

    while (*s) {
        int i;

        switch (*s) {
        case '!':
            ++s;
            // it->condition_sense = !it->condition_sense;
            break;

        case '<':
            ++s;
            // it->want_orig = 1;
            break;

        case '>':
            ++s;
            // it->want_orig = 0;
            break;

        case ',':
            ++s;
            break;

        case '{':
            ++s;
            it->arg = getword(&s, '}');
            break;

        // 状态码
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            i = *s - '0';
            while (isdigit(*++s)) {
                i = i * 10 + (*s) - '0';
            }
            // if (!it->conditions) {
            //     it->conditions = apr_array_make(p, 4, sizeof(int));
            // }
            // *(int *) apr_array_push(it->conditions) = i;
            it->conditions.push_back(i);
            break;

        default:
            /* check for '^' + two character format first */
            if (*s == '^' && *(s+1) && *(s+2)) {
                // handler = (ap_log_handler *)apr_hash_get(log_hash, s, 3);
                // handler = constant_item; 
                handler = hash_func_get(log_hash, s, 3);
                if (handler) {
                    it->t = new char[4];
                    it->t[0] = '%'; it->t[1] = *(s+1); it->t[2] = *(s+2);
                    it->t[3] = '\0';
                    s += 3;
                }
            }
            if (!handler) {  
                // handler = (ap_log_handler *)apr_hash_get(log_hash, s++, 1);  
                handler = hash_func_get(log_hash, s, 1);
                it->t = new char[3];
                it->t[0] = '%'; it->t[1] = *(s);
                it->t[2] = '\0';
                s++;
            }
            if (!handler) {
                // char dummy[2];

                // dummy[0] = s[-1];
                // dummy[1] = '\0';
                // return apr_pstrcat(p, "Unrecognized LogFormat directive %",
                //                dummy, NULL);
                char *msg = new char[36];
                char *m = (char *)"Unrecognized LogFormat directive % ";
                memcpy(msg, m, 35);
                msg[34] = s[-1];
                return msg;
            }
            it->func = handler;
            // if (it->want_orig == -1) {
            //     it->want_orig = handler->want_orig_default;
            // }
            *sa = s;
            return NULL;
        }
    }

    char *msg = new char[56];
    char *r = (char *)"Ran off end of LogFormat parsing args to some directive";
    memcpy(msg, r, 56);
    return msg;
}

std::vector<log_format_item * > parse_log_string(const char *s, const char **err)
{   
    processor_register_config(log_hash);
    // s 是用于解析的format字符串
    std::vector<log_format_item *> a;
    char *res;
    
    while (*s) {
        log_format_item *it = new log_format_item;
        it->is_constant = false;
        if ((res = parse_log_item(it, &s))) {
            *err = res;
            return a;
        }
        // std::cout << it->arg << std::endl;
        a.push_back(it);
    }

    return a;
}

void release(std::vector<log_format_item * > &r) 
{
    for (log_format_item *it : r) {
        if (it->arg != NULL) {
            delete[] it->arg;
            it->arg = NULL;
        }
        if (it->t != NULL) {
            delete[] it->t;
            it->t = NULL;
        }
        delete it;
        it = NULL;
    }
}

void processor_register_config(std::unordered_map<std::string, func_type> &m)
{
    // TODO: register processors of each format describe char.
    m["a"] = pr_host_ipv4;
    m["A"] = pr_host_ipv4;
    m["B"] = pr_digit;
    m["b"] = pr_digit_clf;
    // m["C"]
    m["D"] = pr_digit;
    // m["e"]
    m["h"] = pr_host_ipv4;
    m["H"] = pr_req_protocal;
    m["i"] = pr_info;
    m["k"] = pr_digit;
    m["l"] = pr_identifier;
    m["L"] = pr_digit_clf;
    m["M"] = pr_identifier;
    // m["o"]
    // m["n"]
    m["p"] = pr_digit;
    m["P"] = pr_digit;
    m["q"] = pr_identifier; // 前提是url当中的空格等字符被再次编码，如%20，不包括双引号、空格等空白字符
    m["r"] = pr_req;
    m["R"] = pr_req;
    m["s"] = pr_status_code;
    m["t"] = pr_time;
    m["T"] = pr_digit;
    m["u"] = pr_identifier;
    m["U"] = pr_identifier;
    m["v"] = pr_identifier;
    m["V"] = pr_identifier;
    m["X"] = pr_x;
    m["I"] = pr_digit;
    m["O"] = pr_digit;
    m["S"] = pr_digit;
    // m["^ti"]
    // m["^to"]
}