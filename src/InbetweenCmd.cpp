#include "InbetweenCmd.h"

#include <math.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnDagNode.h>

#include <maya/MAnimCurveChange.h>
#include <maya/MDGModifier.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnAnimCurve.h>

#include <maya/MAnimControl.h>
#include <maya/MAnimUtil.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>

static const char* flagWeightName("-w");
static const char* flagWeightNameLong("-weight");

InbetweenCmd::InbetweenCmd()
{
    //mAnimCache = NULL;
}

InbetweenCmd::~InbetweenCmd()
{
    //delete mAnimCache;
    mAnimCache.clear();
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
    //mAnimCache = NULL;
    //mAnimCache = new MAnimCurveChange();
    //mPlugs = new MPlugArray();
    //mPlugValues = new MDoubleArray();*/

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
        //MFnDependencyNode fnNode(dag.node());
    
        // Debug
        //MGlobal::displayInfo(dag.fullPathName());

        // Get Animated Attributes
        MPlugArray plugs;
        MAnimUtil::findAnimatedPlugs(dag, plugs);

        for (unsigned int p = 0; p < plugs.length(); ++p)
        {
            MPlug plug = plugs[p];

            //MGlobal::displayInfo(plug.name());
            
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

                if (animCurve.numKeys() == 0)
                {
                    continue;
                }

                // Create Cache
                MAnimCurveChange* cache = new MAnimCurveChange();

                // Get Closest Keyframe
                unsigned int closeIndex = animCurve.findClosest(currentTime);
                MTime closeTime = animCurve.time(closeIndex);

                // Get Previous & Next 
                int previousIndex = -1;
                int nextIndex = -1;

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
                    if (closeIndex < animCurve.numKeys() - 1)
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
                    // Still not found
                    // Previous
                    if (closeIndex == 0 && currentTime < closeTime)
                    {
                        previousIndex = -1;
                        nextIndex = 0;
                    }
                    // Next
                    else if (closeIndex == animCurve.numKeys() - 1 && currentTime > closeTime)
                    {
                        nextIndex = -1;
                        previousIndex = animCurve.numKeys() - 1;
                    }
                    else
                    {
                        // Search Previous 2 & Next 2
                        if (previousIndex == -1 && nextIndex == -1)
                        {
                            MDoubleArray distanceQuery;
                            MIntArray indices;

                            double previousMin = currentTime.value() - animCurve.time(0).value();
                            double nextMin = currentTime.value() - animCurve.time(animCurve.numKeys()-1).value();

                            for (int queryIndex = closeIndex - 2; queryIndex < closeIndex + 3; queryIndex++)
                            {
                                if (0 <= queryIndex <= animCurve.numKeys() - 1)
                                {
                                    MTime queryTime = animCurve.time(queryIndex);
                                    double distance = currentTime.value() - queryTime.value();

                                    // Previous 
                                    if (queryTime < currentTime)
                                    {
                                        if (distance < previousMin)
                                        {
                                            previousMin = distance;
                                            previousIndex = queryIndex;
                                        }

                                    }
                                    // Next
                                    else if (queryTime > currentTime)
                                    {
                                        if (distance > nextMin)
                                        {
                                            nextMin = distance;
                                            nextIndex = queryIndex;
                                        }
                                    }
                                }
                            }

                            if (previousIndex == nextIndex)
                            {
                                nextIndex += 1;
                            }
                            else if (previousIndex > nextIndex)
                            {
                                previousIndex -= 1;
                                nextIndex += 1;
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
                else if (previousIndex == -1 || nextIndex == -1)
                {
                    continue;
                }

                // Set new Key
                double previousValue = animCurve.value(previousIndex);
                double nextValue = animCurve.value(nextIndex);
                double newValue = (nextValue - previousValue) * weight + previousValue;

                //plug.setValue(newValue);
                
                if (currentTime == closeTime)
                {
                    animCurve.setValue(closeIndex, newValue, cache);
                }
                else
                {
                    animCurve.addKey(currentTime, newValue, MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, cache);
                }

                mAnimCache.push_back(cache);
                
               
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

    MStatus status = MS::kSuccess;

    redoIter d;
    for (d = mAnimCache.begin(); (status == MS::kSuccess) && (d != mAnimCache.end()); d++)
    {
        status = (*d)->redoIt();
    }

    return status;
}

MStatus InbetweenCmd::undoIt()
{
    /*if (mAnimCache != NULL)
    {
        return mAnimCache->undoIt();
        
    }

    return MS::kFailure;*/

    MStatus status = MS::kSuccess;

    undoIter u;
    for (u = mAnimCache.rbegin(); (status == MS::kSuccess) && (u != mAnimCache.rend()); u++)
    {
        status = (*u)->undoIt();
    }

    return status;
}


