///
#include "Precompiled.h"
///
#include <Shlwapi.h>
///
#include <cpprest/json.h>
//
#include "HTTPServer.h"
#include "ProfileMetadata.h"

enum ParseCommandLineState {
  kParseContinue = 0,
  kParseSkipError = 1,
  kParseErrorArgument = 2,
  kParseSkipExit
};

std::unique_ptr<HTTPServer> g_HTTPServer;
ProfileMetadata g_profileMetadata;

void OnShutdown() { g_HTTPServer->close().wait(); }
void OnInitialize(string_t &address) {
  g_HTTPServer = std::unique_ptr<HTTPServer>(new HTTPServer(address));
  g_HTTPServer->open().wait();
}

bool ParseProfileMetadata(const wchar_t *file) {
  std::wstring profile;
  if (file == nullptr) {
    WCHAR buf[MAX_PATH] = {0};
    if (!GetEnvironmentVariableW(L"AppData", buf, MAX_PATH))
      return false;
    wcscat_s(buf, LR"(\Exile\Exile.json)");
    if (PathFileExistsW(buf)) {
      profile.assign(buf);
    } else if (PathFileExistsW(L"Exile.json")) {
      profile.assign(L"Exile.json");
    }
  } else {
    profile.assign(file);
  }
  std::wifstream jstream;
  jstream.open(profile, std::ios_base::in);
  std::error_code ec;
  web::json::value obj = web::json::value::parse(jstream, ec);
  if (ec) {
    fprintf(stderr, "Parse Json file failed \n");
    return false;
  }
  if (obj.has_field(L"install")) {
    g_profileMetadata.GitInstall() = obj.at(L"install").as_string();
  }
  if (obj.has_field(L"root")) {
    g_profileMetadata.Root() = obj.at(L"root").as_string();
  }
  if (obj.has_field(L"url")) {
    g_profileMetadata.Url() = obj.at(L"url").as_string();
  }
  if (g_profileMetadata.GitInstall().empty()) {
    BOOL result = AddGitInstallationLocationToPath();
  } else {
    AppendPathToEnvironment(g_profileMetadata.GitInstall());
  }
  WCHAR tmp_[MAX_PATH];
  GetEnvironmentVariableW(L"TEMP", tmp_, MAX_PATH);
  g_profileMetadata.Temporary().assign(tmp_);
  return true;
}

void PrintUsage() {
  const char *usage =
      R"(Exile Git HTTP Smart Server
Usage: Exile [option]
  --config      Set profile path,Default: %%AppData%%\Exile\Exile.json
  --help        Print usage and exit
  --version     Print version and exit
)";
  printf(usage);
}

void PrintVersion() { printf("1.0.0\n"); }

int ParseCmdline(int Argc, wchar_t **Argv) {
  if (Argc < 2) {
    if (ParseProfileMetadata(nullptr)) {
      return kParseContinue;
    }
    fprintf(stderr, "cannot open default profile !\n");
    return kParseSkipError;
  }
  for (auto i = 1; i < Argc; i++) {
    auto ArgN = Argv[i];
    if (ArgN[0] == '-') {
      if (_wcsicmp(ArgN, L"--version") == 0) {
        PrintVersion();
        return kParseSkipExit;
      } else if (_wcsicmp(ArgN, L"--help") == 0) {
        PrintUsage();
        return kParseSkipExit;
      } else if (_wcsicmp(ArgN, L"--config") == 0) {
        if (Argc > i + 1) {
          if (ParseProfileMetadata(Argv[i + 1]))
            return kParseContinue;
        }
        return kParseErrorArgument;
      } else {
        return kParseSkipError;
      }
    } else {
      /// Do NO Parsed Argument !!!!
      continue;
    }
  }
  return 0;
}

int wmain(int argc, wchar_t *argv[]) {
  auto result = ParseCmdline(argc, argv);
  switch (result) {
  case kParseContinue:
    break;
  case kParseSkipExit:
    return 0;
  default:
    return result;
  }

  //
  DebugMessage(kInfo, L"Initialize Git Smart Server. \nURL: %s\nRoot: %s\n",
			   g_profileMetadata.Url().data(),
			   g_profileMetadata.Root().data());
  OnInitialize(g_profileMetadata.Url());

#ifdef _DEBUG
  std::cout << "Press ENTER to exit." << std::endl;
  std::string line;
  std::getline(std::cin, line);
  OnShutdown();
#else

#endif
  return 0;
}
