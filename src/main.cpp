#include <maya/MFnPlugin.h>

#include "InbetweenCmd.h"

MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Justin Tirado", "1.0", "Any");

    MStatus status;
    status = plugin.registerCommand(InbetweenCmd::name(), InbetweenCmd::creator, InbetweenCmd::newSyntax);

    if (!status)
    {
        status.perror("Failed to register command.");
        return status;
    }
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    MStatus status;
    status = plugin.deregisterCommand(InbetweenCmd::name());
    
    if (!status)
    {
        status.perror("Failed to unregister command.");
        return status;
    }

    return MS::kSuccess;
}