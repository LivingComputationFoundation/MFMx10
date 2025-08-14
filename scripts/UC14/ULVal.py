from Utils import *
import URefInfo

class ULVal:

    def __init__(self, dmname, dmindex, ucf, cfg):
        self.ucf = ucf
        self.uc = None          # UClass instance 
        self.uvname = dmname    # data member name
        self.uvindex = dmindex  # data member index in direct members
        self.vtype = None
        self.vsize = None       # bitsize IF PRIM?
        self.vpos = None
        self.isprim = False
        self.configureVal(*cfg)

    def visitULVal(self, stguclass, membclass, pos, parenturi, dmidx):
        eprint(f"NEEDYIELDvULV {dumper.dumps(self)} {pos} {parenturi} {dmidx}")
        siz = self.sizeOrNone()
        if siz is None: die(f"NEEDED SIZ BY NOW: {self}")
        if self.isprim:
            eprint(f"XXXXGOTCALLED {self}")
            uri = URefInfo.URefInfo(stguclass,
                                    pos, siz,
                                    membclass,
                                    membclass,
                                    self.uvname,
                                    parenturi, dmidx) # just the prim
        else:
            # Want to visit the whole object of the type of THIS ULVAL, not the prevailing membclass!
            uri = self.uc.visitClassPlus(stguclass, pos, self.uvname, parenturi, dmidx) # a whole instance of the class
        return uri

    def setVPos(self,pos):
        if self.vpos is not None:
            die("ALREADY POS")
        self.vpos = pos

    def __repr__(self):
        return f"<ULVal:{self.vtype}{'*' if self.isprim else ''} @{self.vpos} +{self.vsize}b {self.uvname}>"

    def sizeOrNone(self):
        eprint(f"//sizeOrNone-10 {self} {self.vsize}")
        if self.isprim:
            return self.vsize
        eprint(f"//sizeOrNone-11 {self} {self.uc.overallBits}")
        if self.uc.overallBits is not None: # data members have their own base class instances
            return self.uc.overallBits      # so overallBits, not totalDirectBits
        eprint(f"//sizeOrNone-12 {self}")
        return None

    def report(self):
        return f"{self.vtype}({self.vsize}@{self.vpos}) {self.uvname}"

    def configureVal(self,strtype,strsize=None):
        self.uc = self.ucf.getUClass(strtype)
        self.isprim = self.uc.isPrim()
        if self.isprim: # isPrim
            self.vtype = strtype    # prim type XXX enum? Int32 vs Int3 vs ?
            if strsize is not None:
                self.vsize = int(strsize)
            else:
                eprint(f"DEFALTO {self.uc.defaultSize} {strtype} {strsize} // {self.vtype}:{self.vsize}")
                self.vsize = self.uc.defaultSize
                if self.vsize is None:
                    die(f"NO DEFSIZE FOR {self} in {self.uc}")
            eprint(f"PRIMBO {self.uc} {strtype} {strsize} // {self.vtype}:{self.vsize}")
            #die("TRACKED")
        else:
            uc = self.ucf.getUClass(strtype)
            self.vtype = uc.ucname
        #die(f"UV({strtype},{strsize})")

    def sizeIfKnown(self,vtype,explicitsize):
        eprint(f"SIKK {mv} :: {c}")
        classType = self.ucf.getUClassOrNone(vtype)
        if classType:
            classSize = classType['bitsize']
            eprint(f"SIKCT {vtype} // {classType}  {classSize}")
            if type(classSize) is int:
                size = classSize
            else: size = -1
        else:
            defaultSize = self.primsDefaultSizes.get(vtype,None)
            eprint(f"SIKPT {vtype} // {defaultSize}")
            if defaultSize is not None:     # here means 'is a prim'
                if strsize is not None: # has explicit size
                    size = strsize
                else:
                    size = defaultSize                
            else:               # Not a prim and not a class
                die(f"Unrecognized data member type '{vtype}' in {self}")
        eprint(f"SIKRT {vtype} is {size} in {self}")
        return size

