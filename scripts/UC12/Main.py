#!/usr/bin/env python3
import os

from Utils import *

import Config
import UClass
import UClassFactory
import UStgFactory

import pdb

class Main:
    def __init__(self,codeFile):
        self.codeFile = codeFile
        self.configPath = self.findConfigOrDie(codeFile)
        self.codeCfg = self.loadConfig()
        self.ucf = UClassFactory.UClassFactory()
        self.ustgf = UStgFactory.UStgFactory()
        eprint(f"__main__ {self.codeFile}")

    def process(self):
        self.declareAllUClasses()
        #self.reportAllUClasses()
        #eprint(f">>UCF2 {dumpAlls(self.ucf)}")
        self.settleSizes()
        self.settlePositions()
        self.generateURefs()

    def generateURefs(self):
        for un,uc in self.ucf.allUClasses.items():
            eprint(f"MAIN/genuref:{un}")
#            breakpoint()
            uc.generateURefs()

    def settleSizes(self):
        loops = 0
        while loops < 100:
            loops += 1
            alldone = True
            for un,uc in self.ucf.allUClasses.items():
                alldone &= uc.settleSizes()
            if alldone:
                break
        if loops >= 100:
            die(f"{loops} TIMES UNSETTLING")

    def settlePositions(self):
        for un,uc in self.ucf.allUClasses.items():
            uc.settlePositions()

    def declareAllUClasses(self):
        ccfg = self.codeCfg
        cfgclasses = ccfg.getRequiredSection('Class')
        for k,v in cfgclasses.items():
            uc = self.ucf.getUClass(k)
            uc.configureClass(v)
            #dumper.dump(uc)

    def reportAllUClasses(self):
        for un,uc in self.ucf.allUClasses.items():
            eprint(f"{uc.report()}")

    def generateUClassLibraryData(self):
        self.ucf.generateUClassCodeTables()

    def findConfigOrDie(self,optPath):
        paths = []
        if optPath != None:
            paths.append(optPath)
        for p in paths:
            (exists,path) = self.expandPath(p)
            if exists:
                eprint(f"Found config file {path}")
                return path
        self.die(f"Config file not found in {paths}")

    def expandPath(self,path):
        epath = os.path.expanduser(path)
        if os.path.exists(epath):
            return (True,epath)
        return (False,None)

    def die(self,message):
        eprint(f"Error: {message}")
        exit(2)

    def loadConfig(self):
        path = self.configPath
        if os.path.isfile(path):
            ret = Config.Config("Cfg",path)
        else:
            self.die(f"Can't load {path}")
        ret.load()
        return ret

if __name__ == '__main__':

    import sys
    argc = len(sys.argv)
    if argc <= 1 or argc > 2:
        sys.exit("Usage: " + __file__ + " UCDTOML")
    codeFile = sys.argv[1]

    print("// RUNNING",__file__ ,"\n// ON",codeFile,"---")
    mr = Main(codeFile)
    mr.process()
    eprint("PREFINALCLAMS")
    mr.reportAllUClasses()
    eprint("FINALCLAMS")
    #dumper.dump(mr)
    eprint(dumpAlls(mr))
    mr.generateUClassLibraryData()
    #eprint("FINALOFCLAMS")
    #dumper.dump(mr.codeCfg)

