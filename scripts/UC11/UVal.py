from Utils import *
import URefInfo

class UVal:
    primsDefaultSizes = {
        'Int' : 32,
        'Unsigned' : 32,
        'Bool' : 1,
    }

    def __init__(self, name, cfg, ucf):
        self.uvname = name
        self.ucf = ucf
        self.vtype = None
        self.vsize = None
        self.configureVal(*cfg)

    def configureVal(self,strtype,strsize=None):
        defaultSize = self.primsDefaultSizes.get(strtype,None)  # != 0 -> isPrim
        if defaultSize is not None:
            self.vtype = vtype
            if strsize is not None:
                self.vsize = int(strsize)
            else:
                self.vsize = defaultSize
        else:
            uc = self.ucf.getUClass(strtype)

        die(f"UV({strtype},{strsize})")

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

