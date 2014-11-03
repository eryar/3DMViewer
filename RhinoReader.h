/*
*    Copyright (c) 2014 eryar All Rights Reserved.
*
*        File    : Main.cpp
*        Author  : eryar@163.com
*        Date    : 2014-11-03 18:52
*        Version : 1.0v
*
*    Description : Rhino opennurbs 3DM file reader.
*
*/

#ifndef _RHINOREADER_H_
#define _RHINOREADER_H_

#include <osg/Group>

#include <opennurbs.h>

/**
* @breif Rhino opennurbs 3DM file reader.
*/
class RhinoReader
{
public:
    RhinoReader(const std::string& theFileName);
    ~RhinoReader(void);

    osg::Node* GetRhinoModel(void);

    void SetEdgePrecision(double thePrecision);
    void SetFacePrecision(double thePrecision);

private:
    void Read3DM(const std::string& theFileName);

    osg::Node* BuildBrep(const ON_Brep* theBrep);

    osg::Node* BuildEdge(const ON_Brep* theBrep);

    osg::Node* BuildWireFrameFace(const ON_BrepFace* theFace);

    osg::Node* BuildShadedFace(const ON_BrepFace* theFace);

private:
    double mEdgePrecision;

    double mFacePrecision;

    ONX_Model mRhinoModel;

    osg::Node* mRhinoNode;
};

#endif // _RHINOREADER_H_