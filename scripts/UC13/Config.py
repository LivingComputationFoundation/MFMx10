from Utils import *

import hashlib
import tomlikey as tomli

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
