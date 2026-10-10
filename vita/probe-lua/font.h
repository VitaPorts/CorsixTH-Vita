#pragma once
#include <SDL.h>
namespace probe_font {
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
} // namespace probe_font
