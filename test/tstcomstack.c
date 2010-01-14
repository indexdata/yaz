/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <yaz/test.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>

static int comstack_example(const char *server_address_str)
{    
    COMSTACK stack;
    char *buf = 0;
    int size = 0, length_incoming;
    void *server_address_ip;
    int status;

    char *protocol_package = "GET / HTTP/1.0\r\n\r\n";
    int protocol_package_length = strlen(protocol_package);

    stack = cs_create(tcpip_type, 1, PROTO_HTTP);
    if (!stack) {
        perror("cs_create");  /* use perror() here since we have no stack yet */
        return -1;
    }
    
    server_address_ip = cs_straddr(stack, server_address_str);
    if (!server_address_ip)
    {
        fprintf(stderr, "cs_straddr: address could not be resolved\n");
        return -1;
    }
    
    status = cs_connect(stack, server_address_ip);
    if (status != 0) {
        fprintf(stderr, "cs_connect: %s\n", cs_strerror(stack));
        return -1;
    }
    
    status = cs_put(stack, protocol_package, protocol_package_length);
    if (status) {
        fprintf(stderr, "cs_put: %s\n", cs_strerror(stack));
        return -1;
    }
    
    /* Now get a response */
    
    length_incoming = cs_get(stack, &buf, &size);
    if (!length_incoming) {
        fprintf(stderr, "Connection closed\n");
        return -1;
    } else if (length_incoming < 0) {
        fprintf(stderr, "cs_get: %s\n", cs_strerror(stack));
        return -1;
    }
    
    /* Print result */
    fwrite(buf, length_incoming, 1, stdout);
    
    /* clean up */
    cs_close(stack);
    if (buf)
        free(buf);
    return 0;
}


int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    if (argc == 2)
       comstack_example(argv[1]);
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

