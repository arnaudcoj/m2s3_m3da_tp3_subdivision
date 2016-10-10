#include "SubdivSurface.h"
#include "GLTool.h"
#include <algorithm>

using namespace p3d;
using namespace std;

SubdivSurface::~SubdivSurface() {

}


SubdivSurface::SubdivSurface() {

}


void SubdivSurface::input(p3d::Mesh *m) {
  _input=m;
}

void SubdivSurface::source(p3d::Mesh *m) {
  _source=m;
}


void SubdivSurface::source(const std::string &filename) {
  delete _source;
  _source=new Mesh();
  _source->readInit(filename,false);
  _source->requestInitDraw();
}


/** returns the edge index of [v1,v2] (-1 if not found)
 *
 * @pre : vector _edgeForVertex must be intialized before
 */
int SubdivSurface::findEdge(int v1,int v2) {
  ///
  vector<int> &edgeV1=_edgeOfVertex[v1];
  for(unsigned int i=0;i<edgeV1.size();++i) {
    int b=_edge[edgeV1[i]]._b;
    if (b==v2) return edgeV1[i];
  }
  vector<int> &edgeV2=_edgeOfVertex[v2];
  for(unsigned int i=0;i<edgeV2.size();++i) {
    int b=_edge[edgeV2[i]]._b;
    if (b==v1) return edgeV2[i];
  }
  return -1;
}



void SubdivSurface::prepare() {
  if (!_input) return;
  /// compute edge :
  ///
  int v1,v2;
  _edgeOfVertex.clear();
  _edge.clear();
  _edgeOfVertex.resize(_input->nbPosition());
  for(unsigned int i=0;i<_input->nbFace();++i) { // i is the face (index)
    v1=_input->indexPositionVertexFace(i,-1); // the vertex before the first one in face i (i.e. the last one)
    for(unsigned int j=0;j<_input->nbVertexFace(i);++j) { // j is the j-th vertex in the face i
      v2=_input->indexPositionVertexFace(i,j);
      int foundEdge=findEdge(v1,v2);
      if (foundEdge!=-1) { // the edge already exists
        _edge[foundEdge]._right=i; // if already exists, the _left face is already set
      }
      else { // create the edge [v1,v2]
        Edge e;
        e._a=v1;
        e._b=v2;
        e._left=i; // the current face is put on _left (preserve the orientation of the mesh)
        e._right=-1; // will be set if the same edge is encountered after
        _edge.push_back(e);
        _edgeOfVertex[v1].push_back(_edge.size()-1);
        _edgeOfVertex[v2].push_back(_edge.size()-1);
      }
      v1=v2; // next
    }
  }
}



void SubdivSurface::computePointFace() {
  _pointFace.clear();

  //e3q1
  for(int i = 0; i < _input->nbFace(); i++) {
    int nbVertex = _input->nbVertexFace(i);
    Vector3 averageVertex(0., 0., 0.);

    for(int j = 0; j < nbVertex; j++) {
      averageVertex += _input->positionVertexFace(i,j);
    }

    averageVertex /= nbVertex;

    _pointFace.push_back(averageVertex);
  }
}

void SubdivSurface::computePointEdge() {
  _pointEdge.clear();

  //e3q2
  for(int i = 0; i < _edge.size(); i++) {
    Vector3 averageVertex(0., 0., 0.);

    //compute edges
    Edge e = _edge[i];
    averageVertex += _input->positionMesh(e._a) + _input->positionMesh(e._b);

    //compute faces
    averageVertex += _pointFace[e._left] + _pointFace[e._right];

    averageVertex /= 4.;

    _pointEdge.push_back(averageVertex);
  }
}


void SubdivSurface::computePointVertex() {
  _pointVertex.clear();

  //e3q3
  int nbPoints = _edgeOfVertex.size();

  for(int i = 0; i < nbPoints; i++) {
    int nbEdges = _edgeOfVertex[i].size();

    Vector3 P = _input->positionMesh(i);
    Vector3 F(0., 0., 0.);
    Vector3 R(0., 0., 0.);

    for(int j = 0; j < nbEdges; j++) {
      Edge e = _edge[_edgeOfVertex[i][j]];

      F += _pointFace[e._right];

      R += (_input->positionMesh(e._a) + _input->positionMesh(e._b)) / 2.;
    }

    F /= double(nbEdges);
    R /= double(nbEdges);
    Vector3 averageVertex = (F + 2. * R + (nbEdges - 3.) * P) / nbEdges;

    _pointVertex.push_back(averageVertex);
  }
}

int SubdivSurface::findNextEdge(int i,int j) {

  return -1; // happens for a boundary edge
}

void SubdivSurface::buildMesh() {
  Mesh *m=new Mesh();
  /* TODO : build the new mesh
   * - m->addPositionMesh(aVector3) to add a vertex
   * - m->addFaceMesh({v1,v2,v3,...}) to add a face : caution : v1,v2,v3,... are indexes (int) of the positions of m
   * - caution with the indexes (indexes of m are not the same that the ones for _input : track them).
   *
   */

  //e3q4

  //add all the points previously created

  int firstPtVertex = 0;
  int nbPtVertex = _pointVertex.size();
  for(Vector3 p : _pointVertex) {
    m->addPositionMesh(p);
  }

  int firstPtEdge = nbPtVertex;
  int nbPtEdge = _pointEdge.size();
  for(Vector3 p : _pointEdge) {
    m->addPositionMesh(p);
  }

  int firstPtFace = firstPtEdge + nbPtEdge;
  int nbPtFace = _pointFace.size();
  for(Vector3 p : _pointFace) {
    m->addPositionMesh(p);
  }

  //add all the new faces

  for(int i = 0; i < nbPtVertex; i++) {
    int nbEdge = _edgeOfVertex[i].size();
    for(int j = 0; j < nbEdge; j++) {
        Edge e = _edge[_edgeOfVertex[i][j]];
        int ip = i;
        int ie1 = firstPtEdge + e._a;

        int ifp;
        if (i == e._a) {
          ifp = firstPtFace + e._right;
        } else {
          ifp = firstPtFace + e._left;
        }

        int ie2 = firstPtEdge + e._b;

        cout << i << " " << j << " -> " << ip << " " << ie1 << " " << ifp << " " << ie2 << endl;
        m->addFaceMesh({ip, ie1, ifp, ie2});
      }
  }


  _result=m;
  _result->computeNormal();
  _result->computeTexCoord();
}


void SubdivSurface::catmullClarkIter() {
  prepare();
  computePointFace();
  computePointEdge();
  computePointVertex();
  buildMesh();
}

void SubdivSurface::catmullClark() {
  delete _result;
  _pointVertex.clear();
  _pointEdge.clear();
  _pointFace.clear();
  _result=_source->clone();
  for(int i=0;i<_nbIteration;++i) {
    _input=_result;
    catmullClarkIter();
    delete _input;
  }
}




void SubdivSurface::drawTest() {
  glPointSize(10);


  p3d::ambientColor=Vector4(1,0,0,1);
  p3d::shaderVertexAmbient();
  if (_pointVertex.size()>0) p3d::drawPoints(_pointVertex);


  p3d::ambientColor=Vector4(0,1,0,1);
  p3d::shaderVertexAmbient();
  if (_pointEdge.size()>0) p3d::drawPoints(_pointEdge);


  p3d::ambientColor=Vector4(0,0,1,1);
  p3d::shaderVertexAmbient();
  if (_pointFace.size()>0) p3d::drawPoints(_pointFace);
}





