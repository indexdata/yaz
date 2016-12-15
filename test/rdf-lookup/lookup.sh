# Simple test script to invoke rdf-lookup
# Can not be run as part of 'make check' as it 
# depends on external resources (id.loc.gov)

# Should output some bf:Agent tags with rdf:about attrs
# that point to id.loc.gov

../../util/yaz-record-conv lookup-conf.xml bf.xml > x.xml
echo
grep -C3 Agent x.xml
