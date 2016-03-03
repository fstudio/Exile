/*
*/
#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H
#include <Windows.h>
#include <string>

struct AuthorizedInfo {
	std::string user;
	std::string pwd;
	void *data;
};

typedef bool(*AuthorizedImpl)(AuthorizedInfo *ai);

BOOL AuthorizedWithBasic(const std::wstring &context);

#endif