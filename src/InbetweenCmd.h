#include <stdio.h>
#include <vector>
#include <maya/MString.h>
#include <maya/MArglist.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MStreamUtils.h>
#include <maya/MSyntax.h>
#include <maya/MDGModifier.h>


//#define MAX_NUM_CACHES 100 // Max number of animation caches

class InbetweenCmd : public MPxCommand
{
	/* Maya commands that handles creating an inbetween key for the selected objects.
	* 
	* References:
	*	- https://blog.fireheart.in/a?ID=00400-20962179-ee8e-4f7c-bb40-ced699b54018
	* 
	* Args:
	*	w, weight(double)
	*	
	*/

	public:
		static const MString name() { return "inbetween"; }
		static void* creator() { return new InbetweenCmd; }
		static MSyntax newSyntax();

		InbetweenCmd();
		~InbetweenCmd();

		virtual const bool isUndoable() { return true; }
		virtual MStatus doIt(const MArgList& args);
		virtual MStatus redoIt();
		virtual MStatus undoIt();

	private:
		std::vector<MAnimCurveChange*> mAnimCache;
		typedef std::vector<MAnimCurveChange*>::const_reverse_iterator undoIter;
		typedef std::vector<MAnimCurveChange*>::const_iterator redoIter;
};
