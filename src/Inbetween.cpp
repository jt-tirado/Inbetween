#include "Inbetween.h"

const MString Inbetween::name = "inbetween";

void* Inbetween::creator() {
    return new Inbetween;
}


MStatus Inbetween::doIt(const MArgList& args) {
    //cout << "Inbewteen!" << endl;
    //MStreamUtils::stdOutStream() << "Inbetween!" << "\n";
    MGlobal::displayInfo("Inbetween!");
    return MS::kSuccess;
}