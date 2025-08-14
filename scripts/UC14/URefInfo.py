import UClass

class URefInfo:
    def __init__(self, stguclass, pos, size, membclass, effself, fromname, parenturi, selectidx):
        if not isinstance(stguclass,UClass.UClass):
            raise Exception
        self.stguclass = stguclass    # uclass of 'full object' at some (unspecified) stgnum
        self.refnum = len(stguclass.referenceList) # index of this urefinfo in uclass.referenceList
        stguclass.referenceList.append(self)       # stick us in there
        self.pos = pos                # bitpos of value within full object
        self.size = size              # bitsize of value within full object
        self.membclass = membclass    # uclass of 'full object' of member
        self.effself = effself        # uclass for virtual method lookups
        self.fromname = fromname      # debugging-ish name for URI
        self.selectURIs = { }         # URI#s of base classes or data members, by bc/dm index
        if isinstance(parenturi,URefInfo):
            parenturi.selectURIs[selectidx] = self

    def summarizeSelectURIs(self):
        ret = ""
        for k,v in self.selectURIs.items():
            ret += f"[{k}]->#{v.refnum} "
        return ret

    def addSelectorURI(self, fromIdx, fromURI):
        if self.selectURIs.get(fromIdx) != None:
            raise Exception("DUPE")
        self.selectURIs[fromIdx] = fromURI;

    def __repr__(self):
        return f"(URI#{self.refnum} <{self.stguclass}> @{self.pos} +{self.size} mc:{self.membclass} effs:{self.effself} '{self.fromname} ::: {self.summarizeSelectURIs()}')"

