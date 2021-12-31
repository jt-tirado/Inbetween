# Inbetween
Maya 2022 Plug-in

# Usage
**Mel**
```
inbetween -weight 0.5 -force 1 pCube1;
```

**Python**
```
import maya.cmds as cmds
cmds.inbetween("pCube1", weight=0.5, force=True)
```