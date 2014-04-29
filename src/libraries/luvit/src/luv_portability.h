/*
 *  Copyright 2012 The Luvit Authors. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef LUV_PORTABILITY
#define LUV_PORTABILITY

#if _MSC_VER
#define snprintf _snprintf
#define getpid _getpid
#endif

#if defined(__OpenBSD__) || defined(__MINGW32__) || defined(_MSC_VER)
//# include <nameser.h>
#else
# include <arpa/nameser.h>
#endif

/* Temporary hack: libuv should provide uv_inet_pton and uv_inet_ntop. */
#if 0
#if defined(__MINGW32__) || defined(_MSC_VER)
# include <inet_net_pton.h>
# include <inet_ntop.h>
# define uv_inet_pton ares_inet_pton
# define uv_inet_ntop ares_inet_ntop

#else /* __POSIX__ */
# include <arpa/inet.h>
# define uv_inet_pton inet_pton
# define uv_inet_ntop inet_ntop
#endif
#endif

/* These macros are standard POSIX but aren't in window's sys/stat.h */
#if defined(_WIN32)
# include <sys/types.h>
# include <sys/stat.h>
#ifndef S_ISREG
# define S_ISREG(x)  (((x) & _S_IFMT) == _S_IFREG)
#endif
#ifndef S_ISDIR
# define S_ISDIR(x)  (((x) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISFIFO
# define S_ISFIFO(x) 0
#endif
#ifndef S_ISCHR
# define S_ISCHR(x)  0
#endif
#ifndef S_ISBLK
# define S_ISBLK(x)  0
#endif
#ifndef S_ISLNK
# define S_ISLNK(x)  0
#endif
#ifndef S_ISSOCK
# define S_ISSOCK(x) 0
#endif
#endif

/* Portable method of getting the environment. */
char **luv_os_environ();

#endif
