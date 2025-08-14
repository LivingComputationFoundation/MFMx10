from Utils import *
import URefInfo
import ULVal

class UClass:
    def __init__(self, name, id, ucf):
        self.ucname = name
        self.uclassid = id
        self.defaultSize = None      # only for prims
        self.directBaseList = []     # list of uclass instances
        self.directDataMembers = { } # dmname => ULVal
        self.allBasePositions = { }  # ucbase => pos in this class
        self.referenceList = []  # list of URefInfo
        self.references = { }    # ?? 
        self.type = None
        self.isprim = False
        self.ucf = ucf
        self.totalDirectBits = None    # direct data member sizes only
        self.totalBaseBits = None      # all base sizes direct or indirect
        self.overallBits = None        # totalDirect + totalBase

    def __str__(self):
        return f"{self.ucname}#{self.uclassid}"
    
    def __repr__(self):
        string = f"<{self.type} {self.ucname}#{self.uclassid} ("
        sep = ""
        for db in self.directBaseList:
            string += sep+db.ucname
            sep = " "
        string += ")"
        sep = "{"
        for dm,ul in self.directDataMembers.items():
            string += sep + ul.report()
            sep = " "
        string += "}"
        string += ">"
        return string

    def generateURefs(self):
        if self.isPrim(): return
        
        for uref in self.visitAllRefs(self.uclassid,0, { }):
            eprint(f"PREGURVAR {uref}")
            uref.refnum = len(self.referenceList)
            eprint(f"PSTGURVAR {uref}")
            self.referenceList.append(uref)
            eprint(f"RFLGURVAR {self.referenceList}")
        eprint(f"UR/{self.ucname} \n  {'\n  '.join(map(str,self.referenceList))}")

    def visitClassPlus(self, pos, refnum): # here, self is the stguclass
        yield URefInfo.URefInfo(self,refnum,pos,self.overallBits,self,self.ucname+'*') # the class plus all its bases
        for bbang in self.visitAllBases({ }):
            eprint(f"VVVCCPPP10 {bbang}")
            #val = bbang.visitClassBang(self.uclassid, pos,refnum)
            #eprint(f"VVVCCPPP11 {val}")
            pos,refnum = yield from bbang.visitClassBang(self, pos, refnum)
#            eprint(bbang.visitClassBang(self.uclassid, pos,refnum))
        return pos,refnum

    def visitClassBang(self, stguclass, pos, refnum):
        #eprint(f"vCB10 {(stgclassid, pos, refnum)}")
        yield URefInfo.URefInfo(stguclass,refnum,pos,self.totalDirectBits,self,self.ucname+'!') # the class as a base slice
        #eprint(f"vCB11 {(stgclassid, pos, refnum)}")
        for dm,ulvs in self.directDataMembers.items():
            eprint(f"vCB12 {(dm,ulvs)} bef {(pos,refnum)}")
            pos, refnum = yield from ulvs.visitULVal(stguclass, pos, refnum)
            eprint(f"vCB13 {(dm,ulvs)} aft {(pos,refnum)}")
        #eprint(f"vCB14 {pos,refnum}")
        return pos, refnum
            
    def visitDirectRefs(self,stgclassid,pos,seen):
        """Visit all refs reachable just from the direct data members
        of this uclassid

        """
        for dm,ulvs in self.directDataMembers.items():
            eprint(f"DMVARBs+{pos}+{ab}")            
            yield from ulvs.visitAllRefs(stgclassid,pos,seen)
            pos += ulvs.vsize

        
    def visitAllRefs(self,stgclassid,pos,seen):
        """Visit all refs reachable just from the direct data members
        of this uclassid and any of its base classes

        """
        return self.visitClassPlus(0, 1)
        
        # yield URefInfo.URefInfo(stgclassid,None,pos,self.overallBits,self.uclassid) # the class as a whole object
        # for ab in self.visitAllBases({ }):
        #     ab.visitDirectRefs(stgclassid,pos,seen)
        #     pos += ab.totalDirectBits

        # recursively ref each direct data member
#        for dm,ul in self.directDataMembers.items():
#            string += sep + ul.report()
#            sep = " "
        pass

    def settlePositions(self):
        self.settleDataMemberPositions()
        self.settleBaseClassPositions()

    def settleDataMemberPositions(self):
        # from 0..totalDirectBits-1
        # store pos in ul.vpos
        pos = 0
        for dm,ulv in self.directDataMembers.items():
            ulv.setVPos(pos)
            pos += ulv.sizeOrNone()
        if pos != self.totalDirectBits:
            die(f"INCONS {pos}!={self.totalDirectBits}")

    def settleBaseClassPositions(self):
        # from totalDirectBits..overallBits-1
        # store in allBasePositions[basename]
        pos = 0
        for bc in self.visitAllBases({ }):  # all bases starting with self
            self.allBasePositions[bc] = pos
            pos += bc.totalDirectBits
        if pos != self.overallBits:
            die(f"INCONS {pos}!={self.overallBits}")

    def settleSizes(self):
        if self.overallBits is not None:
            return True

        if not self.settleDirectSizes() or not self.settleBaseSizes():
            return False
        
        # for dm,ulv in self.directDataMembers.items():
        #     siz = ulv.sizeOrNone()
        #     eprint(f"STLSIZDM {dm},{dumper.dumps(ulv)} = {siz}")

        self.overallBits = self.totalDirectBits + self.totalBaseBits
        eprint(f"SSVAB {self.ucname} // {dumpAlls(self.allBasePositions)}")
        #eprint(f"SVAB {self.ucname} {self.overallBits} = {self.totalDirectBits} + {self.totalBaseBits}")
        return True

    def settleDirectSizes(self):
        if self.totalDirectBits is not None:
            return True
        pending = 0
        knownSize = 0
        for dm,ulv in self.directDataMembers.items():
            siz = ulv.sizeOrNone()
            if siz is not None:
                knownSize += siz
            else:
                pending += 1

        if pending == 0:
            self.totalDirectBits = knownSize
            #eprint(f"SETSIZDMS {self.ucname} {self.totalDirectBits}")

        return self.totalDirectBits is not None

    def visitAllBases(self,seen):
        #eprint(f"ISALB {seen}")
        if seen.get(self) is None:
            seen[self] = True
            yield self
            for db in self.directBaseList:
                yield from db.visitAllBases(seen)
            
    def settleBaseSizes(self):
        if self.totalBaseBits is not None:
            return True
        pending = 0
        knownSize = 0
        for bc in self.visitAllBases({ }):
            if bc == self:      # exclude ourselves for base size calc!
                continue
            #eprint(f"sbszes {bc.ucname} {bc.totalDirectBits}")
            siz = bc.totalDirectBits
            if siz is not None:
                knownSize += siz
            else:
                pending += 1
        if pending == 0:
            self.totalBaseBits = knownSize
            return True
        return False

    def report(self):
        string = f"#{self.uclassid} {self.type} {self.ucname}"
        if self.overallBits is not None:
            string += f"({self.totalDirectBits}+{self.totalBaseBits}={self.overallBits}b)"
        string += " :"
        for db in self.directBaseList:
            string += " "+db.ucname
        for dm,ul in self.directDataMembers.items():
            string += "\n     "+ul.report()
        return string

    def isPrim(self):
        return self.isprim
    
    def makePrim(self,ds):
        if self.isprim:
            die(f"Already prim {self}")
        self.isprim = True
        self.defaultSize = ds

    def configureClass(self, classCfg):
        # Storage type
        t = classCfg.get('type','quark')
        if not t in ('quark', 'element', 'transient'):
            die(f'Unknown object {t} for {k}')
        self.type = t
        
        # Direct Base classes
        b = classCfg.get('is',None)
        if not b:
            b = ['UrSelf']
        elif not isinstance(b,(list)):
            b = [b]

        for base in b:
            if base in self.directBaseList:
                die(f"Duplicate direct base {base} in {t} {k}")
            self.directBaseList.append(self.ucf.getUClass(base))

        if len(self.directBaseList) == 0:
            self.directBaseList.append(self.ucf.getUClass('UrSelf'))
        #eprint(f"DIRECT BASES of {self.ucname}: {self.directBaseList}")

        # Direct Data members
        m = classCfg.get('has',{ })
        for (memb,mv) in m.items():
            #eprint(f'    {self.ucname} has memb {memb} is {mv}')
            if self.directDataMembers.get(memb,None):
                die(f"Duplicate direct member {memb} in {self.ucname}")
            self.directDataMembers[memb] = ULVal.ULVal(memb,mv,self.ucf)  # size and pos XXXX MAKE some value class
        eprint(f"DIRECT MEMBERS of {self.ucname}: {self.directDataMembers}")
        
