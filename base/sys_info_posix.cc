// Copyright (c) 2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sys_info.h"

#include <errno.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>

#if !defined(OS_MACOSX)
#include <gdk/gdk.h>
#endif

#if defined(OS_OPENBSD)
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#include "base/basictypes.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/utf_string_conversions.h"

namespace base {

int SysInfo::NumberOfProcessors() {
#if defined(OS_OPENBSD)
  int mib[] = { CTL_HW, HW_NCPU };
  int ncpu;
  size_t size = sizeof(ncpu);
  if (sysctl(mib, 2, &ncpu, &size, NULL, 0) == -1) {
    NOTREACHED();
    return 1;
  }
  return ncpu;
#else
  // It seems that sysconf returns the number of "logical" processors on both
  // mac and linux.  So we get the number of "online logical" processors.
  long res = sysconf(_SC_NPROCESSORS_ONLN);
  if (res == -1) {
    NOTREACHED();
    return 1;
  }

  return static_cast<int>(res);
#endif
}

#if !defined(OS_MACOSX)
// static
int64 SysInfo::AmountOfPhysicalMemory() {
#if defined(OS_FREEBSD)
  // _SC_PHYS_PAGES is not part of POSIX and not available on OS X or
  // FreeBSD
  // TODO(benl): I have no idea how to get this
  NOTIMPLEMENTED();
  return 0;
#else
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  if (pages == -1 || page_size == -1) {
    NOTREACHED();
    return 0;
  }

  return static_cast<int64>(pages) * page_size;
#endif
}
#endif

// static
int64 SysInfo::AmountOfFreeDiskSpace(const FilePath& path) {
  struct statvfs stats;
  if (statvfs(path.value().c_str(), &stats) != 0) {
    return -1;
  }
  return static_cast<int64>(stats.f_bavail) * stats.f_frsize;
}

// static
bool SysInfo::HasEnvVar(const wchar_t* var) {
  std::string var_utf8 = WideToUTF8(std::wstring(var));
  return getenv(var_utf8.c_str()) != NULL;
}

// static
std::wstring SysInfo::GetEnvVar(const wchar_t* var) {
  std::string var_utf8 = WideToUTF8(std::wstring(var));
  char* value = getenv(var_utf8.c_str());
  if (!value) {
    return std::wstring();
  } else {
    return UTF8ToWide(value);
  }
}

// static
std::string SysInfo::OperatingSystemName() {
  utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return "";
  }
  return std::string(info.sysname);
}

// static
std::string SysInfo::OperatingSystemVersion() {
  utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return "";
  }
  return std::string(info.release);
}

// static
std::string SysInfo::CPUArchitecture() {
  utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return "";
  }
  return std::string(info.machine);
}

#if !defined(OS_MACOSX)
// static
void SysInfo::GetPrimaryDisplayDimensions(int* width, int* height) {
  // Note that Bad Things Happen if this isn't called from the UI thread,
  // but also that there's no way to check that from here.  :(
  GdkScreen* screen = gdk_screen_get_default();
  if (width)
    *width = gdk_screen_get_width(screen);
  if (height)
    *height = gdk_screen_get_height(screen);
}

// static
int SysInfo::DisplayCount() {
  // Note that Bad Things Happen if this isn't called from the UI thread,
  // but also that there's no way to check that from here.  :(

  // This query is kinda bogus for Linux -- do we want number of X screens?
  // The number of monitors Xinerama has?  We'll just use whatever GDK uses.
  GdkScreen* screen = gdk_screen_get_default();
  return gdk_screen_get_n_monitors(screen);
}
#endif

// static
size_t SysInfo::VMAllocationGranularity() {
  return getpagesize();
}

#if defined(OS_LINUX)
// static
size_t SysInfo::MaxSharedMemorySize() {
  static size_t limit;
  static bool limit_valid = false;

  if (!limit_valid) {
    std::string contents;
    file_util::ReadFileToString(FilePath("/proc/sys/kernel/shmmax"), &contents);
    limit = strtoul(contents.c_str(), NULL, 0);
    limit_valid = true;
  }

  return limit;
}
#endif

}  // namespace base
