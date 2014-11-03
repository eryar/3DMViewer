#include "RhinoReader.h"

#include <osg/Geode>
#include <osg/Geometry>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/DelaunayTriangulator>

const double TOLERANCE_EDGE = 1e-6;
const double TOLERANCE_FACE = 1e-6;

RhinoReader::RhinoReader(const std::string& theFileName)
: mRhinoNode(NULL)
{
    Read3DM(theFileName);
}

RhinoReader::~RhinoReader(void)
{
}

osg::Node* RhinoReader::GetRhinoModel()
{
    return mRhinoNode;
}

void RhinoReader::Read3DM(const std::string& theFileName)
{
    if (!mRhinoModel.Read(theFileName.c_str()))
    {
        return ;
    }

    osg::Group* aRoot = new osg::Group();

    for (int i = 0; i < mRhinoModel.m_object_table.Count(); ++i)
    {
        ONX_Model_Object anObject = mRhinoModel.m_object_table[i];

        const ON_Brep* aBrep = dynamic_cast<const ON_Brep*> (anObject.m_object);

        if (aBrep)
        {
            aRoot->addChild(BuildBrep(aBrep));
        }
    }

    mRhinoNode = aRoot;
}

osg::Node* RhinoReader::BuildBrep(const ON_Brep* theBrep)
{
    osg::ref_ptr<osg::Group> aGroup = new osg::Group();

    //aGroup->addChild(BuildEdge(theBrep));

    for (int i = 0; i < theBrep->m_F.Count(); ++i)
    {
        ON_BrepFace* aFace = theBrep->Face(i);

        aGroup->addChild(BuildWireFrameFace(aFace));
        //aGroup->addChild(BuildShadedFace(aFace));
    }

    //theBrep->Dump(ON_TextLog());

    return aGroup.release();
}

osg::Node* RhinoReader::BuildEdge(const ON_Brep* theBrep)
{
    osg::ref_ptr<osg::Geode> aGeode = new osg::Geode();

    for (int i = 0; i < theBrep->m_E.Count(); ++i)
    {
        osg::ref_ptr<osg::Geometry> aGeometry = new osg::Geometry();
        osg::ref_ptr<osg::Vec3Array> aVertices = new osg::Vec3Array();

        ON_BrepEdge* anEdge = theBrep->Edge(i);

        double t0 = 0.0;
        double t1 = 0.0;
        double d = 0.0;

        anEdge->GetDomain(&t0, &t1);

        d = (t1 - t0) / 5.0;

        for (double t = t0; (t - t1) < TOLERANCE_EDGE; t += d)
        {
            ON_3dPoint aPoint = anEdge->PointAt(t);

            aVertices->push_back(osg::Vec3(aPoint.x, aPoint.y, aPoint.z));
        }

        aGeometry->setVertexArray(aVertices);
        aGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, aVertices->size()));

        aGeode->addDrawable(aGeometry);
    }

    return aGeode.release();
}

osg::Node* RhinoReader::BuildWireFrameFace(const ON_BrepFace* theFace)
{
    osg::ref_ptr<osg::Geode> aGeode = new osg::Geode();
    
    ON_NurbsSurface aSurface;

    if (theFace->GetNurbForm(aSurface) == 0)
    {
        return NULL;
    }

    double u0 = aSurface.Domain(0).Min();
    double u1 = aSurface.Domain(0).Max();
    double v0 = aSurface.Domain(1).Min();
    double v1 = aSurface.Domain(1).Max();

    double d0 = 0.0;
    double d1 = 0.0;

    d0 = (u1 - u0) / 10.0;
    d1 = (v1 - v0) / 10.0;

    for (double u = u0; (u - u1) < TOLERANCE_FACE; u += d0)
    {
        osg::ref_ptr<osg::Geometry> aGeometry = new osg::Geometry();
        osg::ref_ptr<osg::Vec3Array> aVertices = new osg::Vec3Array();

        for (double v = v0; (v - v1) < TOLERANCE_FACE; v += d1)
        {
            ON_3dPoint aPoint = aSurface.PointAt(u, v);

            aVertices->push_back(osg::Vec3(aPoint.x, aPoint.y, aPoint.z));
        }

        aGeometry->setVertexArray(aVertices);
        aGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, aVertices->size()));

        aGeode->addDrawable(aGeometry);
    }

    for (double v = v0; (v - v1) < TOLERANCE_FACE; v += d1)
    {
        osg::ref_ptr<osg::Geometry> aGeometry = new osg::Geometry();
        osg::ref_ptr<osg::Vec3Array> aVertices = new osg::Vec3Array();

        for (double u = u0; (u - u1) < TOLERANCE_FACE; u += d0)
        {
            ON_3dPoint aPoint = aSurface.PointAt(u, v);

            aVertices->push_back(osg::Vec3(aPoint.x, aPoint.y, aPoint.z));
        }

        aGeometry->setVertexArray(aVertices);
        aGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, aVertices->size()));

        aGeode->addDrawable(aGeometry);
    }

    return aGeode.release();
}

osg::Node* RhinoReader::BuildShadedFace(const ON_BrepFace* theFace)
{
    osg::ref_ptr<osg::Geode> aGeode = new osg::Geode();
    
    ON_NurbsSurface aSurface;

    if (theFace->GetNurbForm(aSurface) == 0)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Geometry> aGeometry = new osg::Geometry();

    osg::ref_ptr<osg::Vec3Array> aUVPoints = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> aPoints = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> aBounds = new osg::Vec3Array();

    osg::ref_ptr<osgUtil::DelaunayTriangulator> dt = new osgUtil::DelaunayTriangulator();
    osg::ref_ptr<osgUtil::DelaunayConstraint> dc = new osgUtil::DelaunayConstraint();

    // add loop for the face.
    for (int i = 0; i < theFace->LoopCount(); ++i)
    {
        ON_BrepLoop* aLoop = theFace->Loop(i);

        if (aLoop->m_type == ON_BrepLoop::outer)
        {
            for (int j = 0; j < aLoop->TrimCount(); ++j)
            {
                ON_BrepTrim* aTrim = aLoop->Trim(j);

                const ON_Curve* aPCurve = aTrim->TrimCurveOf();
                if (aPCurve)
                {
                    ON_3dPoint aStartPoint = aPCurve->PointAtStart();
                    ON_3dPoint aEndPoint = aPCurve->PointAtEnd();

                    aUVPoints->push_back(osg::Vec3(aStartPoint.x, aStartPoint.y, 0.0));
                    aUVPoints->push_back(osg::Vec3(aEndPoint.x, aEndPoint.y, 0.0));
                }
            }
        }
        else if (aLoop->m_type == ON_BrepLoop::inner)
        {
            for (int j = 0; j < aLoop->TrimCount(); ++j)
            {
            }
        }
    }

    dc->setVertexArray(aBounds);
    dc->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, aBounds->size()));

    // triangulate the parametric space.
    //dt->addInputConstraint(dc);
    dt->setInputPointArray(aUVPoints);
    dt->triangulate();
    //dt->removeInternalTriangles(dc);

    for (osg::Vec3Array::const_iterator j = aUVPoints->begin(); j != aUVPoints->end(); ++j)
    {
        // evaluate the point on the surface
        ON_3dPoint aPoint = aSurface.PointAt((*j).x(), (*j).y());

        aPoints->push_back(osg::Vec3(aPoint.x, aPoint.y, aPoint.z));
    }

    //aGeometry->setVertexArray(aUVPoints);
    aGeometry->setVertexArray(aPoints);
    aGeometry->addPrimitiveSet(dt->getTriangles());

    aGeode->addDrawable(aGeometry);

    // use smoothing visitor to set the average normals
    //osgUtil::SmoothingVisitor sv;
    //sv.apply(*aGeode);

    return aGeode.release();
}