#ifndef _KERNEL_UTSNAME_H
#define _KERNEL_UTSNAME_H

struct utsname {
    char sysname[65];    /* Operating system name (e.g., "Linux") */
    char nodename[65];   /* Name within communications network
                          to which the node is attached, if any */
    char release[65];    /* Operating system release
                          (e.g., "2.6.28") */
    char version[65];    /* Operating system version */
    char machine[65];    /* Hardware type identifier */
    char domainname[65];
};

#endif