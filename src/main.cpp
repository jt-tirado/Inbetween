#include <maya/MFnPlugin.h>

#include "Inbetween.h"

MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Justin Tirado", "1.0", "Any");
    plugin.registerCommand(Inbetween::name, Inbetween::creator);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);
    plugin.deregisterCommand(Inbetween::name);
    return MS::kSuccess;
}