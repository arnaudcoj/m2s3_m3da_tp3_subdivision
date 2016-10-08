#include "SubdivCurve.h"
#include <cmath>
#include <iostream>

#include "Vector3.h"
#include "Matrix4.h"

using namespace std;
using namespace p3d;

SubdivCurve::~SubdivCurve() {
}

SubdivCurve::SubdivCurve() {
  _nbIteration=1;
  _source.clear();
  _result.clear();

}


void SubdivCurve::addPoint(const p3d::Vector3 &p) {
  _source.push_back(p);
}

void SubdivCurve::point(int i,const p3d::Vector3 &p) {
  _source[i]=p;
}


void SubdivCurve::chaikinIter(const vector<Vector3> &p) {
  _result.clear();

  //Q1
  int n = p.size();

  for (int i = 0; i < n - 1; i++) {
    Vector3 q2i = (3./4.) * p[i] + (1./4.) * p[i + 1];
    Vector3 q2ip1 = (1./4.) * p[i] + (3./4.) * p[i + 1];

    _result.push_back(q2i);
    _result.push_back(q2ip1);
  }

  //Q2
  if(isClosed()) {
    Vector3 q2i = (3./4.) * p[n - 1] + (1./4.) * p[0];
    Vector3 q2ip1 = (1./4.) * p[n - 1] + (3./4.) * p[0];

    _result.push_back(q2i);
    _result.push_back(q2ip1);
  }

}

void SubdivCurve::dynLevinIter(const vector<Vector3> &p) {
  _result.clear();

  //Q1
  int n = p.size();

  //Premier point traité ici car p[i - 1] => p[n - 1]
  Vector3 q2i = p[0];
  Vector3 q2ip1 = -(1./16.) * (p[2] + p[n - 1]) + (9./16.) * (p[1] + p[0]);

  _result.push_back(q2i);
  _result.push_back(q2ip1);

  //Le % n gère les indices pour les i + 1, i + 2, etc.
  for (int i = 1; i < n; i++) {
    Vector3 q2i = p[i % n];
    Vector3 q2ip1 = -(1./16.) * (p[(i + 2) % n] + p[(i - 1) % n]) + (9./16.) * (p[(i + 1) % n] + p[i % n]);
    _result.push_back(q2i);
    _result.push_back(q2ip1);
  }
}


void SubdivCurve::chaikin() {
  if (_source.size()<2) return;
  vector<Vector3> current;
  _result=_source;
  for(int i=0;i<_nbIteration;++i) {
    current=_result;
    chaikinIter(current);
  }
}

void SubdivCurve::dynLevin() {
  if (_source.size()<2) return;
  if (!isClosed()) return;
  vector<Vector3> current;
  _result=_source;
  for(int i=0;i<_nbIteration;++i) {
    current=_result;
    dynLevinIter(current);
  }
}


