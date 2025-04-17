#ifndef CUSTOM_ERRNO_H
#define CUSTOM_ERRNO_H

#include <errno.h> 

#ifndef EADDRINUSE
#define EADDRINUSE      100
#endif

#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL   101
#endif

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT    102
#endif

#ifndef EALREADY
#define EALREADY        103
#endif

#ifndef EBADMSG
#define EBADMSG         104
#endif

#ifndef ECANCELED
#define ECANCELED       105
#endif

#ifndef ECONNABORTED
#define ECONNABORTED    106
#endif

#ifndef ECONNREFUSED
#define ECONNREFUSED    107
#endif

#ifndef ECONNRESET
#define ECONNRESET      108
#endif

#ifndef EDESTADDRREQ
#define EDESTADDRREQ    109
#endif

#ifndef EHOSTUNREACH
#define EHOSTUNREACH    110
#endif

#ifndef EIDRM
#define EIDRM           111
#endif

#ifndef EINPROGRESS
#define EINPROGRESS     112
#endif

#ifndef EISCONN
#define EISCONN         113
#endif

#ifndef ELOOP
#define ELOOP           114
#endif

#ifndef EMSGSIZE
#define EMSGSIZE        115
#endif

#ifndef ENETDOWN
#define ENETDOWN        116
#endif

#ifndef ENETRESET
#define ENETRESET       117
#endif

#ifndef ENETUNREACH
#define ENETUNREACH     118
#endif

#ifndef ENOBUFS
#define ENOBUFS         119
#endif

#ifndef ENODATA
#define ENODATA         120
#endif

#ifndef ENOLINK
#define ENOLINK         121
#endif

#ifndef ENOMSG
#define ENOMSG          122
#endif

#ifndef ENOPROTOOPT
#define ENOPROTOOPT     123
#endif

#ifndef ENOSR
#define ENOSR           124
#endif

#ifndef ENOSTR
#define ENOSTR          125
#endif

#ifndef ENOTCONN
#define ENOTCONN        126
#endif

#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE 127
#endif

#ifndef ENOTSOCK
#define ENOTSOCK        128
#endif

#ifndef ENOTSUP
#define ENOTSUP         129
#endif

#ifndef EOPNOTSUPP
#define EOPNOTSUPP      130
#endif

#ifndef EOTHER
#define EOTHER          131
#endif

#ifndef EOVERFLOW
#define EOVERFLOW       132
#endif

#ifndef EOWNERDEAD
#define EOWNERDEAD      133
#endif

#ifndef EPROTO
#define EPROTO          134
#endif

#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT 135
#endif

#ifndef EPROTOTYPE
#define EPROTOTYPE      136
#endif

#ifndef ETIME
#define ETIME           137
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT       138
#endif

#ifndef ETXTBSY
#define ETXTBSY         139
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK     140
#endif

#endif // CUSTOM_ERRNO_H