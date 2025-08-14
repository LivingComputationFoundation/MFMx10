import UClass

class UClassFactory:
    def __init__(self):
        self.allUClasses = { }
        self.nextUClassIdNum = 1
        primsDefaultSizes = {
            'Int' : 32,
            'Unsigned' : 32,
            'Bool' : 1,
        }
        for pt,ds in primsDefaultSizes.items():
            puc = self.getUClass(pt)
            puc.makePrim(ds)
            
        self.urs = self.getUClass('UrSelf')
        self.urs.type = 'quark'
        
    def getExistingUClass(self,name):
        return self.allUClasses[name]

    def getUClassOrNone(self,name):
        return self.allUClasses.get(name)

    def getUClass(self,name):
        ret = self.allUClasses.get(name)
        if ret is None:
            id = self.nextUClassIdNum
            self.nextUClassIdNum += 1
            ret = UClass.UClass(name,id,self)
            self.allUClasses[name] = ret
        return ret
