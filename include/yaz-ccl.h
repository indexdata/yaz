
#ifndef YAZ_CCL_H
#define YAZ_CCL_H

#include <proto.h>
#include <ccl.h>

Z_RPNQuery *ccl_rpn_query (struct ccl_rpn_node *p);
Z_AttributesPlusTerm *ccl_scan_query (struct ccl_rpn_node *p);

#endif
