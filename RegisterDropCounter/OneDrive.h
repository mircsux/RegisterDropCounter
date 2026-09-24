#pragma once
// OneDrive history folder: write the same RDCH1 file the OneDrive client syncs.

#include "History.h"

#include <shlobj.h>

#include <fstream>
#include <string>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

namespace rdc {

struct Options {
  bool syncOneDrive = false;
  bool darkMode = false;
  std::wstring fontFace = L"Segoe UI";
  std::wstring folder;  // empty = auto-detect OneDrive\RegisterDropCounter
};

// GDI face names are at most 31 characters. Any Unicode in that range is kept.
inline std::wstring SanitizeFontFace(std::wstring name) {
  std::wstring s;
  s.reserve(name.size());
  for (wchar_t c : name) {
    if (c == 0 || c == L'\r' || c == L'\n' || c == L'\t') continue;
    s.push_back(c);
    if (s.size() >= 31) break;
  }
  if (!s.empty()) {
    const wchar_t last = s.back();
    if (last >= 0xD800 && last <= 0xDBFF) s.pop_back();
  }
  size_t a = 0;
  while (a < s.size() && s[a] == L' ') ++a;
  size_t b = s.size();
  while (b > a && s[b - 1] == L' ') --b;
  s = s.substr(a, b - a);
  if (s.empty()) return L"Segoe UI";
  return s;
}

inline std::wstring DetectOneDriveRoot() {
#if !defined(_WIN32)
  return L"";
#else
  wchar_t buf[MAX_PATH];
  const wchar_t* vars[] = {L"OneDriveConsumer", L"OneDrive", L"OneDriveCommercial"};
  for (auto v : vars) {
    DWORD n = GetEnvironmentVariableW(v, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
      DWORD attr = GetFileAttributesW(buf);
      if (attr != INVALID_FILE_ATTRIBUTES) return buf;
    }
  }
  PWSTR p = nullptr;
  // OneDrive known folder (FOLDERID_SkyDriveFolder), inlined so any SDK compiles.
  static const GUID kOneDriveFolder = {
      0xA52BBA46, 0xE9E1, 0x435f, {0xB3, 0xD9, 0x28, 0xDA, 0xA6, 0x48, 0xC0, 0xF6}};
  if (SUCCEEDED(SHGetKnownFolderPath(kOneDriveFolder, 0, nullptr, &p)) && p) {
    std::wstring s(p);
    CoTaskMemFree(p);
    return s;
  }
  return L"";
#endif
}

inline bool FolderExists(const std::wstring& dir) {
#if !defined(_WIN32)
  (void)dir;
  return false;
#else
  if (dir.empty()) return false;
  DWORD attr = GetFileAttributesW(dir.c_str());
  return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

inline bool EnsureFolder(const std::wstring& dir) {
#if !defined(_WIN32)
  (void)dir;
  return false;
#else
  if (dir.empty()) return false;
  if (FolderExists(dir)) return true;
  int err = SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
  return err == ERROR_SUCCESS || err == ERROR_ALREADY_EXISTS || err == ERROR_FILE_EXISTS;
#endif
}

inline std::wstring DefaultSyncFolder() {
  std::wstring root = DetectOneDriveRoot();
  if (root.empty()) return L"";
  if (root.back() == L'\\' || root.back() == L'/') root.pop_back();
  return root + L"\\RegisterDropCounter";
}

inline std::wstring ResolvedSyncFolder(const Options& o) {
  if (!o.folder.empty() && FolderExists(o.folder)) return o.folder;
  return DefaultSyncFolder();
}

inline std::wstring CloudHistoryPath(const Options& o) {
  std::wstring folder = ResolvedSyncFolder(o);
  if (folder.empty()) return L"";
  return folder + L"\\RegisterDropCounter.history";
}

inline std::string WideToUtf8(const std::wstring& w) {
  if (w.empty()) return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
  if (n <= 0) return {};
  std::string out((size_t)n, '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], n, nullptr, nullptr);
  return out;
}

inline std::wstring BytesToWide(const std::string& bytes) {
  if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB &&
      (unsigned char)bytes[2] == 0xBF) {
    const char* p = bytes.data() + 3;
    int n = (int)bytes.size() - 3;
    int w = MultiByteToWideChar(CP_UTF8, 0, p, n, nullptr, 0);
    if (w <= 0) return L"";
    std::wstring out((size_t)w, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, p, n, &out[0], w);
    return out;
  }
  if (bytes.size() >= 2 && (unsigned char)bytes[0] == 0xFF && (unsigned char)bytes[1] == 0xFE) {
    size_t n = (bytes.size() - 2) / 2;
    return std::wstring(reinterpret_cast<const wchar_t*>(bytes.data() + 2), n);
  }
  int w = MultiByteToWideChar(CP_ACP, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
  if (w <= 0) return L"";
  std::wstring out((size_t)w, L'\0');
  MultiByteToWideChar(CP_ACP, 0, bytes.data(), (int)bytes.size(), &out[0], w);
  return out;
}

inline bool SaveOptions(const std::wstring& path, const Options& o) {
#if !defined(_WIN32)
  (void)path;
  (void)o;
  return false;
#else
  std::wstring body = L"RDCO1\n";
  body += o.syncOneDrive ? L"1\n" : L"0\n";
  body += o.folder;
  body += L"\n";
  body += o.darkMode ? L"1\n" : L"0\n";
  body += SanitizeFontFace(o.fontFace);
  body += L"\n";
  const std::string utf8 = std::string("\xEF\xBB\xBF") + WideToUtf8(body);
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            nullptr);
  if (file == INVALID_HANDLE_VALUE) return false;
  DWORD wrote = 0;
  const BOOL ok = WriteFile(file, utf8.data(), (DWORD)utf8.size(), &wrote, nullptr);
  CloseHandle(file);
  return ok && wrote == utf8.size();
#endif
}

inline bool LoadOptions(const std::wstring& path, Options* o) {
  o->syncOneDrive = false;
  o->darkMode = false;
  o->fontFace = L"Segoe UI";
  o->folder.clear();
#if !defined(_WIN32)
  (void)path;
  return false;
#else
  HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) return false;
  DWORD size = GetFileSize(file, nullptr);
  if (size == INVALID_FILE_SIZE || size > 1024 * 1024) {
    CloseHandle(file);
    return false;
  }
  std::string bytes((size_t)size, '\0');
  DWORD got = 0;
  const BOOL ok = size == 0 || ReadFile(file, &bytes[0], size, &got, nullptr);
  CloseHandle(file);
  if (!ok) return false;
  bytes.resize(got);
  const std::wstring text = BytesToWide(bytes);
  std::wstring lines[5];
  int n = 0;
  std::wstring cur;
  for (wchar_t c : text) {
    if (c == L'\n') {
      if (!cur.empty() && cur.back() == L'\r') cur.pop_back();
      if (n < 5) lines[n++] = cur;
      cur.clear();
    } else {
      cur.push_back(c);
    }
  }
  if (!cur.empty() && n < 5) {
    if (cur.back() == L'\r') cur.pop_back();
    lines[n++] = cur;
  }
  if (n < 1 || lines[0] != L"RDCO1") return false;
  if (n > 1) o->syncOneDrive = lines[1] == L"1";
  if (n > 2) o->folder = lines[2];
  if (n > 3) o->darkMode = lines[3] == L"1";
  if (n > 4) o->fontFace = SanitizeFontFace(lines[4]);
  return true;
#endif
}

}  // namespace rdc
