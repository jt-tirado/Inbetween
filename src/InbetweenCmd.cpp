#include "InbetweenCmd.h"

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnDagNode.h>

#include <maya/MAnimCurveChange.h>
#include <maya/MDGModifier.h>
#include <maya/MArgDatabase.h>

#include <maya/MAnimControl.h>
#include <maya/MAnimUtil.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>

static const char* flagWeightName("-w");
static const char* flagWeightNameLong("-weight");


MSyntax InbetweenCmd::newSyntax()
{
    MSyntax syntax;

    // Weight
    syntax.addFlag(flagWeightName, flagWeightNameLong, MSyntax::kDouble);

    // Selection
    syntax.setObjectType(MSyntax::kSelectionList, 1);
    syntax.useSelectionAsDefault(true);

    // No query or edit
    syntax.enableEdit(false);
    syntax.enableQuery(false);

    return syntax;
}

MStatus InbetweenCmd::parseArgs(const MArgList& args)
{
    MStatus status = MS::kSuccess;

    MArgParser argParser(syntax(), args);
    MArgDatabase argData(syntax(), args, &status);

    mCurrentTime = MAnimControl::currentTime();

    if (argParser.isFlagSet(flagWeightName))
    {
        mWeight = argParser.flagArgumentDouble(flagWeightName, 0);
    }
    else
    {
        MGlobal::displayError("No weight value provided.");
        return MS::kFailure;
    }

    // Parse Objects
    mObjects.clear();

    MSelectionList objects;
    status = argData.getObjects(objects);

    if (!status)
    {
        MGlobal::displayError("No objects provided.");
        return MS::kFailure;
    }
    else
    {
        for (unsigned int i = 0; i < objects.length(); ++i)
        {
            MDagPath dag;
            status = objects.getDagPath(i, dag);
            mObjects.append(dag);
        }

    }

    return status;
}

int InbetweenCmd::previousKeyIndex(MFnAnimCurve& animCurve, MTime& time)
{
    int previousIndex = -1;

    // Get Closest Keyframe
    int closeIndex = animCurve.findClosest(time);
    MTime closeTime = animCurve.time(closeIndex);
    int lastIndex = animCurve.numKeys() - 1;

    // Current time is on top of previous or next key.
    if (time == closeTime)
    {
        // Previous
        if (closeIndex > 0)
        {
            previousIndex = closeIndex - 1;
        }
        else
        {
            previousIndex = closeIndex;
        }

    }
    else
    {
        // Current time is outside key range.
        if (closeIndex == lastIndex && time > closeTime)
        {
            previousIndex = lastIndex;
        }
        else
        {
            // Current Time is between two keyframes
            if (closeTime < time)
            {
                previousIndex = closeIndex;
            }
            else if (closeTime > time)
            {
                previousIndex = closeIndex - 1;
            }

        }
    }

    // Clamp
    if (previousIndex < 0)
    {
        previousIndex = 0;
    }
    else if (previousIndex > lastIndex)
    {
        previousIndex = lastIndex;
    }

    return previousIndex;
}

int InbetweenCmd::nextKeyIndex(MFnAnimCurve& animCurve, MTime& time)
{
    int nextIndex = -1;

    // Get Closest Keyframe
    int closeIndex = animCurve.findClosest(time);
    MTime closeTime = animCurve.time(closeIndex);
    int lastIndex = animCurve.numKeys() - 1;

    // Current time is on top of previous or next key.
    if (time == closeTime)
    {
        // Next
        if (closeIndex < lastIndex)
        {
            nextIndex = closeIndex + 1;
        }
        else
        {
            nextIndex = closeIndex;
        }
    }
    else
    {
        // Current time is outside key range.
        if (closeIndex == 0 && time < closeTime)
        {
            nextIndex = 0;
        }
        else
        {
            // Current Time is between two keyframes
            if (closeTime < time)
            {
                nextIndex = closeIndex + 1;
            }
            else if (closeTime > time)
            {
                nextIndex = closeIndex;
            }
        }
    }

    // Clamp
    if (nextIndex < 0)
    {
        nextIndex = 0;
    }
    else if (nextIndex > lastIndex)
    {
        nextIndex = lastIndex;
    }

    return nextIndex;
}

MStatus InbetweenCmd::doIt(const MArgList& args) 
{
    MStatus status = MS::kSuccess;

    // Parse Args
    status = parseArgs(args);

    // Do It
    if (status == MS::kSuccess)
    {
        status = redoIt();
    }

    return status;
}

MStatus InbetweenCmd::redoIt()
{
    MStatus status = MS::kSuccess;

    if (mCache == NULL)
    {
        mCache = new AnimationCurveCache();
    }
    else
    {
        mCache->clear();
    }

    for (unsigned int i = 0; i < mObjects.length(); ++i)
    {
        MDagPath dag = mObjects[i];

        // Get Animated Attributes
        MPlugArray plugs;
        MAnimUtil::findAnimatedPlugs(dag, plugs);

        for (unsigned int p = 0; p < plugs.length(); ++p)
        {
            MPlug plug = plugs[p];

            // Get Animation Curve
            MObjectArray animation;

            if (!MAnimUtil::findAnimation(plug, animation))
            {
                continue;
            }

            for (unsigned int a = 0; a < animation.length(); a++)
            {
                MObject animObject = animation[a];

                if (!animObject.hasFn(MFn::kAnimCurve))
                {
                    continue;
                }

                MFnAnimCurve animCurveFn(animObject);

                if (animCurveFn.numKeys() < 2)
                {
                    continue;
                }

                // Get Closest Keyframe
                unsigned int closeIndex = animCurveFn.findClosest(mCurrentTime);
                MTime closeTime = animCurveFn.time(closeIndex);

                // Get Previous & Next 
                int previousIndex = previousKeyIndex(animCurveFn, mCurrentTime);
                int nextIndex = nextKeyIndex(animCurveFn, mCurrentTime);

                if (previousIndex == nextIndex)
                {
                    continue;
                }

                // Get new value from weight
                double previousValue = animCurveFn.value(previousIndex);
                double nextValue = animCurveFn.value(nextIndex);
                double newValue = (nextValue - previousValue) * mWeight + previousValue;

                MAnimCurveChange* animCache = new MAnimCurveChange();

                // Set new key
                if (mCurrentTime == closeTime)
                {
                    animCurveFn.setValue(closeIndex, newValue, animCache);
                    
                }
                else
                {
                    animCurveFn.addKeyframe(mCurrentTime, newValue, animCache);
                }

                mCache->add(animCache);
            }
        }
    }

    return status;
}

MStatus InbetweenCmd::undoIt()
{
    if (mCache->length() > 0)
    {
        for (unsigned int i = 0; i < mCache->length(); i++)
        {
            MAnimCurveChange* data = mCache->data()[i];
            data->undoIt();
        }
    }

    return MS::kSuccess;
}


