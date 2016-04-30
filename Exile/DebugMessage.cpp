//
///
//
#include "Precompiled.h"

#ifdef _DEBUG
#include <stdarg.h>

enum ConsoleColorEnum {
  kConsoleColorReset,
  kConsoleColorDebug = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
  kConsoleColorInfo = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
  kConsoleColorWarn = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED,
  kConsoleColorError = FOREGROUND_INTENSITY | FOREGROUND_RED,
  kConsoleColorFatal = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE
};

class ConsoleFile {
public:
  ConsoleFile() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    oldColor = csbi.wAttributes;
  }
  bool Write(int level, const wchar_t *buf, DWORD counts) {
    WORD color;
    switch (level) {
    case kDebug:
      color = kConsoleColorDebug;
      break;
    case kInfo:
      color = kConsoleColorInfo;
      break;
    case kWarn:
      color = kConsoleColorWarn;
      break;
    case kError:
      color = kConsoleColorError;
      break;
    case kFatal:
      color = kConsoleColorFatal;
      break;
    default:
      color = oldColor;
      break;
    }
    SetConsoleTextAttribute(hConsole, color);
    DWORD dwWrite;
    WriteConsoleW(hConsole, buf, counts, &dwWrite, nullptr);
    SetConsoleTextAttribute(hConsole, oldColor);
    return true;
  }

private:
  HANDLE hConsole;
  WORD oldColor;
};

int DebugMessage(DebugMessageLevel level, const wchar_t *format, ...) {
  static ConsoleFile consoleFile;
  WCHAR buffer[8192];
  va_list ap;
  va_start(ap, format);
  auto l = vswprintf_s(buffer, format, ap);
  va_end(ap);
  consoleFile.Write(level, buffer, l);
  return 0;
}

#endif
