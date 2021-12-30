#include <vector>

#include <maya/MString.h>
#include <maya/MArglist.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MDagModifier.h>
#include <maya/MDGModifier.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MSelectionList.h>

#include <maya/MDagPathArray.h>

enum class ModifyType { Set, Add, Delete };

class AnimationCurveCache
{
	public:
		AnimationCurveCache() : mData() {}
		~AnimationCurveCache() { mData.clear(); }

		void add(MAnimCurveChange* animCurveChange)
		{
			mData.push_back(animCurveChange);
		}

		void clear() { mData.clear(); }
		size_t length() { return mData.size(); }
		std::vector<MAnimCurveChange*> data() { return mData; }

	private:
		std::vector<MAnimCurveChange*> mData;
};


class InbetweenCmd : public MPxCommand
{
	public:
		static const MString name() { return "inbetween"; }
		static void* creator() { return new InbetweenCmd; }
		static MSyntax newSyntax();

		static int previousKeyIndex(MFnAnimCurve& animCurve, MTime& time);
		static int nextKeyIndex(MFnAnimCurve& animCurve, MTime& time);

		InbetweenCmd();
		~InbetweenCmd();

		bool isUndoable() const { return true; }
		MStatus doIt(const MArgList& args);
		MStatus redoIt();
		MStatus undoIt();

	private:
		double mWeight;
		MTime mCurrentTime;

		//MDGModifier mDGMod;
		//MAnimCurveChange* mAnimCache;
		//std::vector<MAnimCurveChange*> mCache;

		MDagPathArray mObjects;
		AnimationCurveCache* mCache = NULL;

		MStatus parseArgs(const MArgList& args);
};
