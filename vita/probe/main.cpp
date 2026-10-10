#include <SDL.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/stat.h>

#ifndef PROBE_GIT_SHA
#define PROBE_GIT_SHA "unknown"
#endif
#ifndef PROBE_BUILD_MODE
#define PROBE_BUILD_MODE "unknown"
#endif
#ifndef PROBE_SDK_ID
#define PROBE_SDK_ID "unrecorded"
#endif

namespace {
constexpr int kWidth = 960;
constexpr int kHeight = 544;
constexpr char kLogDir[] = "ux0:data/corsixth";
constexpr char kLogPath[] = "ux0:data/corsixth/probe.log";

enum Check { LOGFILE, SDL_INIT, WINDOW, RENDERER, TARGET, DRAW_TARGET, COMPOSITE, CHECK_COUNT };
const char* const kCheckNames[] = {
    "LOGFILE", "SDL INIT", "WINDOW", "RENDERER", "TARGET", "DRAW TARGET", "COMPOSITE"};
enum Result { UNTESTED, PASS, FAIL };

FILE* log_file = nullptr;
Result results[CHECK_COUNT] = {};
int runs = 0;

void log_line(const char* message) {
  std::printf("%s\n", message);
  std::fflush(stdout);
  if (log_file) {
    std::fprintf(log_file, "%s\n", message);
    if (std::fflush(log_file) != 0) {
      std::printf("log flush failed: errno=%d (%s)\n", errno, std::strerror(errno));
      std::fflush(stdout);
      results[LOGFILE] = FAIL;
    }
  }
}

void log_check(Check check, Result result, const char* detail) {
  results[check] = result;
  char line[512];
  std::snprintf(line, sizeof(line), "run=%d check=%s result=%s detail=%.440s", runs,
                kCheckNames[check], result == PASS ? "PASS" : "FAIL", detail);
  log_line(line);
}

void log_sdl_failure(Check check, const char* operation) {
  char line[512];
  std::snprintf(line, sizeof(line), "%s: SDL error: %s", operation, SDL_GetError());
  log_check(check, FAIL, line);
}

void open_log() {
  if (mkdir(kLogDir, 0777) != 0 && errno != EEXIST) {
    char line[256];
    std::snprintf(line, sizeof(line), "mkdir %s failed: errno=%d (%s)",
                  kLogDir, errno, std::strerror(errno));
    log_check(LOGFILE, FAIL, line);
    return;
  }
  log_file = std::fopen(kLogPath, "w");
  if (!log_file) {
    char line[256];
    std::snprintf(line, sizeof(line), "fopen %s failed: errno=%d (%s)",
                  kLogPath, errno, std::strerror(errno));
    log_check(LOGFILE, FAIL, line);
    return;
  }
  log_check(LOGFILE, PASS, kLogPath);
}

// Compact 5x7 uppercase glyphs for diagnostic text; no font or asset dependency.
const char* glyph(char c) {
  switch (c) {
    case 'A': return "01110100011000111111100011000110001";
    case 'B': return "11110100011000111110100011000111110";
    case 'C': return "01111100001000010000100001000001111";
    case 'D': return "11110100011000110001100011000111110";
    case 'E': return "11111100001000011110100001000011111";
    case 'F': return "11111100001000011110100001000010000";
    case 'G': return "01111100001000010111100011000101111";
    case 'H': return "10001100011000111111100011000110001";
    case 'I': return "11111001000010000100001000010011111";
    case 'J': return "00111000100001000010100101001001100";
    case 'K': return "10001100101010011000101001001010001";
    case 'L': return "10000100001000010000100001000011111";
    case 'M': return "10001110111010110101100011000110001";
    case 'N': return "10001110011010110011100011000110001";
    case 'O': return "01110100011000110001100011000101110";
    case 'P': return "11110100011000111110100001000010000";
    case 'Q': return "01110100011000110001101011001001101";
    case 'R': return "11110100011000111110101001001010001";
    case 'S': return "01111100001000001110000010000111110";
    case 'T': return "11111001000010000100001000010000100";
    case 'U': return "10001100011000110001100011000101110";
    case 'V': return "10001100011000110001100010101000100";
    case 'W': return "10001100011000110101101011101110001";
    case 'X': return "10001100010101000100010101000110001";
    case 'Y': return "10001100010101000100001000010000100";
    case 'Z': return "11111000010001000100010001000011111";
    case '0': return "01110100011001110101110011000101110";
    case '1': return "00100011000010000100001000010001110";
    case '2': return "01110100010000100010001000100011111";
    case '3': return "11110000010000101110000010000111110";
    case '4': return "00010001100101010010111110001000010";
    case '5': return "11111100001000011110000010000111110";
    case '6': return "01111100001000011110100011000101110";
    case '7': return "11111000010001000100010000100001000";
    case '8': return "01110100011000101110100011000101110";
    case '9': return "01110100011000101111000010000111110";
    case ':': return "00000001000010000000001000010000000";
    case '/': return "00001000010001000100010001000010000";
    case '-': return "00000000000000011111000000000000000";
    case '.': return "00000000000000000000000000011000110";
    case '|': return "00100001000010000100001000010000100";
    case ' ': return "00000000000000000000000000000000000";
    default: return "11111100010001000100001000000000100";
  }
}

bool color(SDL_Renderer* renderer, Uint8 red, Uint8 green, Uint8 blue) {
  return SDL_SetRenderDrawColor(renderer, red, green, blue, 255) == 0;
}

bool text(SDL_Renderer* renderer, int x, int y, int scale, const char* value) {
  for (const char* p = value; *p; ++p, x += 6 * scale) {
    const char* bits = glyph(*p);
    for (int row = 0; row < 7; ++row) {
      for (int col = 0; col < 5; ++col) {
        if (bits[row * 5 + col] == '1') {
          SDL_Rect pixel{x + col * scale, y + row * scale, scale, scale};
          if (SDL_RenderFillRect(renderer, &pixel) != 0) return false;
        }
      }
    }
  }
  return true;
}

bool fill(SDL_Renderer* renderer, SDL_Rect rect, Uint8 red, Uint8 green, Uint8 blue) {
  return color(renderer, red, green, blue) && SDL_RenderFillRect(renderer, &rect) == 0;
}

bool draw_target(SDL_Renderer* renderer) {
  if (!color(renderer, 230, 0, 180) || SDL_RenderClear(renderer) != 0) return false;
  SDL_Rect teal{0, 120, 320, 120};
  SDL_Rect red{112, 78, 96, 84};
  if (!fill(renderer, teal, 0, 220, 190) || !fill(renderer, red, 245, 28, 20)) return false;
  if (!color(renderer, 255, 235, 0)) return false;
  return SDL_RenderDrawLine(renderer, 8, 8, 311, 231) == 0 &&
         SDL_RenderDrawLine(renderer, 311, 8, 8, 231) == 0;
}

bool draw_status(SDL_Renderer* renderer) {
  SDL_Rect strip{20, 90, 285, 330};
  if (!fill(renderer, strip, 12, 19, 32)) return false;
  char line[100];
  for (int i = 0; i < CHECK_COUNT; ++i) {
    const Result result = results[i];
    if (!color(renderer, result == PASS ? 20 : 255,
               result == PASS ? 230 : result == FAIL ? 50 : 190,
               result == UNTESTED ? 30 : 50)) return false;
    std::snprintf(line, sizeof(line), "%s: %s", kCheckNames[i],
                  result == PASS ? "PASS" : result == FAIL ? "FAIL" : "UNTESTED");
    if (!text(renderer, 32, 105 + i * 43, 2, line)) return false;
  }
  return true;
}

bool draw_backbuffer(SDL_Renderer* renderer) {
  if (!color(renderer, 12, 19, 32) || SDL_RenderClear(renderer) != 0) return false;
  SDL_Rect blue{40, 476, 880, 24};
  SDL_Rect left{315, 145, 14, 274};
  SDL_Rect right{775, 145, 14, 274};
  if (!fill(renderer, blue, 20, 80, 255) || !fill(renderer, left, 255, 255, 255) ||
      !fill(renderer, right, 255, 255, 255)) return false;
  if (!color(renderer, 240, 240, 240) ||
      !text(renderer, 32, 22, 2, "CORSIXTH-VITA / M1A GXM PROBE")) return false;
  char line[100];
  std::snprintf(line, sizeof(line), "BUILD %.12s  RUN %d", PROBE_GIT_SHA, runs);
  if (!text(renderer, 32, 48, 2, line)) return false;
  if (!draw_status(renderer)) return false;
  if (!color(renderer, 240, 240, 240) ||
      !text(renderer, 32, 515, 2, "CROSS: RERUN  |  START: EXIT")) return false;
  return true;
}

void graphics_run(SDL_Renderer* renderer, bool native_renderer) {
  ++runs;
  log_line("graphics run begin");
  results[TARGET] = results[DRAW_TARGET] = results[COMPOSITE] = UNTESTED;
  SDL_Texture* target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_TARGET, 320, 240);
  if (!target) {
    log_sdl_failure(TARGET, "SDL_CreateTexture RGBA8888 TARGET 320x240");
  } else {
    log_check(TARGET, PASS, "RGBA8888 SDL_TEXTUREACCESS_TARGET 320x240 created");
    if (SDL_SetRenderTarget(renderer, target) != 0) {
      log_sdl_failure(DRAW_TARGET, "SDL_SetRenderTarget(texture)");
    } else {
      const bool drawn = draw_target(renderer);
      char draw_error[512];
      std::snprintf(draw_error, sizeof(draw_error), "%s", SDL_GetError());
      if (SDL_SetRenderTarget(renderer, nullptr) != 0) {
        log_sdl_failure(DRAW_TARGET, "SDL_SetRenderTarget(NULL)");
      } else if (!drawn) {
        log_check(DRAW_TARGET, FAIL, draw_error);
      } else {
        log_check(DRAW_TARGET, PASS, "magenta teal red rectangle yellow X rendered offscreen");
      }
    }
  }
  if (SDL_GetRenderTarget(renderer) != nullptr && SDL_SetRenderTarget(renderer, nullptr) != 0) {
    log_sdl_failure(COMPOSITE, "restore default render target");
  } else if (!draw_backbuffer(renderer)) {
    log_sdl_failure(COMPOSITE, "draw direct backbuffer reference/status");
  } else {
    if (target && results[DRAW_TARGET] == PASS) {
      SDL_Rect destination{365, 160, 384, 288};
      if (SDL_RenderCopy(renderer, target, nullptr, &destination) != 0) {
        log_sdl_failure(COMPOSITE, "SDL_RenderCopy(target)");
      } else {
        log_check(COMPOSITE, PASS, "target copied to backbuffer; visual verification requires Vita photo");
      }
    } else {
      log_check(COMPOSITE, FAIL, "offscreen target unavailable");
    }
    if (!draw_status(renderer)) log_sdl_failure(COMPOSITE, "draw final status strip");
    if (!native_renderer) log_line("GXM evidence FAIL: renderer not identified as native accelerated Vita renderer");
    SDL_RenderPresent(renderer);
    log_line("SDL_RenderPresent called (void API; physical output unverified)");
  }
  if (target) SDL_DestroyTexture(target);
  log_line("graphics run end");
}

}  // namespace

int main() {
  open_log();
  char line[512];
  std::time_t now = std::time(nullptr);
  std::snprintf(line, sizeof(line), "build git=%s mode=%s sdk=%s compiler=%s cxx=%ld time=%lld",
                PROBE_GIT_SHA, PROBE_BUILD_MODE, PROBE_SDK_ID, __VERSION__, static_cast<long>(__cplusplus),
                static_cast<long long>(now));
  log_line(line);
  SDL_version compiled, runtime;
  SDL_VERSION(&compiled);
  SDL_GetVersion(&runtime);
  std::snprintf(line, sizeof(line), "SDL compiled=%u.%u.%u runtime=%u.%u.%u",
                compiled.major, compiled.minor, compiled.patch,
                runtime.major, runtime.minor, runtime.patch);
  log_line(line);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) {
    log_sdl_failure(SDL_INIT, "SDL_Init");
    if (log_file) std::fclose(log_file);
    return 1;
  }
  log_check(SDL_INIT, PASS, "video events gamecontroller initialized");
  SDL_Window* window = SDL_CreateWindow("CorsixTH Vita GXM Probe", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, kWidth, kHeight, SDL_WINDOW_SHOWN);
  if (!window) {
    log_sdl_failure(WINDOW, "SDL_CreateWindow");
    SDL_Quit();
    if (log_file) std::fclose(log_file);
    return 1;
  }
  log_check(WINDOW, PASS, "960x544 SDL_WINDOW_SHOWN; no SDL_WINDOW_OPENGL");
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    log_sdl_failure(RENDERER, "SDL_CreateRenderer accelerated");
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer) log_line("fallback renderer created for failure display only");
  }
  bool native_renderer = false;
  if (renderer) {
    SDL_RendererInfo info{};
    if (SDL_GetRendererInfo(renderer, &info) != 0) {
      log_sdl_failure(RENDERER, "SDL_GetRendererInfo");
    } else {
      native_renderer = (info.flags & SDL_RENDERER_ACCELERATED) &&
                        !(info.flags & SDL_RENDERER_SOFTWARE) &&
                        (std::strstr(info.name, "vita") || std::strstr(info.name, "Vita") ||
                         std::strstr(info.name, "gxm") || std::strstr(info.name, "GXM"));
      std::snprintf(line, sizeof(line),
                    "renderer name=%s flags=0x%08x accelerated=%d software=%d targettexture=%d max=%dx%d formats=%u",
                    info.name, info.flags, !!(info.flags & SDL_RENDERER_ACCELERATED),
                    !!(info.flags & SDL_RENDERER_SOFTWARE),
                    !!(info.flags & SDL_RENDERER_TARGETTEXTURE),
                    info.max_texture_width, info.max_texture_height, info.num_texture_formats);
      log_line(line);
      for (Uint32 i = 0; i < info.num_texture_formats; ++i) {
        std::snprintf(line, sizeof(line), "renderer texture_format[%u]=%s", i,
                      SDL_GetPixelFormatName(info.texture_formats[i]));
        log_line(line);
      }
      log_check(RENDERER, native_renderer && (info.flags & SDL_RENDERER_TARGETTEXTURE) ? PASS : FAIL,
                native_renderer ? "native Vita renderer; target texture flag checked" :
                                  "native accelerated Vita renderer not identified");
    }
  } else {
    log_sdl_failure(RENDERER, "SDL_CreateRenderer fallback");
  }

  SDL_GameController* controller = nullptr;
  SDL_Joystick* joystick = nullptr;
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) { controller = SDL_GameControllerOpen(i); break; }
  }
  if (!controller && SDL_NumJoysticks() > 0) joystick = SDL_JoystickOpen(0);
  log_line(controller ? "input: SDL_GameController Cross=A Start=START" :
           joystick ? "input: SDL_Joystick Cross=button2 Start=button11" :
                      "input: no controller opened; SDL keyboard events still handled");
  if (renderer) graphics_run(renderer, native_renderer);
  bool running = true;
  while (running) {
    SDL_Event event;
    if (!SDL_WaitEventTimeout(&event, 100)) continue;
    bool rerun = false;
    if (event.type == SDL_QUIT) running = false;
    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
      rerun = event.cbutton.button == SDL_CONTROLLER_BUTTON_A;
      running = running && event.cbutton.button != SDL_CONTROLLER_BUTTON_START;
    }
    if (event.type == SDL_JOYBUTTONDOWN && !controller) {
      rerun = event.jbutton.button == 2;
      running = running && event.jbutton.button != 11;
    }
    if (event.type == SDL_KEYDOWN) {
      rerun = event.key.keysym.sym == SDLK_x;
      running = running && event.key.keysym.sym != SDLK_RETURN;
    }
    if (rerun && renderer) graphics_run(renderer, native_renderer);
  }
  std::snprintf(line, sizeof(line), "final runs=%d logfile=%s renderer=%s target=%s draw=%s composite=%s",
                runs, results[LOGFILE] == PASS ? "PASS" : "FAIL",
                results[RENDERER] == PASS ? "PASS" : "FAIL",
                results[TARGET] == PASS ? "PASS" : "FAIL",
                results[DRAW_TARGET] == PASS ? "PASS" : "FAIL",
                results[COMPOSITE] == PASS ? "PASS" : "FAIL");
  log_line(line);
  if (joystick) SDL_JoystickClose(joystick);
  if (controller) SDL_GameControllerClose(controller);
  if (renderer) SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  if (log_file) { std::fflush(log_file); std::fclose(log_file); }
  return 0;
}
