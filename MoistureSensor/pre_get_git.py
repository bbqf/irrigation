Import("env")

head = open("../.git/refs/heads/main", "r")
rev = head.read(7)
head.close()

env.Append(CXXFLAGS=["-DREVISION="+rev])
