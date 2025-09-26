#include "parse_var_replace.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static VarEntry g_vars[MAX_VARS];
static int g_warn_undefined_once = 0;

static void trim_ws(char *s) {
  // 去前导
  char *p = s;
  while (*p && isspace((unsigned char)*p))
    p++;
  if (p != s)
    memmove(s, p, strlen(p) + 1);
  // 去后缀
  size_t n = strlen(s);
  while (n > 0 && isspace((unsigned char)s[n - 1]))
    s[--n] = '\0';
}

void var_init(void) {
  memset(g_vars, 0, sizeof(g_vars));
  g_warn_undefined_once = 0;
}

static int find_var_index(const char *name) {
  for (int i = 0; i < MAX_VARS; i++) {
    if (g_vars[i].used && strcmp(g_vars[i].name, name) == 0)
      return i;
  }
  return -1;
}

int var_set(const char *name, const char *value) {
  if (!name)
    return -1;
  char nbuf[MAX_WORD_NUMBERS];
  strncpy(nbuf, name, sizeof(nbuf) - 1);
  nbuf[sizeof(nbuf) - 1] = '\0';
  trim_ws(nbuf);
  if (nbuf[0] == '\0')
    return -1;

  int idx = find_var_index(nbuf);
  if (idx >= 0) {
    strncpy(g_vars[idx].value, value ? value : "",
            sizeof(g_vars[idx].value) - 1);
    g_vars[idx].value[sizeof(g_vars[idx].value) - 1] = '\0';
    return 0;
  }
  for (int i = 0; i < MAX_VARS; i++) {
    if (!g_vars[i].used) {
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

int var_get(const char *name, char *out, size_t out_size) {
  if (!out || out_size == 0)
    return 0;
  out[0] = '\0';
  if (!name || !*name)
    return 0;

  char nbuf[MAX_WORD_NUMBERS];
  strncpy(nbuf, name, sizeof(nbuf) - 1);
  nbuf[sizeof(nbuf) - 1] = '\0';
  trim_ws(nbuf);

  int idx = find_var_index(nbuf);
  const char *val = NULL;
  if (idx >= 0) {
    val = g_vars[idx].value;
  } else {
    // 退回环境变量
    val = getenv(nbuf);
    if (!val)
      val = "";
  }
  strncpy(out, val, out_size - 1);
  out[out_size - 1] = '\0';
  return (int)strlen(out);
}

static void safe_append(char *dst, size_t dst_size, const char *src) {
  if (!dst || !src || dst_size == 0)
    return;
  size_t cur = strlen(dst);
  size_t cap = (cur < dst_size - 1) ? (dst_size - 1 - cur) : 0;
  if (cap == 0)
    return;
  strncat(dst, src, cap);
}

static void safe_append_ch(char *dst, size_t dst_size, char ch) {
  char buf[2] = {ch, '\0'};
  safe_append(dst, dst_size, buf);
}

static int expand_recursive(const char *src, char *dst, size_t dst_size,
                            int depth) {
  if (!src || !dst || dst_size == 0)
    return -1;
  dst[0] = '\0';
  if (depth <= 0) {
    // 超过最大递归深度，直接原样输出
    safe_append(dst, dst_size, src);
    return 0;
  }

  for (size_t i = 0; src[i];) {
    if (src[i] == '$') {
      if (src[i + 1] == '$') {
        // $$ -> 输出单个 $
        safe_append_ch(dst, dst_size, '$');
        i += 2;
        continue;
      } else if (src[i + 1] == '(' || src[i + 1] == '{') {
        char close = (src[i + 1] == '(') ? ')' : '}';
        size_t j = i + 2;
        while (src[j] && src[j] != close)
          j++;
        if (src[j] == close) {
          // 提取变量名
          char name[MAX_WORD_NUMBERS];
          size_t nlen = j - (i + 2);
          if (nlen >= sizeof(name))
            nlen = sizeof(name) - 1;
          memcpy(name, src + i + 2, nlen);
          name[nlen] = '\0';
          trim_ws(name);

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

void var_expand_into(const char *src, char *dst, size_t dst_size) {
  // 设定最大递归深度，避免自引用死循环
  // 例如 A=$(B) B=$(A) 的场景
  expand_recursive(src ? src : "", dst, dst_size, 16);

  // 对未定义变量的提示（仅一次）
  // 实际检测已在 var_get 里静默为 ""，这里不额外扫描，以简化实现
  (void)g_warn_undefined_once;
}

void var_export_to_env(void) {
  for (int i = 0; i < MAX_VARS; i++) {
    if (!g_vars[i].used)
      continue;
    // setenv 覆盖导出
    setenv(g_vars[i].name, g_vars[i].value, 1);
  }
}