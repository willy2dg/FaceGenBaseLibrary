//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Jan 17, 2014
//
// Basic FBX export

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dNormals.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"

using namespace std;

static
size_t
idModel(size_t mm)
{return mm*100+1; }

static
size_t
idGeometry(size_t mm)
{return mm*100+2; }

static
size_t
idGeoExp(size_t mm,size_t ee)
{return ee*1000+mm*100+7; }

static
size_t
idDeformer(size_t mm,size_t ee)
{return ee*1000+mm*100+8; }

static
size_t
idSubdeformer(size_t mm,size_t ee)
{return ee*1000+mm*100+9; }

static
size_t
idMaterial(size_t mm,size_t tt)
{return mm*100+tt*10+3; }

static
size_t
idTexture(size_t mm,size_t tt)
{return mm*100+tt*10+4; }

static
size_t
idVideo(size_t mm,size_t tt)
{return mm*100+tt*10+5; }

static
string
nmVideo(size_t mm,size_t tt)
{return "\"Video::Video"+fgToString(mm)+"_"+fgToString(tt)+"\""; }

void
fgSaveFbx(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes,
    string                      imgFormat)
{
    FGASSERT(!meshes.empty());
    FgPath      path(filename);
    path.ext = "fbx";
    FgOfstream  ofs(path.str());
    ofs.precision(7);
    ofs <<
        "; FBX 7.4.0 project file\n"
        "; Created by FaceGen\n"
        "FBXHeaderExtension:  {\n"
        "    FBXHeaderVersion: 1003\n"
        "    FBXVersion: 7400\n"
        "}\n"
        "Definitions:  {\n"
        "    Version: 100\n"
        "    ObjectType: \"Model\" {\n"
        "    }\n"
        "    ObjectType: \"Geometry\" {\n"
        "    }\n"
        "    ObjectType: \"Material\" {\n"
        "    }\n"
        "    ObjectType: \"Texture\" {\n"
        "    }\n"
        "    ObjectType: \"Video\" {\n"
        "    }\n"
        "    ObjectType: \"Deformer\" {\n"
        "    }\n"
        "}\n"
        "Objects:  {\n";
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        ofs <<
            "    Model: " << idModel(mm) << ", \"Model::" << mesh.name << "\", \"Mesh\" {\n"
            "        Version: 232\n"
            "        Properties70:  {\n"
            "            P: \"ScalingMax\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
            "            P: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n"
            "            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\",0,0,0\n"
            "        }\n"
            "        Shading: T\n"
            "        Culling: \"CullingOff\"\n"
            "    }\n"
            "    Geometry: " << idGeometry(mm) << ", \"Geometry::" << mesh.name << "\" , \"Mesh\" {\n"
            "        Vertices: *" << mesh.verts.size()*3 << " {\n"
            "            a: ";
        for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
            FgVect3F    v = mesh.verts[vv];
            if (vv > 0)
                ofs << ",";
            ofs << v[0] << "," << v[1] << "," << v[2];
        }
        ofs << endl <<
            "        }\n"
            "        PolygonVertexIndex: *" << mesh.numTris()*3+mesh.numQuads()*4 << " {\n"
            "            a: ";
        bool        start = true;
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Fg3dSurface & surf = mesh.surfaces[ss];
            for (size_t tt=0; tt<surf.tris.size(); ++tt) {
                FgVect3UI   i = surf.tris.vertInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << int(~i[2]);     // bitwise negation of last index WTF
            }
            for (size_t tt=0; tt<surf.quads.size(); ++tt) {
                FgVect4UI   i = surf.quads.vertInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << i[2] << "," << int(~i[3]);
            }
        }
        ofs << endl <<
            "        }\n"
            "        GeometryVersion: 124\n"
            "        LayerElementNormal: 0 {\n"
            "            Version: 102\n"
            "            Name: \"\"\n"
            "            MappingInformationType: \"ByVertice\"\n"
            "            ReferenceInformationType: \"Direct\"\n"
            "            Normals: *" << mesh.verts.size()*3 << " {\n"
            "                a: ";
        Fg3dNormals     norms = fgNormals(mesh);
        for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
            FgVect3F    n = norms.vert[vv];
            if (vv > 0)
                ofs << ",";
            ofs << n[0] << "," << n[1] << "," << n[2];
        }
        ofs << endl <<
            "            }\n"
            "        }\n"
            "        LayerElementUV: 0 {\n"
            "            Version: 101\n"
            "            Name: \"UVs" << mm << "\"\n"
            "            MappingInformationType: \"ByPolygonVertex\"\n"
            "            ReferenceInformationType: \"IndexToDirect\"\n"
            "            UV: *" << mesh.uvs.size()*2 << " {\n"
            "                a: ";
        for (size_t uu=0; uu<mesh.uvs.size(); ++uu) {
            FgVect2F    uv = mesh.uvs[uu];
            if (uu > 0)
                ofs << ",";
            ofs << uv[0] << "," << uv[1];
        }
        ofs << endl <<
            "            }\n"
            "            UVIndex: *" << mesh.numTris()*3+mesh.numQuads()*4 << " {\n"
            "                a: ";
        start = true;
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Fg3dSurface & surf = mesh.surfaces[ss];
            for (size_t tt=0; tt<surf.tris.uvInds.size(); ++tt) {
                FgVect3UI   i = surf.tris.uvInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << i[2];
            }
            for (size_t tt=0; tt<surf.quads.uvInds.size(); ++tt) {
                FgVect4UI   i = surf.quads.uvInds[tt];
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << i[0] << "," << i[1] << "," << i[2] << "," << i[3];
            }
        }
        ofs << endl <<
            "            }\n"
            "        }\n";
        ofs <<
            "        LayerElementMaterial: 0 {\n"
            "            Version: 101\n"
            "            Name: \"\"\n"
            "            MappingInformationType: \"ByPolygon\"\n"
            "            ReferenceInformationType: \"IndexToDirect\"\n"
            "            Materials: *" << mesh.numFacets() << " {\n"
            "                a: ";
        start = true;
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Fg3dSurface & surf = mesh.surfaces[ss];
            size_t              num = surf.tris.uvInds.size() + surf.quads.uvInds.size();
            for (size_t ii=0; ii<num; ++ii) {
                if (start)
                    start = false;
                else
                    ofs << ",";
                ofs << ss;
            }
        }
        ofs << endl <<
            "            }\n"
            "        }\n"
            "        Layer: 0 {\n"
            "            Version: 100\n"
            "            LayerElement:  {\n"
            "                Type: \"LayerElementNormal\"\n"
            "                TypedIndex: 0\n"
            "            }\n"
            "            LayerElement:  {\n"
            "                Type: \"LayerElementUV\"\n"
            "                TypedIndex: 0\n"
            "            }\n"
            "            LayerElement:  {\n"
            "                Type: \"LayerElementMaterial\"\n"
            "                TypedIndex: 0\n"
            "            }\n"
            "        }\n";
        ofs <<
            "    }\n";
        for (size_t ee=0; ee<mesh.numMorphs(); ++ee) {
            FgIndexedMorph  morph = mesh.getMorphAsIndexedDelta(ee);
            ofs << 
                "    Geometry: " << idGeoExp(mm,ee) << ", \"Geometry::" << morph.name << "\", \"Shape\" {\n"
                "        Version: 100\n"
                "        Indexes: *" << morph.baseInds.size() << " {\n"
                "            a: ";
            for (size_t ii=0; ii<morph.baseInds.size(); ++ii) {
                if (ii > 0)
                    ofs << ",";
                ofs << morph.baseInds[ii];
            }
            ofs << endl <<
                "        }\n"
                "        Vertices: *" << morph.baseInds.size()*3 << " {\n"
                "            a: ";
            for (size_t ii=0; ii<morph.baseInds.size(); ++ii) {
                if (ii > 0)
                    ofs << ",";
                FgVect3F    v = morph.verts[ii];
                ofs << v[0] << "," << v[1] << "," << v[2];
            }
            ofs << endl <<
                "        }\n"
                "    }\n"
                "    Deformer: " << idDeformer(mm,ee) << ", \"Deformer::" << morph.name << "\", \"BlendShape\" {\n"
                "        Version: 100\n"
                "    }\n"
                "    Deformer: " << idSubdeformer(mm,ee) << ", \"SubDeformer::" << morph.name << "\", \"BlendShapeChannel\" {\n"
                "        Version: 100\n"
                "        DeformPercent: 0\n"
                "        FullWeights: *1 {\n"
                "            a: 10\n"
                "        }\n"
                "    }\n";
        }
    }
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        for (size_t tt=0; tt<mesh.surfaces.size(); ++tt) {
            ofs <<
                "    Material: " << idMaterial(mm,tt) << ", \"Material::Material" << mm << "_" << tt << "\", \"\" {\n"
                "        Version: 102\n"
                "        ShadingModel: \"phong\"\n"
                "        MultiLayer: 0\n"
                "        Properties70:  {\n"
                "            P: \"AmbientColor\", \"Color\", \"\", \"A\",0,0,0\n"
                "            P: \"DiffuseColor\", \"Color\", \"\", \"A\",1,1,1\n"
                "            P: \"Emissive\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
                "            P: \"Ambient\", \"Vector3D\", \"Vector\", \"\",0,0,0\n"
                "            P: \"Diffuse\", \"Vector3D\", \"Vector\", \"\",1,1,1\n"
                "            P: \"Opacity\", \"double\", \"Number\", \"\",1\n"
                "        }\n"
                "    }\n";
            FgString    texBaseExt = path.base + fgToString(mm) + "_" + fgToString(tt) + "." + imgFormat;
            if (mesh.surfaces[tt].material.albedoMap)
                fgSaveImgAnyFormat(path.dir() + texBaseExt,*mesh.surfaces[tt].material.albedoMap);
            ofs <<
                "    Texture: " << idTexture(mm,tt) << ", \"Texture::Texture" << mm << "_" << tt << "\", \"TextureVideoClip\" {\n"
                "        Type: \"TextureVideoClip\"\n"
                "        Version: 202\n"
                "        TextureName: \"Texture::Texture" << mm << "_" << tt << "\"\n"
                "        Properties70:  {\n"
                "            P: \"CurrentTextureBlendMode\", \"enum\", \"\", \"\",0\n"
                "            P: \"UVSet\", \"KString\", \"\", \"\", \"UVs" << mm << "\"\n"
                "            P: \"UseMaterial\", \"bool\", \"\", \"\",1\n"
                "        }\n"
                "        Media: " << nmVideo(mm,tt) << "\n"
                "        FileName: \"" << texBaseExt << "\"\n"
                "        RelativeFilename: \"" << texBaseExt << "\"\n"
                "        ModelUVTranslation: 0,0\n"
                "        ModelUVScaling: 1,1\n"
                "        Texture_Alpha_Source: \"None\"\n"
                "        Cropping: 0,0,0,0\n"
                "    }\n";
            ofs <<
                "    Video: " << idVideo(mm,tt) << ", " << nmVideo(mm,tt) << ", \"Clip\" {\n"
                "        Type: \"Clip\"\n"
                "        Properties70:  {\n"
                "            P: \"Path\", \"KString\", \"XRefUrl\", \"\", \"" << texBaseExt << "\"\n"
                "        }\n"
                "        UseMipMap: 0\n"
                "        Filename: \"" << texBaseExt << "\"\n"
                "        RelativeFilename: \"" << texBaseExt << "\"\n"
                "    }\n";
        }
    }
    ofs <<
        "}\n"
        "Connections: {\n";
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        ofs << "    C: \"OO\", " << idModel(mm) << ",0\n";
        for (size_t ee=0; ee<mesh.numMorphs(); ++ee) {
            ofs << "    C: \"OO\", " << idGeoExp(mm,ee) << "," << idSubdeformer(mm,ee) << "\n";
            ofs << "    C: \"OO\", " << idSubdeformer(mm,ee) << "," << idDeformer(mm,ee) << "\n";
            ofs << "    C: \"OO\", " << idDeformer(mm,ee) << "," << idGeometry(mm) << "\n";

        }
    }
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        ofs << "    C: \"OO\", " << idGeometry(mm) << "," << idModel(mm) << "\n";
        for (size_t tt=0; tt<mesh.surfaces.size(); ++tt) {
            ofs <<
                "    C: \"OO\", " << idMaterial(mm,tt) << "," << idModel(mm) << "\n"
                "    C: \"OP\", " << idTexture(mm,tt) << "," << idMaterial(mm,tt) << ", \"DiffuseColor\"\n"
                "    C: \"OO\", " << idVideo(mm,tt) << "," << idTexture(mm,tt) << "\n";
            if (mesh.surfaces[tt].material.albedoMap && fgUsesAlpha(*mesh.surfaces[tt].material.albedoMap))
                ofs <<
                "    C: \"OP\", " << idTexture(mm,tt) << "," << idMaterial(mm,tt) << ", \"TransparentColor\"\n";
        }
    }
    ofs << "}\n";
}

void
fgSaveFbxTest(const FgArgs & args)
{
    FGTESTDIR
    FgString            dd = fgDataDir();
    string              rd = "base/";
    vector<Fg3dMesh>    meshes;
    meshes.push_back(fgLoadTri(dd+rd+"Mouth.tri"));
    meshes.back().surfaces[0].setAlbedoMap(fgLoadImgAnyFormat(dd+rd+"Mouth.tga"));
    meshes.push_back(fgLoadTri(dd+rd+"Glasses.tri"));
    meshes.back().surfaces[0].setAlbedoMap(fgLoadImgAnyFormat(dd+rd+"Glasses.tga"));
    meshes = fgSvec(fgMergeMeshes(meshes));
    fgSaveFbx("meshExportFbx",meshes);
    fgRegressFileRel("meshExportFbx.fbx","base/test/");
    fgRegressFileRel("meshExportFbx0_0.png","base/test/");
    fgRegressFileRel("meshExportFbx0_1.png","base/test/");
}

// */
