from enum import Enum

class UStgType(Enum):
    EW = 0
    AREG = 1
    TEMP = 2

class UStg:
    def __init__(self, stgtype, stgnum):
        self.stgnum = stgnum
        self.type = stgtype
        self.classid = -1
        
