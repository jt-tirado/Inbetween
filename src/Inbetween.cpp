#include "Inbetween.h"

MStatus Inbetween::doIt(const MArgList& args) {
    cout << "Hello World " << args.asString(0).asChar() << endl;
    return MS::kSuccess;
}

void* Inbetween::creator() {
    return new Inbetween;
}