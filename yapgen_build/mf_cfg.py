
import os;

C_YAPGEN_DIR = os.sep.join(os.getcwd().split(os.sep)[:-1] + ["yapgen"])

index = 0
C_YAPGEN_PROG = index; index += 1

index = 0
CFG_NAME   = index; index += 1
CFG_TARGET = index; index += 1
CFG_DIR    = index; index += 1
CFG_MODULE = index; index += 1

c_cfg = [
    ["YAPGEN",True,C_YAPGEN_DIR,None],
]

