#!/usr/bin/env python3
import os
import sys

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

import hashlib

import tomlikey as tomli

import dumper
dumper.max_depth = 10

class Config:
    def __init__(self, name, path):
        self.name = name
        self.path = path
        self.reset()

    def reset(self):
        self.hash = None
        self.rawfile = None
        self.rawfileCS = None

    def load(self):
        # Read whole file for storage
        with open(self.path,"rb") as file:
            self.rawfile = file.read()
        eprint("CONFIGLEN =",len(self.rawfile),"FOR",self.path)

        # Save its hash for checking
        h = hashlib.sha256()
        h.update(self.rawfile)
        self.rawfileCS = h.digest()
        
        # Parse it using dtomlib
        self.hash = tomli.loads(self.rawfile.decode())

    def getRequiredSection(self,name):
        assert name in self.hash, f"Unknown section {name}"
        return self.hash[name]
        
    def getOptionalSection(self,name):
        return self.hash.get(name, None)

    def getInitializedSection(self,name,value):
        have = self.getOptionalSection(name)
        if not have:
            self.hash[name] = value
        return self.getRequiredSection(name)

class Main:
    def __init__(self,name,codeFile):
        self.name = name
        eprint("__main__",self.name)
        self.configPath = self.findConfigOrDie(codeFile)
        self.codeCfg = self.loadConfig()

    def findConfigOrDie(self,optPath):
        paths = []
        if optPath != None:
            paths.append(optPath)
        # e = os.environ
        # ekey ='BEEVEESOURCESPATH'
        # if ekey in e:           # First try environmental variable
        #     paths.append(e[ekey])
        # paths.append('../code/BVBrain.cfg') # Then 'std loc'
        # paths.append('./BVBrain.cfg')       # Last ditch
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
            ret = Config("Cfg",path)
        else:
            self.die(f"Can't load {path}")
        ret.load()
        return ret

    def defineAllClasses(self):
        ccfg = self.codeCfg
        cfgclasses = ccfg.getRequiredSection('Class')
        self.classes = { }
        for k,v in cfgclasses.items():
            #eprint(f'STUFFJDKDKD {k}:{v}')
            t = v.get('type','quark')
            if not t in ('quark', 'element', 'transient'):
                self.die(f'Unknown object {t} for {k}')
            if self.classes.get(k,None):
                self.die(f"Duplicate {t} class {k} (this can't happen)")
            self.classes[k] = {
                'name' : k,
                'type' : t,
                'bitsize' : None, 
                'directBases' : { },
                'directMembers' : { },
                'allBases' : [ [], {} ],
                'allRefs' : [ [], {} ]   # member name -> [refnum, pos, siz, effself ] ?
            }
            c = self.classes[k]
            #eprint(f"  TDTHG {t} {k}--{c}")
            b = v.get('is',None)
            if not b:
                b = []
            elif not isinstance(b,(list)):
                b = [b]

    def declareDirectMembers(self):
        ccfg = self.codeCfg
        cfgclasses = ccfg.getRequiredSection('Class')
        for k,v in cfgclasses.items():
            eprint(f'DeDiMe {k}:{v}')
            c = self.classes[k]
            m = v.get('has',{ })
            dm = c['directMembers']
            for (memb,mv) in m.items():
                eprint(f'    {k} has memb {memb} is {mv}')
                if dm.get(memb,None):
                    self.die(f"Duplicate direct member {memb} in {k}")
                dm[memb] = [[mv], -1, -1]  # size and pos
            eprint(f"DIRECT MEMBERS of {k}: {dm}")
        
    primsDefaultSizes = { # Never 0..
        'Int' : 32,
        'Unsigned' : 32,
        'Bool' : 1,
    }

    def sizeIfKnown(self,mv,c):
        eprint(f"SIKK {mv} :: {c}")
        ctype = mv[0][0]
        classType = self.classes.get(ctype,None)
        if classType:
            classSize = classType['bitsize']
            eprint(f"SIKCT {ctype} // {classType}  {classSize}")
            if type(classSize) is int:
                size = classSize
            else: size = -1
        else:
            defaultSize = self.primsDefaultSizes.get(ctype,None)
            eprint(f"SIKPT {ctype} // {defaultSize}")
            if defaultSize:     # here means 'is a prim'
                if len(mv[0]) > 1: # has explicit size
                    size = mv[0][1]
                else:
                    size = defaultSize                
            else:               # Not a prim and not a class
                self.die(f"Unrecognized data member type '{ctype}' in {c}")
        eprint(f"SIKRT {mv} is {size} in {c}")
        return size

    def computeDirectMembers(self):
        totalPending = 0
        pendingClasses = []
        for k,c in self.classes.items():
            pending = 0
            knownSize = 0
            dms = c['directMembers']
            eprint(f'CDM {k}:{dms}')
            knownSize = 0
            for m,mv in dms.items():
                mspec = mv[0]
                bsize = mv[1]
                eprint(f'CDMMMM {m} // {mv}')
                if bsize >= 0:   # already have size
                    knownSize = knownSize + bsize # count it in
                else:                               
                    mv[1] = self.sizeIfKnown(mv[0],c) # try updating (might still be < 0)
                    eprint(f'NAILED? {k}: {m} ===== {mv} in {dms[m]}')
                    pending = pending + 1
                    pendingClasses.append(mv[0][0])
            if pending == 0:
                c['bitsize'] = knownSize
                eprint(f'CDMS {k} is exactly {c['bitsize']} direct data member bits')
            else:
                totalPending = totalPending + pending
                eprint(f'CDMS {k} is at least {knownSize} direct data member bits with {pending} pending')
        return (totalPending, pendingClasses)
    
    def computeDirectBases(self):
        ccfg = self.codeCfg
        cfgclasses = ccfg.getRequiredSection('Class')
        for k,v in cfgclasses.items():
            eprint(f'CDB {k}:{v}')
            c = self.classes[k]
            t = c['type']
            b = v.get('is',None)
            if not b:
                b = ['UrSelf']
            elif not isinstance(b,(list)):
                b = [b]
            eprint(f"  CDB2 {t} {k}--{c} !!!{b}")

            db = c['directBases']
            for base in b:
                eprint(f'    MDKBASE {t} {k} is {base}')
                if db.get(base,None):
                    self.die(f"Duplicate direct base {base} in {t} {k}")
                db[base] = []
            if not db.get('UrSelf',None):
                db['UrSelf'] = []

            eprint(f"DIRECT BASES of {k}: {db}")

    def addAllBases(self,v,c):
        (abl,abh) = v['allBases']
        eprint(f'aab10{(abl,abh)}')
        if type(abh.get(c,None)) is int:     # if already added
            return              # done
        abh[c] = len(abl)
        abl.append(c)
        eprint(f'aab11{(abl,abh)}')
        for db,l in v['directBases'].items():
            self.addAllBases(v,db) # else recurse

    def computeAllBases(self):
        for k,v in self.classes.items():
            self.addAllBases(v,k)  
            eprint(f'ABS {k}:{v}')

    def addRef(self,ar,name,pos,siz,effself):
        refnum = len(ar[0])
        #siz = ar[1]
        refval = [refnum,pos,siz,effself]
        if ar[1].get(name,None):
            eprint(f'NAME ALREADY PRESENT, SKIPPING "{name}"')
        else:
            ar[0].append(name)
            ar[1][name] = refval
            eprint(f'ADRF #{refnum}/{refval} "{name}"')
            pos = pos + siz
        return pos

    def isPrimMember(self,mv):
        ctype = mv[0][0][0]
        if self.primsDefaultSizes.get(ctype,None):
            ret = True
        else: ret = False
        eprint(f'ISPM #{mv} : {ctype} -> {ret}')
        return ret

    def addMemberRefs(self,pos,c,m,mv,effself):
        eprint(f'AMR {pos}:{m}:{mv}:{c}')
        if self.isPrimMember(mv):
            pos = self.addRef(c['allRefs'],m,pos,mv[1],effself)
        else:
            pos = self.addRef(c['allRefs'],m,pos,mv[1],effself)
        return pos

    def getMemberInfo(self,cn,mn):
        c = self.classes[cn]
        ar = c['allRefs']
        membs = ar[1]
        #eprint(f"GMBI {cn} {mn} {membs}")
        mi = membs[mn]
        return mi

    def addAllBaseMemberRefs(self,pos,cn,c,b):
        siz = self.getSizeOfClass(b)
        eprint(f"AABMR! {cn} {b} {pos} {siz}")
        self.addRef(c['allRefs'],b,pos,siz,cn)
        dms = c['directMembers']
        for m,mv in dms.items():
            eprint(f'ADLRF {pos} cn:{cn} mv:{mv} mv0:{mv[0]}')
            pos = self.addMemberRefs(pos,c,m,mv,mv[0][0][0])
        return pos
        
    def addAllRefs(self,pos,v,cn):
        c = self.classes[cn]
        abs = c['allBases'][0]
        eprint(f'ADALLREFS {cn} : {abs} {pos}')

        for b in abs:
            pos = self.addAllBaseMemberRefs(pos,cn,c,b)
            eprint(f'ADLRFB {cn} : {b} {pos}')
        return pos

    def computeAllRefs(self):
        for k,v in self.classes.items():
            self.addAllRefs(0,v,k)  

    def getSizeOfClass(self,cname):
        c = self.classes[cname]
        bs = c['bitsize']
        if bs < 0: self.die(f"No size for class {cname} : {c}")
        return bs

    def positionDirectsFor(self,cname,c):
        nextbitpos = 0
        dms = c['directMembers']
        for m,mv in dms.items():
            msize = mv[1]
            if msize < 0 or mv[2] >= 0:
                self.die(f"can't happen {msize} .. {mv}")
            mv[2] = nextbitpos
            nextbitpos = nextbitpos + msize
        dbs = c['directBases']
        for bname,bv in dbs.items():
            eprint(f'PDF {bname}:{bv}:{nextbitpos}')
            bv.append(nextbitpos)
            bsize = self.getSizeOfClass(bname)
            nextbitpos = nextbitpos + bsize

    def positionAllDirects(self):
        for k,v in self.classes.items():
            self.positionDirectsFor(k,v)

    def sizeAllDirects(self):
        for k,v in self.classes.items():
            self.sizeDirectsFor(k,v)

    def sizeDirectsFor(self,cname,c):
        dms = c['directMembers']
        for m,mv in dms.items():
            eprint(f"SIDIFO {cname} : {mv}")
        #     msize = mv[1]
        #     if msize < 0 or mv[2] >= 0:
        #         self.die(f"can't happen {msize} .. {mv}")
        #     mv[2] = nextbitpos
        #     nextbitpos = nextbitpos + msize
        # dbs = c['directBases']
        # for bname,bv in dbs.items():
        #     eprint(f'PDF {bname}:{bv}:{nextbitpos}')
        #     bv.append(nextbitpos)
        #     bsize = self.getSizeOfClass(bname)
        #     nextbitpos = nextbitpos + bsize

    def settleSizes(self):
        pend = 99999999
        lastpend = 99999999
        limit = 100
        for loop in range(1,limit):
            last2pend = lastpend
            lastpend = pend
            (pend, classes) = self.computeDirectMembers()
            eprint(f"#{loop} Pending Classes: {pend}")
            if pend == 0 or pend == last2pend:
                break
        if pend != 0:
            self.die(f"{pend} unresolved pending class(es): {classes}")

    def process(self):
        self.defineAllClasses()
        self.declareDirectMembers()
        self.computeDirectBases()
        self.computeAllBases()
        self.settleSizes()
        self.sizeAllDirects()
        self.positionAllDirects()
        self.computeAllRefs()
        eprint("DSFOIFSDIO")
        dumper.dump(self)

    def reportClass(self,cls):
        cn = cls['name']
        eprint(f"| {cls['type']} {cn}")
        for mb in cls['directMembers']:
            mi = self.getMemberInfo(cn,mb)
            #eprint(f"|  {mb}: {mi}")
            eprint(f"|   {mb}: rn={mi[0]} pos={mi[1]} siz={mi[2]}b eff={mi[3]}")
        abl,abh = cls['allBases']
        #eprint(f"ALBS{abl}")
        for bcn in abl:
            rn = abh[bcn]
            #eprint(f"| ** {bcn}: pos={cn,bcn}")
            bci = self.getMemberInfo(cn,bcn)
            eprint(f"|  {bcn}: rn={bci[0]} pos={bci[1]} siz={bci[2]}")

    def report(self):
        eprint("+--------------  ")
        # 1: Stats on class count etc
        classCount = len(self.classes)
        eprint(f"| Classes defined {classCount}")

        # 2: alpha table of class reports
        for k in sorted(self.classes.keys()):
            eprint(f"| ")
            cls = self.classes[k]
            self.reportClass(cls)
            

if __name__ == '__main__':

    import sys
    argc = len(sys.argv)
    if argc <= 1 or argc > 2:
        sys.exit("Usage: " + __file__ + " UCDTOML")
    codeFile = sys.argv[1]

    print("// RUNNING",__file__ ,"\n// ON",codeFile,"---")
    mr = Main("ZONGProd",codeFile)
    mr.process()
    eprint("FINALCLAMS")
    mr.report()
