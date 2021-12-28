#include <stdio.h>
#include <maya/MString.h>
#include <maya/MArglist.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MStreamUtils.h>

class Inbetween : public MPxCommand
{
	public:
		static const MString name;

		MStatus doIt(const MArgList& args);
		static void* creator();
};
