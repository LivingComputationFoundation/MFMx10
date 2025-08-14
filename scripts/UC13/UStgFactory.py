from Utils import *
from UStg import *

import dumper

class UStgFactory:
    def __init__(self):
        self.ustgList = []
        for i in range(41): self.addUStg(UStgType.EW)
        for i in range(10): self.addUStg(UStgType.AREG)

    def addUStg(self,type):
        id = len(self.ustgList)
        if id >= 256: die("Too many UStg") 
        us = UStg(type,id)
        self.ustgList.append(us)

    def getUStg(self,id):
        return self.ustgList[id]
