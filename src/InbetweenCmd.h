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

class AnimationCache
{
	public:
		struct AnimCurveData
		{
			MObject& animObject;
			ModifyType type;
			unsigned int index;
			double value;
			AnimCurveData(MObject& animObject, ModifyType type, int index = -1, double value = 0) : animObject(animObject), type(type), index(index), value(value) {}
		};

		AnimationCache() : mData() {}
		~AnimationCache() { mData.clear(); }

		void add(MObject& animObject, ModifyType type, unsigned int index = -1, double value = 0)
		{
			AnimationCache::AnimCurveData* data = new AnimationCache::AnimCurveData(animObject, type, index, value);
			mData.push_back(data);
		}

		void clear() { mData.clear(); }
		size_t length() { return mData.size(); }
		std::vector<AnimCurveData*> data() { return mData; }

		static AnimationCache* instance() 
		{
			if (!mInstance)
			{
				mInstance = new AnimationCache;
			}

			return mInstance;
		}

	private:
		static AnimationCache* mInstance;
		std::vector<AnimCurveData*> mData;
};

class AnimationCurveCache
{
	public:
		~AnimationCurveCache() { mData.clear(); }

		void add(MAnimCurveChange* animCurveChange)
		{
			mData.push_back(animCurveChange);
		}

		void clear() { mData.clear(); }
		size_t length() { return mData.size(); }
		std::vector<MAnimCurveChange*> data() { return mData; }

		static AnimationCurveCache* instance()
		{
			if (!mInstance)
			{
				mInstance = new AnimationCurveCache;
			}

			return mInstance;
		}

		static void release()
		{
			AnimationCurveCache::instance()->clear();

			if (mInstance != NULL)
			{
				delete mInstance;
				mInstance = NULL;
			}
		}

	private:
		AnimationCurveCache() : mData() {}

		static AnimationCurveCache* mInstance;
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
		//AnimationCache* mCache;

		MStatus parseArgs(const MArgList& args);
};
