//
//
//
#ifndef PROFILE_METADATA_H
#define PROFILE_METADATA_H

#include <string>

class ProfileMetadata {
public:
  ProfileMetadata() {
	  url_ = L"http://localhost:10240";
	  root_ = LR"(F:\Github)";
  }
  std::wstring &Root() { return this->root_; }
  std::wstring &GitInstall() { return this->install_; }
  std::wstring &Url() { return this->url_; }
  std::wstring &Temporary() { return this->tmp_; }
  const std::wstring &Root() const { return this->root_; }
  const std::wstring &GitInstall() const { return this->install_; }
  const std::wstring &Url()const { return this->url_; }
  const std::wstring &Temporary()const { return this->tmp_; }
private:
  std::wstring root_;
  std::wstring install_;
  std::wstring url_;
  std::wstring tmp_;
};

#endif
