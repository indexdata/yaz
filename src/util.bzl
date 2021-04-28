
def cplush(names):
    l = []
    for name in names:
        l.append(name + ".c")
        l.append("yaz/" + name + ".h")
    return l

def c_dir(dir, names):
    l = []
    for name in names:
        if dir == ".":
            l.append(name + ".c")
        else:
            l.append(dir + "/" + name + ".c")
    return l

def h_dir(dir, names):
    l = []
    for name in names:
        if dir == ".":
            l.append(name + ".h")
        else:
            l.append(dir + "/" + name + ".h")
    return l

def listToString(list, sep):
    r = ""
    for elem in list:
        if r == "":
            r = elem
        else:
            r = r + sep + elem
    return r

def man_page(name, manname, srcs):
    native.genrule(
	name = name,
	srcs = srcs,
	outs = [ manname ],
	cmd = "xsltproc --path $(RULEDIR) -o $(location " + manname + ") $(location common/id.man.xsl) $(location " + srcs[0] + ") 2>/dev/null",
        tools = [
	    "common/id.man.xsl",
	    "local.ent",
	    "entities.ent",
	    "common/common.ent"
	]
    )
