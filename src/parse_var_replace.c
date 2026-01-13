#include "parse_var_replace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
/*
 * 变量替换模块
 *
 * 提供：
 * - var_init           初始化变量表
 * - var_set            设置/覆盖变量
 * - var_get            读取变量（若不存在，回退到环境变量，否则为空串）
 * - var_expand_into    将包含 $(NAME)/${NAME} 的字符串递归展开
 * - var_export_to_env  将内部变量导出到进程环境
 *
 * 注意：
 * - 所有字符串写入都做了边界保护，发生截断时保持以'\0'结尾。
 */

/*
 * g_vars: 进程内的变量表
 * - 存放若干 (name, value) 对，线性查找，便于实现覆盖语义。
 */
static VarEntry g_vars[MAX_VARS];

// 作用：清空变量表，重置内部状态。
void var_init(void) { memset(g_vars, 0, sizeof(g_vars)); }

// 作用：在线性表中查找变量名对应的下标
static int find_var_index(const char *name)
{
    for (int i = 0; i < MAX_VARS; i++)
    {
        if (g_vars[i].used && strcmp(g_vars[i].name, name) == 0) return i;
    }
    return -1;
}

/*
 * var_set
 * 作用：设置或覆盖一个变量。
 * 参数：
 *   - name : 变量名，允许含空白，函数内部会 trim；不可为 NULL。
 *   - value: 变量值，可为 NULL（等价于空串）。
 * 返回：
 *   - 0 : 成功（新增或覆盖）；
 *   - -1: 失败（name 无效或表已满）。
 * 说明：
 *   - 后设置的同名变量会覆盖之前的值；
 *   - 写入时做长度截断并保证以 '\0' 结尾。
 */
int var_set(const char *name, const char *value)
{
    if (!name || !*name) return -1;

    // 将name的值临时存储到nbuf
    char nbuf[MAX_WORD_NUMBERS];
    strncpy(nbuf, name, sizeof(nbuf) - 1);
    nbuf[sizeof(nbuf) - 1] = '\0';
    trim(nbuf);

    if (nbuf[0] == '\0') return -1;

    // 如果name已存入线性表，覆盖value的值
    int idx = find_var_index(nbuf);
    if (idx >= 0)
    {
        strncpy(g_vars[idx].value, value ? value : "", sizeof(g_vars[idx].value) - 1);
        g_vars[idx].value[sizeof(g_vars[idx].value) - 1] = '\0';
        return 0;
    }

    // name未存入过线性表，在线性表中找空位置存入并存储对应value
    for (int i = 0; i < MAX_VARS; i++)
    {
        if (!g_vars[i].used)
        {
            g_vars[i].used = 1;
            strncpy(g_vars[i].name, nbuf, sizeof(g_vars[i].name) - 1);
            g_vars[i].name[sizeof(g_vars[i].name) - 1] = '\0';
            strncpy(g_vars[i].value, value ? value : "", sizeof(g_vars[i].value) - 1);
            g_vars[i].value[sizeof(g_vars[i].value) - 1] = '\0';
            return 0;
        }
    }

    fprintf(stderr, "Variable table is full, cannot set '%s'\n", nbuf);
    return -1;
}

/*
 * var_get
 * 作用：读取变量值到调用者提供的缓冲区。
 * 参数：
 *   - name    : 变量名（可含空白，内部会 trim）。
 *   - out     : 输出缓冲区指针，必须非 NULL。
 *   - out_size: 输出缓冲区大小，必须 > 0。
 * 返回：
 *   - 实际写入到 out 的字符数（不含终止符）。
 * 行为：
 *   - 若内部表找不到对应项，回退查询进程环境变量；
 *   - 若仍不存在，写入空串；
 *   - 始终保证 out 以 '\0' 结尾；可能发生截断。
 */
int var_get(const char *name, char *out, size_t out_size)
{
    if (!out || out_size == 0) return 0;

    out[0] = '\0';

    if (!name || !*name) return 0;

    // 临时存储name
    char nbuf[MAX_WORD_NUMBERS];
    strncpy(nbuf, name, sizeof(nbuf) - 1);
    nbuf[sizeof(nbuf) - 1] = '\0';
    trim(nbuf);

    int idx         = find_var_index(nbuf);
    const char *val = NULL;
    if (idx >= 0)
    {
        val = g_vars[idx].value;
    }
    else
    {
        // 退回环境变量
        val = getenv(nbuf);
        if (!val) val = "";
    }

    strncpy(out, val, out_size - 1);
    out[out_size - 1] = '\0';
    return (int)strlen(out);
}

/*
 * safe_append（内部函数）
 * 作用：安全地将 src 追加到 dst 尾部，自动处理容量与终止符。
 * 参数：
 *   - dst, dst_size: 目标缓冲区及其大小；
 *   - src          : 要追加的字符串；
 * 返回：无
 * 说明：
 *   - 若容量不足，仅追加可容纳的前缀；
 *   - 若任一指针无效或 dst_size==0 则不操作。
 */
static void safe_append(char *dst, size_t dst_size, const char *src)
{
    if (!dst || !src || dst_size == 0) return;
    size_t cur = strlen(dst);
    size_t cap = (cur < dst_size - 1) ? (dst_size - 1 - cur) : 0;
    if (cap == 0) return;
    strncat(dst, src, cap);
}

/*
 * safe_append_ch（内部函数）
 * 作用：向目标缓冲区安全地追加单个字符。
 */
static void safe_append_ch(char *dst, size_t dst_size, char ch)
{
    char buf[2] = {ch, '\0'};
    safe_append(dst, dst_size, buf);
}

/*
 * expand_recursive（内部函数）
 * 作用：对 src 字符串进行变量展开并写入 dst，支持递归嵌套展开。
 * 参数：
 *   - src     : 输入字符串（可为 NULL，等价于空串）。
 *   - dst     : 结果输出缓冲区。
 *   - dst_size: 输出缓冲区大小。
 *   - depth   : 递归剩余深度（防止循环引用造成无限递归）。
 * 返回：
 *   - 0 : 成功；
 *   - -1: 参数错误。
 * 支持的语法：
 *   - $(NAME) 与 ${NAME}
 * 特殊规则：
 *   - $$ 将被展开为单个字符 '$'；
 *   - 未匹配到闭合括号时，按普通字符原样输出；
 *   - 如果name过长则截取至最大长度
 *   - 若 depth <= 0，则不再展开，直接输出 src 原文。
 */
static int expand_recursive(const char *src, char *dst, size_t dst_size, int depth)
{
    if (!src || !dst || dst_size == 0) return -1;

    dst[0] = '\0';

    if (depth <= 0)
    {
        // 超过最大递归深度，直接原样输出
        safe_append(dst, dst_size, src);
        return 0;
    }

    for (size_t i = 0; src[i];)
    {
        // 提取${}/$()/中的name
        if (src[i] == '$')
        {
            if (src[i + 1] == '$')
            {
                // $$ -> 输出单个 $
                safe_append_ch(dst, dst_size, '$');
                i += 2;
                continue;
            }
            else if (src[i + 1] == '(' || src[i + 1] == '{')
            {
                char close = (src[i + 1] == '(') ? ')' : '}';
                size_t j   = i + 2;
                while (src[j] && src[j] != close) j++;
                if (src[j] == close)
                {
                    // 提取变量名
                    char name[MAX_WORD_NUMBERS];
                    size_t nlen = j - (i + 2);
                    if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
                    memcpy(name, src + i + 2, nlen);
                    name[nlen] = '\0';
                    trim(name);

                    // 获取变量值
                    char val[MAX_LINE_LENGTH];
                    var_get(name, val, sizeof(val));

                    // 递归展开变量值
                    char exp_val[MAX_LINE_LENGTH];
                    expand_recursive(val, exp_val, sizeof(exp_val), depth - 1);
                    safe_append(dst, dst_size, exp_val);

                    i = j + 1;
                    continue;
                }
                // 未找到闭合符，视为普通字符处理
            }
        }
        // 普通字符或无法识别的模式
        safe_append_ch(dst, dst_size, src[i]);
        i++;
    }
    return 0;
}

/*
 * var_expand_into
 * 作用：对输入字符串进行变量展开（包含嵌套），并输出到目标缓冲区。
 * 参数：
 *   - src     : 输入字符串（若为 NULL 视为空）。
 *   - dst     : 输出缓冲区。
 *   - dst_size: 输出缓冲区大小。
 * 行为：
 *   - 设定最大递归深度为 16 层；
 *   - 采用 var_get 获取变量值，未定义变量替换为空串；
 *   - 始终保证 dst 以 '\0' 结尾。
 */
void var_expand_into(const char *src, char *dst, size_t dst_size)
{
    // 设定最大递归深度，避免自引用死循环
    // 例如 A=$(B) B=$(A) 的场景
    expand_recursive(src ? src : "", dst, dst_size, 16);
}

/*
 * var_export_to_env
 * 作用：将内部变量表逐项导出到进程环境（setenv，覆盖同名项）。
 * 使用场景：
 *   - 便于后续执行外部命令时，shell 仍可以使用 $NAME 的形式引用。
 */
void var_export_to_env(void)
{
    for (int i = 0; i < MAX_VARS; i++)
    {
        if (!g_vars[i].used) continue;
        // setenv 覆盖导出
        setenv(g_vars[i].name, g_vars[i].value, 1);
    }
}

// 打印当前内部变量表（按存储顺序），便于调试
void var_print_all(void)
{
    for (int i = 0; i < MAX_VARS; i++)
    {
        if (!g_vars[i].used) continue;
        printf("%s=%s\n", g_vars[i].name, g_vars[i].value);
    }
}