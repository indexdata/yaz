
def cplush(names):
    l = []
    for name in names:
        l.append("src/" + name + ".c")
        l.append("include/yaz/" + name + ".h")
    return l
