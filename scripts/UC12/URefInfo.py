import UClass

class URefInfo:
    def __init__(self, stguclass, refnum, pos, size, membclass, effself, fromname):
        if not isinstance(stguclass,UClass.UClass):
            raise Exception
        self.stguclass = stguclass    # uclass of 'full object' at some (unspecified) stgnum
        self.refnum = refnum          # index of this urefinfo in uclass.referenceList
        self.pos = pos                # bitpos of value within full object
        self.size = size              # bitsize of value within full object
        self.membclass = membclass    # uclass of 'full object' of member
        self.effself = effself        # uclass for virtual method lookups
        self.fromname = fromname

    def __repr__(self):
        return f"(URI #{self.refnum} <{self.stguclass}> @{self.pos} +{self.size} mc:{self.membclass} effs:{self.effself} '{self.fromname}')"

