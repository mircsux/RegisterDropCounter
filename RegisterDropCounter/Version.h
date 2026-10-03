#pragma once
// Product version. Bump APP_VERSION on every user-facing update.

namespace rdc {

inline constexpr const wchar_t* kAppVersion = L"2.37.0";
inline constexpr const wchar_t* kAppReleaseDate = L"2026-10-03";

inline const wchar_t* VersionStamp() { return L"2.37.0  (2026-10-03)"; }

}  // namespace rdc
