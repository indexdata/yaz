
def cplush(names):
    l = []
    for name in names:
        l.append("src/" + name + ".c")
        l.append("include/yaz/" + name + ".h")
    return l

def c_dir(dir, names):
    l = []
    for name in names:
        l.append(dir + "/" + name + ".c")
    return l

def h_dir(dir, names):
    l = []
    for name in names:
        l.append(dir + "/" + name + ".h")
    return l
