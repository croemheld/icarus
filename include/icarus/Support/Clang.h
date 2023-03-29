//
// Created by croemheld on 28.03.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_CLANG_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_CLANG_H

#if defined(__clang__) && !defined(ICARUS_CLANG_VERSION)

#if __has_warning("-Wsingle-bit-bitfield-constant-conversion")
#define ICARUS_CLANG_VERSION 16
#elif __has_warning("-Walways-inline-coroutine")
#define ICARUS_CLANG_VERSION 15
#elif __has_warning("-Wargument-undefined-behaviour")
#define ICARUS_CLANG_VERSION 14
#elif __has_warning("-Waix-compat")
#define ICARUS_CLANG_VERSION 13
#elif __has_warning("-Wformat-insufficient-args")
#define ICARUS_CLANG_VERSION 12
#elif __has_warning("-Wimplicit-const-int-float-conversion")
#define ICARUS_CLANG_VERSION 11
#elif __has_warning("-Wmisleading-indentation")
#define ICARUS_CLANG_VERSION 10
#elif defined(__FILE_NAME__)
#define ICARUS_CLANG_VERSION 9
#elif __has_warning("-Wextra-semi-stmt") || __has_builtin(__builtin_rotateleft32)
#define ICARUS_CLANG_VERSION 8
#elif __has_warning("-Wc++98-compat-extra-semi")
#define ICARUS_CLANG_VERSION 7
#elif __has_warning("-Wpragma-pack")
#define ICARUS_CLANG_VERSION 6
#elif __has_warning("-Wbitfield-enum-conversion")
#define ICARUS_CLANG_VERSION 5
#elif __has_attribute(diagnose_if)
#define ICARUS_CLANG_VERSION 4
#else
#define ICARUS_CLANG_VERSION 1
#endif

#endif /* defined(__clang__) && !defined(ICARUS_CLANG_VERSION) */

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_CLANG_H
