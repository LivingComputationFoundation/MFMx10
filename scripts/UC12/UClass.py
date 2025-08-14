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
        eprint(f"//generateURefs-10 {self}")
        if self.isPrim(): return
        
        eprint(f"//generateURefs-11")
        for uref in self.visitAllRefs(self,0, { }):
            #eprint(f"//generateURefs-12 {self} {uref}")
            uref.refnum = len(self.referenceList)
            #eprint(f"//generateURefs-13 {self} {uref}")
            self.referenceList.append(uref)
            eprint(f"//generateURefs-14 {self} {self.referenceList}")
        eprint(f"//generateURefs-299 {self}")
        eprint(f"UR/{self.ucname} \n  {'\n  '.join(map(str,self.referenceList))}")

    def visitClassPlus(self, stguclass, pos, refnum, oname): # here, self is the membclass
        eprint(f"  //visitClassPlus-10 {self}")
        yield URefInfo.URefInfo(stguclass, refnum,
                                pos, self.overallBits,
                                self, self, # self is membclass and effself both, here?
                                oname+'.'+self.ucname+'*') # the class plus all its bases
        eprint(f"  //visitClassPlus-11 {self}")
        for bbang in self.visitAllBases({ }):
            eprint(f"  //visitClassPlus-12 {bbang}")
            #val = bbang.visitClassBang(self.uclassid, pos,refnum)
            #eprint(f"VVVCCPPP11 {val}")
            pos,refnum = yield from bbang.visitClassBang(stguclass, self, pos, refnum)
            eprint(f"  //visitClassPlus-13 {pos,refnum}")
#            eprint(bbang.visitClassBang(self.uclassid, pos,refnum))
        eprint(f"  //visitClassPlus-299 {pos,refnum}")
        return pos,refnum

    def visitClassBang(self, stguclass, membclass, pos, refnum):
        eprint(f"  //visitClassBang-10 {self} stg:{stguclass} mbc: {membclass} @{pos} #{refnum}")
        #eprint(f"vCB10 {(stgclassid, pos, refnum)}")
        yield URefInfo.URefInfo(stguclass, refnum,
                                pos, self.totalDirectBits,
                                membclass, self, # XXX ?
                                self.ucname+'!') # the class as a base slice
        eprint(f"  //visitClassBang-11 {self} stg:{stguclass} mbc:{membclass} +{pos} #{refnum}")
        for dm,ulvs in self.directDataMembers.items():
            eprint(f"  //visitClassBang-12 {(dm,ulvs)} bef {(pos,refnum)}")
            pos, refnum = yield from ulvs.visitULVal(stguclass, ulvs.uc, pos, refnum)
            eprint(f"  //visitClassBang-13 {(dm,ulvs)} aft {(pos,refnum)}")
        eprint(f"  //visitClassBang-14 {pos,refnum}")
        return pos, refnum
            
    # def visitDirectRefs(self,stgclass,pos,seen):
    #     """Visit all refs reachable just from the direct data members
    #     of this uclassid

    #     """
    #     eprint(f"//visitDirectRefs-10 ({self}, {stgclass}, {pos}, {seen}):")
    #     for dm,ulvs in self.directDataMembers.items():
    #         yield from ulvs.visitAllRefs(stgclass,pos,seen)
    #         pos += ulvs.vsize

        
    def visitAllRefs(self,stgclass,pos,seen):
        """Visit all refs reachable just from the direct data members
        of this uclassid and any of its base classes

        """
        return self.visitClassPlus(stgclass, 0, 0, self.ucname)
        
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

    def cppMangledName(self,pref):
        return cppMangler(f"UClass{pref}_{self.ucname}")

    def getDirectBaseInfo(self):
        dbcount = len(self.directBaseList)
        dbdef = "     0u /* no bases */" if dbcount == 0 else f"""{
'\n'.join(map(lambda x : f"     {x.uclassid}u, /* {x.ucname} */",self.directBaseList))}"""
        return (dbcount,dbdef)
    
    def getAllBaseInfo(self):
        abcount = len(self.allBasePositions)
        abdef = ""
        for ucb in self.allBasePositions:
            abdef += f"   {ucb.uclassid}u,  /* {ucb.ucname} */\n"
        return (abcount,abdef)

    def getAllURefInfo(self):
        urcount = len(self.referenceList)
        if urcount == 0:
            return  (0,"{ 0 }")
        urdef = ""
        for ur in self.referenceList:
            urdef += f"  {{ {ur.stguclass.uclassid}, {ur.membclass.uclassid}, {ur.effself.uclassid}, {ur.pos}, {ur.size} }}, // {ur}\n"
        return (urcount,urdef)

    def generateUClassData(self,dbcount,dbdefname):
        print("")
        print(f"/** {self.report()} */")
        (abcount,abdef) = self.getAllBaseInfo()
        (urcount,allurefs) = self.getAllURefInfo()
        abname = self.cppMangledName("AllBases")
        urname = self.cppMangledName("AllURefs")
        print(f"""  static const u16 {abname}[] = {{
{abdef}  }};
  static const URefDescriptor {urname}[] = {{
{allurefs}  }};
  static UClass {self.cppMangledName("")}(
    /*uclassid*/  {self.uclassid}u,
    /*ucname*/    "{self.ucname}",
    /*dirbases*/  {dbdefname},
    /*dbcount*/   {dbcount},
    /*allbases*/  {abname},
    /*abcount*/   {abcount},
    /*alluref*/   {urname},
    /*urefcount*/ {urcount}
        );
 """)

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
        dmindex = 0
        for (memb,mv) in m.items():
            eprint(f'    {self.ucname} has memb {memb} is {mv}')
            if self.directDataMembers.get(memb,None):
                die(f"Duplicate direct member {memb} in {self.ucname}")
            self.directDataMembers[memb] = ULVal.ULVal(memb,dmindex,self.ucf,mv)  # size and pos XXXX MAKE some value class
            dmindex += 1

        eprint(f"DIRECT MEMBERS of {self.ucname}: {self.directDataMembers}")
        
