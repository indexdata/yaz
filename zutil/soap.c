/*
 * Copyright (c) 2002-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: soap.c,v 1.1 2003-02-12 15:06:44 adam Exp $
 */

#include <yaz/soap.h>

static const char *soap_v1_1 = "http://schemas.xmlsoap.org/soap/envelope/";
static const char *soap_v1_2 = "http://www.w3.org/2001/06/soap-envelope";

int z_soap_error(ODR o, Z_SOAP *p,
                 const char *fault_code, const char *fault_string,
                 const char *details)
{
    p->which = Z_SOAP_error;
    p->u.soap_error = odr_malloc(o, sizeof(*p->u.soap_error));
    p->u.soap_error->fault_code = odr_strdup(o, fault_code);
    p->u.soap_error->fault_string = odr_strdup(o, fault_string);
    if (details)
        p->u.soap_error->details = odr_strdup(o, details);
    else
        p->u.soap_error->details = 0;
    return -1;
}

int z_soap_codec(ODR o, Z_SOAP **pp, 
                 char **content_buf, int *content_len,
                 Z_SOAP_Handler *handlers)
{
    if (o->direction == ODR_DECODE)
    {
        Z_SOAP *p;
        xmlNodePtr ptr, pptr;
        xmlDocPtr doc;
        int i, ret;

        if (!content_buf || !*content_buf || !content_len)
            return -1;

        *pp = p = odr_malloc(o, sizeof(*p));
        p->ns = soap_v1_1;

        doc = xmlParseMemory(*content_buf, *content_len);
        if (!doc)
            return z_soap_error(o, p, "SOAP-ENV:Client",
                                "Bad XML Document", 0);
        /* check that root node is Envelope */
        ptr = xmlDocGetRootElement(doc);
        if (!ptr || ptr->type != XML_ELEMENT_NODE ||
            strcmp(ptr->name, "Envelope"))
        {
            xmlFreeDoc(doc);
            return z_soap_error(o, p, "SOAP-ENV:Client",
                                "No Envelope element", 0);
        }
        else
        {
            /* determine SOAP version */
            const char * ns_envelope = ptr->ns->href;
            if (!strcmp(ns_envelope, soap_v1_1))
                p->ns = soap_v1_1;
            else if (!strcmp(ns_envelope, soap_v1_2))
                p->ns = soap_v1_2;
            else
            {
                xmlFreeDoc(doc);
                return z_soap_error(o, p, "SOAP-ENV:Client",
                                    "Bad SOAP version", 0);
            }
        }
        ptr = ptr->children;
        while(ptr && ptr->type == XML_TEXT_NODE)
            ptr = ptr->next;
        if (ptr && ptr->type == XML_ELEMENT_NODE &&
            !strcmp(ptr->ns->href, p->ns) &&
            !strcmp(ptr->name, "Header"))
        {
            ptr = ptr->next;
            while(ptr && ptr->type == XML_TEXT_NODE)
                ptr = ptr->next;
        }
        /* check that Body is present */
        if (!ptr || ptr->type != XML_ELEMENT_NODE || 
            strcmp(ptr->name, "Body"))
        {
            xmlFreeDoc(doc);
            return z_soap_error(o, p, "SOAP-ENV:Client",
                                "SOAP Body element not found", 0);
        }
        if (strcmp(ptr->ns->href, p->ns))
        {
            xmlFreeDoc(doc);
            return z_soap_error(o, p, "SOAP-ENV:Client",
                                "SOAP bad NS for Body element", 0);
        }
        pptr = ptr;
        ptr = ptr->children;
        while (ptr && ptr->type == XML_TEXT_NODE)
            ptr = ptr->next;
        if (!ptr || ptr->type != XML_ELEMENT_NODE)
        {
            xmlFreeDoc(doc);
            return z_soap_error(o, p, "SOAP-ENV:Client",
                                "SOAP No content for Body", 0);
        }
        /* check for fault package */
        if (!strcmp(ptr->ns->href, p->ns)
            && !strcmp(ptr->name, "Fault") && ptr->children)
        {
            ptr = ptr->children;

            p->which = Z_SOAP_fault;
            p->u.fault = odr_malloc(o, sizeof(*p->u.fault));
            p->u.fault->fault_code = 0;
            p->u.fault->fault_string = 0;
            p->u.fault->details = 0;
            while (ptr)
            {
                if (ptr->children && ptr->children->type == XML_TEXT_NODE)
                {
                    if (!strcmp(ptr->name, "faultcode"))
                        p->u.fault->fault_code =
                            odr_strdup(o, ptr->children->content);
                    if (!strcmp(ptr->name, "faultstring"))
                        p->u.fault->fault_string =
                            odr_strdup(o, ptr->children->content);
                    if (!strcmp(ptr->name, "details"))
                        p->u.fault->details =
                            odr_strdup(o, ptr->children->content);
                }
                ptr = ptr->next;
            }
            ret = 0;
        }
        else
        {
            for (i = 0; handlers[i].ns; i++)
                if (!strcmp(ptr->ns->href, handlers[i].ns))
                    break;
            if (handlers[i].ns)
            {
                void *handler_data = 0;
                ret = (*handlers[i].f)(o, pptr, &handler_data,
                                       handlers[i].client_data,
                                       handlers[i].ns);
                if (ret || !handler_data)
                    z_soap_error(o, p, "SOAP-ENV:Client",
                                 "SOAP Handler returned error", 0);
                else
                {
                    p->which = Z_SOAP_generic;
                    p->u.generic = odr_malloc(o, sizeof(*p->u.generic));
                    p->u.generic->no = i;
                    p->u.generic->ns = handlers[i].ns;
                    p->u.generic->p = handler_data;
                }
            }
            else
            {
                ret = z_soap_error(o, p, "SOAP-ENV:Client", 
                                   "No handler for NS", 0);
            }
        }
        xmlFreeDoc(doc);
        return ret;
    }
    else if (o->direction == ODR_ENCODE)
    {
        Z_SOAP *p = *pp;
        xmlNsPtr ns_env;
        xmlNodePtr envelope_ptr, body_ptr;
        xmlChar *buf_out;
        int len_out;

        xmlDocPtr doc = xmlNewDoc("1.0");

        envelope_ptr = xmlNewNode(0, "Envelope");
        ns_env = xmlNewNs(envelope_ptr, p->ns, "SOAP-ENV");
        body_ptr = xmlNewChild(envelope_ptr, ns_env, "Body", 0);
        xmlDocSetRootElement(doc, envelope_ptr);

        if (p->which == Z_SOAP_fault || p->which == Z_SOAP_error)
        {
            Z_SOAP_Fault *f = p->u.fault;
            xmlNodePtr fault_ptr = xmlNewChild(body_ptr, ns_env, "Fault", 0);
            xmlNewChild(fault_ptr, ns_env, "faultcode",  f->fault_code);
            xmlNewChild(fault_ptr, ns_env, "faultstring", f->fault_string);
            if (f->details)
                xmlNewChild(fault_ptr, ns_env, "details", f->details);
        }
        else if (p->which == Z_SOAP_generic)
        {
            int ret, no = p->u.generic->no;
            
            ret = (*handlers[no].f)(o, body_ptr, &p->u.generic->p,
                                    handlers[no].client_data,
                                    handlers[no].ns);
            if (ret)
                return ret;
        }
        xmlDocDumpMemory(doc, &buf_out, &len_out);
        *content_buf = odr_malloc(o, len_out);
        *content_len = len_out;
        memcpy(*content_buf, buf_out, len_out);
        xmlFree(buf_out);
        xmlFreeDoc(doc);
        return 0;
    }
    return 0;
}
