///

#include <Windows.h>
#include <assert.h>
#include <string>

#pragma comment(lib, "crypt32.lib")

#ifdef _DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

BOOL AuthorizedWithBasic(const std::wstring &context) {
  if (context.size() < 8)
    return FALSE;
  BOOL result = FALSE;
  DWORD dwSize = 0;
  DWORD dwSkipChars = 0;
  DWORD dwActualFormat = 0;
  std::string user, pwd;
  auto ptr = context.c_str();
  auto size = context.size();
  if (context.compare(0, 6, L"Basic ") == 0) {
    ptr += 6;
    size = context.size() - 6;
  }
  if (CryptStringToBinaryW(ptr, NULL, CRYPT_STRING_BASE64, NULL, &dwSize,
                           &dwSkipChars, &dwActualFormat)) {
    BYTE *pbContent = static_cast<BYTE *>(LocalAlloc(LPTR, dwSize));
    CryptStringToBinaryW(ptr, size, CRYPT_STRING_BASE64, pbContent, &dwSize,
                         &dwSkipChars, &dwActualFormat);
    std::string ctx(reinterpret_cast<char *>(pbContent), dwSize);
    std::string::size_type np;
    if ((np = ctx.find(':')) != ctx.npos) {
      user = ctx.substr(0, np);
      pwd = ctx.substr(np);
      result = TRUE;
    }
    LocalFree(pbContent);
  }
  if (result) {
    printf("User: %s  Password: %s \n", user.c_str(), pwd.c_str());
  }
  return result;
}