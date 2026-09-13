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
  std::wstring folder;  // empty = auto-detect OneDrive\RegisterDropCounter
};

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

inline bool SaveOptions(const std::wstring& path, const Options& o) {
#if !defined(_WIN32)
  (void)path;
  (void)o;
  return false;
#else
  std::wofstream out(path.c_str());
  if (!out) return false;
  out << L"RDCO1\n" << (o.syncOneDrive ? 1 : 0) << L"\n" << o.folder << L"\n";
  return true;
#endif
}

inline bool LoadOptions(const std::wstring& path, Options* o) {
  o->syncOneDrive = false;
  o->folder.clear();
#if !defined(_WIN32)
  (void)path;
  return false;
#else
  std::wifstream in(path.c_str());
  if (!in) return false;
  std::wstring mag;
  std::getline(in, mag);
  if (mag != L"RDCO1") return false;
  int sync = 0;
  in >> sync;
  in.ignore(64, L'\n');
  std::getline(in, o->folder);
  while (!o->folder.empty() && (o->folder.back() == L'\r' || o->folder.back() == L'\n'))
    o->folder.pop_back();
  o->syncOneDrive = sync != 0;
  return true;
#endif
}

}  // namespace rdc
