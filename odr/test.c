#include <stdio.h>
#include <odr.h>

typedef ODR_BITMASK Z_ReferenceId;

typedef struct Z_InitRequest
{
    Z_ReferenceId *referenceId;    /* OPTIONAL */
    ODR_BITMASK *options;
    ODR_BITMASK *protocolVersion;
    int *preferredMessageSize;
    int *maximumRecordSize;
    char *idAuthentication;      /* OPTIONAL */
    char *implementationId;      /* OPTIONAL */
    char *implementationName;    /* OPTIONAL */
    char *implementationVersion; /* OPTIONAL */
} Z_InitRequest;

int z_ReferenceId(ODR o, Z_ReferenceId **p, int opt)
{
    return odr_implicit(o, odr_octetstring, (ODR_OCT**) p, ODR_CONTEXT, 2, opt);
}

int z_InitRequest(ODR o, Z_InitRequest **p, int opt)
{
    Z_InitRequest *pp;

    if (!odr_sequence_begin(o, p, sizeof(Z_InitRequest)))
    	return opt;
    pp = *p;
    return
    	z_ReferenceId(o, &pp->referenceId, 1) &&
    	odr_implicit(o, odr_bitstring, &pp->protocolVersion, ODR_CONTEXT, 
	    3, 0) &&
	odr_implicit(o, odr_bitstring, &pp->options, ODR_CONTEXT, 4, 0) &&
	odr_implicit(o, odr_integer, &pp->preferredMessageSize, ODR_CONTEXT,
	    5, 0) &&
	odr_implicit(o, odr_integer, &pp->maximumRecordSize, ODR_CONTEXT,
	    6, 0) &&
	odr_implicit(o, odr_visiblestring, &pp->idAuthentication, ODR_CONTEXT,
	    7, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationId, ODR_CONTEXT,
	    110, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationName, ODR_CONTEXT,
	    111, 1) &&
	odr_implicit(o, odr_visiblestring, &pp->implementationVersion,
	    ODR_CONTEXT, 112, 1) &&
	odr_sequence_end(o);
}

int main()
{
    int i;
    unsigned char buf[4048];
    struct odr o;
    Z_InitRequest ireq, *ireqp, *ireq2p;
    ODR_BITMASK options, protocolVersion;
    char *iId = "YAZ", *iName = "Yet Another Z39.50 Implementation",
    	*iVersion = "0.1";
    int maximumRS = 4096, preferredMS = 2048;

    ODR_MASK_ZERO(&protocolVersion);
    ODR_MASK_SET(&protocolVersion, 0);
    ODR_MASK_SET(&protocolVersion, 1);

    ODR_MASK_ZERO(&options);
    ODR_MASK_SET(&options, 0);
    ODR_MASK_SET(&options, 1);
    ODR_MASK_SET(&options, 2);

    ireq.referenceId = 0;
    ireq.protocolVersion = &protocolVersion;
    ireq.options = &options;
    ireq.preferredMessageSize = &preferredMS;
    ireq.maximumRecordSize = &maximumRS;
    ireq.idAuthentication = 0;
    ireq.implementationId = iId;
    ireq.implementationName = iName;
    ireq.implementationVersion = iVersion;
    ireqp = &ireq;

    o.buf = buf;
    o.bp=o.buf;
    o.left = o.buflen = 1024;
    o.direction = ODR_ENCODE;
    o.t_class = -1;

    z_InitRequest(&o, &ireqp, 0);

    o.direction = ODR_DECODE;
    o.bp = o.buf;

    z_InitRequest(&o, &ireq2p, 0);
}    
