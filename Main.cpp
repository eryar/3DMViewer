/*
*    Copyright (c) 2014 eryar All Rights Reserved.
*
*        File    : Main.cpp
*        Author  : eryar@163.com
*        Date    : 2014-11-01 18:52
*        Version : 1.0v
*
*    Description : OpenNURBS 3DM file viewer.
*
*      Key words : OpenNURBS, BRep, 3DM Viewer.
*/

#include <iostream>

// OpenSceneGraph library.
#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include "RhinoReader.h"


#ifdef _DEBUG
    #pragma comment(lib, "osgd.lib")
    #pragma comment(lib, "osgGAd.lib")
    #pragma comment(lib, "osgUtild.lib")
    #pragma comment(lib, "osgViewerd.lib")

    // OpenNURBS toolkit.
    #pragma comment(lib, "opennurbs_d.lib")
#else
    #pragma comment(lib, "osg.lib")
    #pragma comment(lib, "osgGA.lib")
    #pragma comment(lib, "osgUtil.lib")
    #pragma comment(lib, "osgViewer.lib")

    // OpenNURBS toolkit.
    #pragma comment(lib, "opennurbs.lib")
#endif


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "please input the 3dm file..." << std::endl;

        return 0;
    }

    ON::Begin();

    RhinoReader aReader(argv[1]);

    osgViewer::Viewer viewer;

    viewer.setSceneData(aReader.GetRhinoModel());

    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

    ON::End();

    return viewer.run();
}