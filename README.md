# Inbetween
Maya 2022 Plug-in

Inbetween is a simple maya command similar to a tool called Tween Machine but 10x faster. Basically, if you have an animated object, create a key that favors the previous or the next keyframe values.

Originally created with the intention of getting comfortable with the inner workings of the Maya API, and building fast and efficient animation tools in Maya.

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