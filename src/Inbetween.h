#include <stdio.h>
#include <maya/MString.h>
#include <maya/MArglist.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>

class Inbetween : public MPxCommand
{
	public:
		MStatus doIt(const MArgList& args);
		static void* creator();
};
