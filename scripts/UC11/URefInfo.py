class URefInfo:
    def __init__(self, stguclass, refnum, pos, size, effself, fromname):
        self.stguclass = stguclass    # uclass of 'full object' at some (unspecified) stgnum
        self.refnum = refnum          # index of this urefinfo in uclass.referenceList
        self.pos = pos                # bitpos of value within full object
        self.size = size              # bitsize of value within full object
        self.effself = effself        # uclass for virtual method lookups
        self.fromname = fromname

    def __repr__(self):
        return f"(URI #{self.refnum} @{self.pos} +{self.size} {self.stguclass}/{self.effself} '{self.fromname}')"

