#pragma once
// Product version. Bump APP_VERSION on every user-facing update.

namespace rdc {

inline constexpr const wchar_t* kAppVersion = L"2.6.1";
inline constexpr const wchar_t* kAppReleaseDate = L"2026-09-13";

inline const wchar_t* VersionStamp() { return L"2.6.1  (2026-09-13)"; }

}  // namespace rdc
