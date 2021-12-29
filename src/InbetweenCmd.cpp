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

InbetweenCmd::InbetweenCmd()
{
}

InbetweenCmd::~InbetweenCmd()
{
    delete mAnimCache;
}

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

MStatus InbetweenCmd::doIt(const MArgList& args) 
{
    MStatus status = MS::kSuccess;

    // Cache
    mAnimCache = new MAnimCurveChange();

    // Parse Args
    MArgParser argParser(syntax(), args);
    MArgDatabase argData(syntax(), args, &status);

    double weight;
    MTime currentTime = MAnimControl::currentTime();

    if (argParser.isFlagSet(flagWeightName))
    {
        weight = argParser.flagArgumentDouble(flagWeightName, 0);
    }
    else
    {
        MGlobal::displayError("No weight value provided.");
        return MS::kFailure;
    }

    // Parse Objects
    MSelectionList objects;
    status = argData.getObjects(objects);
    
    if (!status)
    {
        MGlobal::displayError("No objects provided.");
        return MS::kFailure;
    }

    for (unsigned int i = 0; i < objects.length(); ++i)
    {
        MObject node;
        status = objects.getDependNode(i, node);

        MDagPath dag;
        objects.getDagPath(i, dag);

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

                MFnAnimCurve animCurve(animObject);

                if (animCurve.numKeys() < 2)
                {
                    continue;
                }

                // Get Closest Keyframe
                unsigned int closeIndex = animCurve.findClosest(currentTime);
                MTime closeTime = animCurve.time(closeIndex);

                // Get Previous & Next 
                int previousIndex = -1;
                int nextIndex = -1;

                int lastIndex = animCurve.numKeys() - 1;

                // Current time is on top of previous or next key.
                if (currentTime == closeTime)
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
                    // Previous
                    if (closeIndex == 0 && currentTime < closeTime)
                    {
                        previousIndex = -1;
                        nextIndex = 0;
                    }
                    // Next
                    else if (closeIndex == lastIndex && currentTime > closeTime)
                    {
                        nextIndex = -1;
                        previousIndex = lastIndex;
                    }
                    else
                    {
                        // Current Time is between two keyframes
                        if (previousIndex == -1 && nextIndex == -1)
                        {
                            // Previous
                            if (closeTime < currentTime)
                            {
                                previousIndex = closeIndex;
                                nextIndex = closeIndex + 1;
                            }
                            else if (closeTime > currentTime)
                            {
                                nextIndex = closeIndex;
                                previousIndex = closeIndex - 1;
                            }
                            
                        }
                    }
                }

                // Debug
                MString debug;
                debug += previousIndex;
                debug += nextIndex;
                //MGlobal::displayInfo(debug);

                if (previousIndex == nextIndex)
                {
                    continue;
                }
                else if (previousIndex < 0 || previousIndex > lastIndex)
                {
                    continue;
                }
                else if (nextIndex < 0 || nextIndex > lastIndex)
                {
                    continue;
                }

                // Get new value from weight
                double previousValue = animCurve.value(previousIndex);
                double nextValue = animCurve.value(nextIndex);
                double newValue = (nextValue - previousValue) * weight + previousValue;
                
                // Set new key
                if (currentTime == closeTime)
                {
                    animCurve.setValue(closeIndex, newValue, mAnimCache);
                }
                else
                {
                    animCurve.addKeyframe(currentTime, newValue, mAnimCache);
                }

                
               
            }
        }
    }

    return status;
}

MStatus InbetweenCmd::redoIt()
{
    /*if (mAnimCache != NULL)
    {
        return mAnimCache->redoIt();
        
    }

    return MS::kFailure;*/
    mAnimCache->redoIt();
    //mDagMod.doIt();
    return MS::kSuccess;
}

MStatus InbetweenCmd::undoIt()
{
    /*if (mAnimCache != NULL)
    {
        return mAnimCache->undoIt();
        
    }

    return MS::kFailure;*/
    mAnimCache->undoIt();
    //mDagMod.undoIt();
    return MS::kSuccess;
}


