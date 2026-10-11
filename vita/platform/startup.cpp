#include "startup.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace corsixth_vita {
namespace {

constexpr char kLogPath[] = "ux0:data/corsixth/logs/boot.log";
constexpr char kConfigPath[] = "ux0:data/corsixth/user";
FILE* boot_log = nullptr;

bool ensure_directory(const char* path) {
  if (mkdir(path, 0777) == 0) return true;
  const int error_number = errno;
  if (error_number == EEXIST) {
    struct stat info {};
    if (stat(path, &info) == 0) {
      if (S_ISDIR(info.st_mode)) return true;
      log_errno("filesystem", path, ENOTDIR);
      return false;
    }
    log_errno("filesystem", path, errno);
    return false;
  }
  log_errno("filesystem", path, error_number);
  return false;
}

bool check_resource(const char* path) {
  FILE* input = std::fopen(path, "rb");
  if (!input) {
    log_errno("resource", path, errno);
    return false;
  }
  if (std::fclose(input) != 0) {
    log_errno("resource", path, errno);
    return false;
  }
  return log("resource", path);
}

bool check_config_redirect() {
  FILE* input = std::fopen("app0:/config.path.txt", "rb");
  if (!input) {
    log_errno("resource", "app0:/config.path.txt", errno);
    return false;
  }
  char contents[128] = {};
  const bool read_ok = std::fgets(contents, sizeof(contents), input) != nullptr;
  const int read_error = read_ok ? 0 : (std::ferror(input) ? errno : EINVAL);
  const int close_result = std::fclose(input);
  if (!read_ok || close_result != 0) {
    log_errno("resource", "read app0:/config.path.txt",
              read_ok ? errno : read_error);
    return false;
  }
  contents[std::strcspn(contents, "\r\n")] = '\0';
  if (std::strcmp(contents, kConfigPath) != 0) {
    log("resource", "FAIL config.path.txt does not select the Vita user directory");
    return false;
  }
  return log("resource", "config.path.txt selects ux0:data/corsixth/user");
}

}  // namespace

bool log(const char* phase, const char* detail) {
  if (!boot_log) {
    std::fprintf(stderr, "[vita:%s] %s\n", phase, detail);
    std::fflush(stderr);
    return false;
  }
  if (std::fprintf(boot_log, "[vita:%s] %s\n", phase, detail) < 0 ||
      std::fflush(boot_log) != 0) {
    const int error_number = errno;
    std::fprintf(stderr, "[vita:logging] write/flush %s failed: errno=%d (%s)\n",
                 kLogPath, error_number, std::strerror(error_number));
    std::fflush(stderr);
    return false;
  }
  return true;
}

void log_errno(const char* phase, const char* operation, int error_number) {
  char detail[320];
  std::snprintf(detail, sizeof(detail), "FAIL %s errno=%d (%s)", operation,
                error_number, std::strerror(error_number));
  log(phase, detail);
}

bool initialize() {
  if (!ensure_directory("ux0:data/corsixth") ||
      !ensure_directory("ux0:data/corsixth/logs")) return false;
  boot_log = std::fopen(kLogPath, "w");
  if (!boot_log) {
    log_errno("logging", kLogPath, errno);
    return false;
  }
  if (!log("platform", "boot log open; app0: resources are read-only")) return false;
  if (!ensure_directory(kConfigPath)) return false;
  constexpr const char* user_directories[] = {
      "ux0:data/corsixth/user/Saves",
      "ux0:data/corsixth/user/Screenshots",
      "ux0:data/corsixth/user/Levels",
      "ux0:data/corsixth/user/Campaigns",
      "ux0:data/corsixth/user/Logs",
  };
  for (const char* path : user_directories) {
    if (!ensure_directory(path)) return false;
  }
  if (!log("filesystem", "writable user, save, screenshot, level, campaign and log directories ready"))
    return false;
  constexpr const char* resources[] = {
      "app0:/CorsixTH.lua",
      "app0:/Lua/config_finder.lua",
      "app0:/Bitmap/bootstrap_font.dat",
      "app0:/Campaigns/example.campaign",
      "app0:/Levels/example.level",
  };
  for (const char* path : resources) {
    if (!check_resource(path)) return false;
  }
  if (!check_config_redirect()) return false;
  return log("platform", "startup path checks passed");
}

void shutdown() {
  if (!boot_log) return;
  log("platform", "shutdown");
  const int close_result = std::fclose(boot_log);
  const int close_error = errno;
  boot_log = nullptr;
  if (close_result != 0)
    log_errno("logging", "close boot.log", close_error);
}

}  // namespace corsixth_vita
