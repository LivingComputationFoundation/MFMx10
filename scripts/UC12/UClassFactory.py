from Utils import *
import UClass
import time

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

    def generateUClassCodeTables(self):
        self.generateCommonHeader()

        # Generate direct base tables
        dbc = { }  # uc : (count,info)
        dbmap = { }  # (count,info) : index
        for un,uc in self.allUClasses.items():
            tup = uc.getDirectBaseInfo()
            if tup not in dbmap:
                dbmap[tup] = len(dbmap)
            dbc[uc] = tup

        for ((dbcount,dbdef),idx) in dbmap.items():
            dbname = cppMangler(f"UnifiedDirectBases_{idx}") 
            print(f"""  static const u16 {dbname}[] = {{
{dbdef} }};""")

        for un,uc in self.allUClasses.items():
            (dbcount,dbdef) = dbc[uc]
            idx = dbmap[(dbcount,dbdef)]
            dbname = cppMangler(f"UnifiedDirectBases_{idx}") 
            uc.generateUClassData(dbcount,dbname)
            
        self.generateCommonFooter()

    def generateCommonHeader(self):
        print(f"""
// HARO HARO IT'S {time.ctime()}

  const u32 UCLG_CLASS_COUNT = {len(self.allUClasses.items())+1}u;
        
 """)

    def generateCommonFooter(self):
        print(f"""
static UClass * UCLG_ALL_UCLASSES[] = {{
   0,    // #0 ILLEGAL RESERVED
""")
        for un,uc in self.allUClasses.items():
            print(f"   &{uc.cppMangledName("")},    // #{uc.uclassid} {un}")
        print(f"""
  }};
 """)
        print(f"""
  UClassLibrary::UClassLibrary()
    : mEW(0)
    , mUClassCount(UCLG_CLASS_COUNT)
    , mAllUClasses(UCLG_ALL_UCLASSES)
  {{ }}
        """)
        print(f"""
// BAHBAHWIHDUNAT {time.ctime()}
// THATSALLFOKES
        """)
        
