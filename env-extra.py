#pylint: disable = I,E,R,W,C,F
from os.path import isfile
Import("env")  # type: ignore
assert isfile(".env")
try:
    f = open(".env", "r")
    lines = f.readlines()
    envs = []
    for line in lines:
        line.strip()
        if line.startswith('#'):
            continue
        envs.append("-D{}".format(line.strip()))
    env.Append(BUILD_FLAGS=envs) # type: ignore
except IOError:
    print("File .env not accessible",)
finally:
    f.close()