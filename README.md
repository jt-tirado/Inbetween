# Inbetween
Maya 2022 Plug-in

Inbetween command is similar to a tool called Tween Machine but 10x faster. Basically, if you have an an animated object, create a key that favors the previous or the next keyframe values.

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