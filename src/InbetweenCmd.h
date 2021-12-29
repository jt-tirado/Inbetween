#include <maya/MString.h>
#include <maya/MArglist.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MDagModifier.h>


class InbetweenCmd : public MPxCommand
{
	public:
		static const MString name() { return "inbetween"; }
		static void* creator() { return new InbetweenCmd; }
		static MSyntax newSyntax();
		

		InbetweenCmd();
		virtual ~InbetweenCmd();

		virtual const bool isUndoable() { return true; }
		virtual MStatus doIt(const MArgList& args);
		virtual MStatus redoIt();
		virtual MStatus undoIt();

	private:
		MAnimCurveChange* mAnimCache;
};
