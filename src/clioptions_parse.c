#include "clioptions_parse.h"
#include "parse.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 用法提示
void print_usage(const char *prog) {
  printf("Usage: %s [options] [target ...]\n", prog);
  printf("Options:\n");
  printf("  -h, --help             Show this help and exit\n");
  printf("  -v, --verbose_mode     Enable verbose mode\n");
  printf("  -j N, --jobs N         Set max parallel jobs (or use -jN)\n");
  printf("\nExamples:\n");
  printf("  %s -j4                 Build default target with 4 jobs\n", prog);
  printf("  %s app libutil.o       Build specified targets\n", prog);
}

int parse_args(int argc, char **argv, CliOptions *out) {
  memset(out, 0, sizeof(*out));
  out->jobs = 0;

  for (int i = 1; i < argc; ++i) {
    const char *a = argv[i];

    if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
      out->show_help = 1;
      continue;
    }
    if (strcmp(a, "-v") == 0 || strcmp(a, "--verbose_mode") == 0) {
      out->verbose_mode = 1;
      continue;
    }
    if (strcmp(a, "-j") == 0 || strcmp(a, "--jobs") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "Option %s requires an integer\n", a);
        return -1;
      }
      char *end = NULL;
      errno = 0;
      // 用十进制解析为 long，end 会指向数字解析结束的位置
      long v = strtol(argv[++i], &end, 10);
      if (argv[i] == end || *end != '\0' || errno == ERANGE) {
        fprintf(stderr, "Invalid jobs value: '%s'\n", argv[i]);
        return -1;
      }
      out->jobs = (v > 0) ? (int)v : 0;
      continue;
    }
    if (strncmp(a, "-j", 2) == 0 && a[2] != '\0') { // -jN
      char *end = NULL;
      errno = 0;
      long v = strtol(a + 2, &end, 10);
      if (*(a + 2) == '\0' || *end != '\0' || errno == ERANGE) {
        fprintf(stderr, "Invalid jobs value: '%s'\n", a);
        return -1;
      }
      out->jobs = (v > 0) ? (int)v : 0;
      continue;
    }
    if (a[0] == '-') {
      // 未知选项
      fprintf(stderr, "Unknown option: %s\n", a);
      return -1;
    }

    // 非选项：认为是构建目标名
    safe_copy(out->targets_arr[out->n_targets],
              sizeof(out->targets_arr[out->n_targets]), a);
    out->n_targets++;
  }

  return 0;
}