/*
 * A quick little implementation of the makensap function of the
 * xtimosi package. It's needed to make the demo apps work.
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

static struct sockaddr_in *tcpip_strtoaddr(const char *str);

/*
 * text format is <hostname> | <IP-address> ':' <port>. No spaces.
 * The binary form is a struct sockaddr_ip.
 */
int makensap(char *text, unsigned char *binary)
{
    struct sockaddr_in *addr;

    fprintf(stderr, "Mapping textform NSAP '%s'\n", text);
    if (!(addr = tcpip_strtoaddr(text)))
    {
        fprintf(stderr, "Failed to resolve '%s'\n", text);
        return -1;
    }
    memcpy(binary, addr, sizeof(struct sockaddr_in));
    return sizeof(struct sockaddr_in);
}

/*
 * This is the address mapper from the comstack submodule of YAZ.
 */
static struct sockaddr_in *tcpip_strtoaddr(const char *str)
{
    static struct sockaddr_in add;
    struct hostent *hp;
    char *p, buf[512];
    short int port = 210;
    unsigned long tmpadd;

    add.sin_family = AF_INET;
    strcpy(buf, str);
    if ((p = strchr(buf, ':')))
    {
        *p = 0;
        port = atoi(p + 1);
    }
    add.sin_port = htons(port);
    if ((hp = gethostbyname(buf)))
        memcpy(&add.sin_addr.s_addr, *hp->h_addr_list, sizeof(struct in_addr));
    else if ((tmpadd = inet_addr(buf)) != 0)
        memcpy(&add.sin_addr.s_addr, &tmpadd, sizeof(struct in_addr));
    else
        return 0;
    return &add;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

