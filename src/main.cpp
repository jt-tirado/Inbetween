#include <maya\MSimple.h>
#include <maya\MGlobal.h>

DeclareSimpleCommand(Inbetween, "Justin Tirado", "1.0")

MStatus Inbetween::doIt(const MArgList& args) {
    MGlobal::displayInfo("Inbetween!");
    return MS::kSuccess;
}