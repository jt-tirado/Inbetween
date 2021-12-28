#include <maya/MSimple.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>

#include "Inbetween.h"

MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Justin Tirado", "1.0", "Any");
    plugin.registerCommand("inbetween", Inbetween::creator);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);
    plugin.deregisterCommand("inbetween");
    return MS::kSuccess;
}