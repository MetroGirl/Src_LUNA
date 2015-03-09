#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace luna {

inline
std::string wide_to_ascii(std::wstring const& wide)
{
  auto Length = ::WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
  std::vector<char> MB(Length);
  auto Result = ::WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, MB.data(), Length, nullptr, nullptr);
  std::string Return;
  std::copy(MB.begin(), MB.end(), std::back_inserter(Return));
  return Return;
}

inline
std::wstring ascii_to_wide(std::string const& ascii)
{
  auto length = ::MultiByteToWideChar(CP_ACP, 0, ascii.c_str(), -1, nullptr, 0);
  std::vector<wchar_t> wide(length);
  auto result = ::MultiByteToWideChar(CP_ACP, 0, ascii.c_str(), -1, wide.data(), length);
  std::wstring ret;
  std::copy(wide.begin(), wide.end(), std::back_inserter(ret));
  return ret;
}

}
