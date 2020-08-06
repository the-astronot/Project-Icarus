#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Triangle.h"
#include "Plane.h"
#include "Vertex.h"
#include "Model.h"

typedef unsigned int uint;

std::vector<Vertex> cubeGen(Vertex origin, double sidelength);
void sphereGen(Model &sphere, double radius, double delta);
void slicer(Model &model);
void meshGen(Model &model);
void stlWriter(Model &model, std::string fileName);
void printUpdate(int num, float den);

int main(int argc, char* argv[]) {
  // BASIC SHAPES GENERATOR
  Vertex origin = Vertex(0,0,0,0);
  double radius, delta;
  std::string fileName;
  radius = atof(argv[2]);
  delta = atof(argv[3]);
  fileName = argv[1];
  Model Sphere(origin, false);
  std::vector<Vertex> cube;
  std::vector<Vertex*> sphere;
  std::vector<Triangle> mesh;
  std::cout << "Generating Vertices...      " << std::endl;
  sphereGen(Sphere, radius, delta);
  cube = cubeGen(origin, 10.00);
  //std::sort(cube.begin(), cube.end(), sortX);
  //std::sort(sphere.begin(), sphere.end(), sortX);
  /*
  for (int i=0; i<cube.size(); i++) {
    std::cout << cube[i].getX() << "," << cube[i].getY() << "," << cube[i].getZ() << std::endl;
  }
  */
  std::vector<Plane> planeX, planeY;
  std::cout << "Slicing...                " << std::endl;
  slicer(Sphere);
  std::cout << "Generating Mesh...        " << std::endl;
  meshGen(Sphere);
  std::cout << "Writing to File...        " << std::endl;
  stlWriter(Sphere, fileName);
  std::cout << "Completed                 " << std::endl;
}


std::vector<Vertex> cubeGen(Vertex origin, double sidelength) {
  std::vector<Vertex> vertices;
  double delta = sidelength/2.0;
  double i=1;
  double j=1;
  double k=1;
  for (int m=0; m<8; m++) {
    Vertex current = Vertex(i*delta + origin.getX(), j*delta + origin.getY(),
                            k*delta + origin.getZ(), m);
    i = -1*i;
    if (i==1) {j = -1*j;}
    if (m>=3) {k=-1;}
    vertices.push_back(current);
  }
  return vertices;
}


void sphereGen(Model &sphere, double radius, double delta) {
  double num_slices = ((2*radius)/delta) + 1;
  double a, b, c, x, y, z;
  a = sphere.getOrigin().getX();
  b = sphere.getOrigin().getY();
  c = sphere.getOrigin().getZ();
  Vertex* current;
  int num = 0;
  for (uint i=0; i<num_slices; i++) {
    for (uint j=0; j<num_slices; j++) {
      x = delta*i - radius;
      y = delta*j - radius;
      z = sqrt(pow(radius,2) - pow(x-a,2) - pow(y-b,2)) + c;
      if (z <= radius) {
        current = new Vertex(x, y, z, num);
        num++;
        sphere.addVertex(current);
        if (z != 0) {
          current = new Vertex(x, y, -1*z, num);
          sphere.addVertex(current);
          num++;
        }
      }
      printUpdate(i*num_slices+j, pow(num_slices,2));
    }
  }
}


void slicer(Model &model) {
  int first = 0;
  int last = 0;
  uint p = 0;
  // Slice X
  Plane cur_plane;

  model.sortByX();
  uint size = model.getNumVertices();
  double x = model.getVertices(0)->getX();
  for(uint i=1; i<=size; i++) {
    if (i == size) {
      std::vector<Vertex*> current;
      for (int j=first; j<=last; j++) {
        current.push_back(model.getVertices(j));
      }
      cur_plane = Plane(current, p);
      model.addXPlane(cur_plane);
    } else if (model.getVertices(i)->getX() == x) {
      last = i;
    } else {
      std::vector<Vertex*> current;
      for (int j=first; j<=last; j++) {
        current.push_back(model.getVertices(j));
      }
      cur_plane = Plane(current, p);
      p++;
      model.addXPlane(cur_plane);
      first = i;
      last = i;
      x = model.getVertices(i)->getX();
    }
    printUpdate(i, size);
  }
  /*
  // Now Slice Y
  std::sort(vertices.begin(), vertices.end(), sortY);
  double y = vertices[0].getY();
  first = 0;
  last = 0;
  p = 0;
  for(int i=1; i<=vertices.size(); i++) {
    if (i == vertices.size()+1) {
      std::vector<Vertex> current;
      for (int j=first; j<=last; j++) {
        current.push_back(vertices[j]);
      }
      Plane cur_plane = Plane(current, p);
      planeY.push_back(cur_plane);
    } else if (vertices[i].getY() == y) {
      last = i;
    } else {
      std::vector<Vertex> current;
      for (int j=first; j<=last; j++) {
        current.push_back(vertices[j]);
      }
      Plane cur_plane = Plane(current, p);
      p++;
      planeY.push_back(cur_plane);
      first = i;
      last = i;
      y = vertices[i].getY();
    }
  }
  */
}


void meshGen(Model &model) {
  Triangle newT;
  Vertex *nextHigh, *nextLow;
  bool endLow, endHigh, nextIsLow;
  int num1, num2;
  int tot = 0;
  uint size = model.getNumXPlanes();
  for (uint i=1; i<size; i++) {
    Plane p1 = model.getPlaneX(i-1);
    Plane p2 = model.getPlaneX(i);
    endLow = false;
    endHigh = false;
    num1 = 1;
    num2 = 1;
    //std::cout << p1.getNumVert() << std::endl;
    if (p1.getNumVert() > 1) {nextLow = p1.getVertex(1);}
    else {endLow = true;}
    if (p2.getNumVert() > 1) {nextHigh = p2.getVertex(1);}
    else {endHigh = true;}
    // Choose next Vertex for Triangle
    while (!endLow || !endHigh) {
      if (!endLow && !endHigh) {
        if (nextLow->getTheta() >= nextHigh->getTheta()) {
          nextIsLow=true;
        } else {
          nextIsLow=false;
        }
      } else if (!endLow) {
        nextIsLow=true;
      } else {
        nextIsLow=false;
      }
      // Create New Triangle
      if (nextIsLow) {
        newT = Triangle(nextLow, p1.getVertex(num1-1), p2.getVertex(num2-1), tot);
        num1++;
      } else {
        newT = Triangle(nextHigh, p1.getVertex(num1-1), p2.getVertex(num2-1), tot);
        num2++;
      }
      model.addTriangle(newT);
      tot++;
      if (p1.getNumVert() > num1) {nextLow = p1.getVertex(num1);}
      else {endLow = true;}
      if (p2.getNumVert() > num2) {nextHigh = p2.getVertex(num2);}
      else {endHigh = true;}
    }
    // Clean Up Last 2 Triangles
    if (p1.getNumVert() > 1) {
      newT = Triangle(p1.getVertex(0), p1.getVertex(num1-1), p2.getVertex(num2-1), tot);
      tot++;
      num1 = 1;
      model.addTriangle(newT);
    }
    if (p2.getNumVert() > 1) {
      newT = Triangle(p2.getVertex(0), p1.getVertex(num1-1), p2.getVertex(num2-1), tot);
      tot++;
      num2 = 1;
      model.addTriangle(newT);
    }
    printUpdate(i, size);
  }
}


void stlWriter(Model &model, std::string fileName){
  std::ofstream STLFILE(fileName + ".stl");
  Triangle triangle;
  Vertex* vertex;
  uint size = model.getMeshSize();
  STLFILE << "solid Sphere_Model\n";

  // Add all Triangles
  for(uint i=0; i<size; i++) {
    triangle = model.getMesh(i);
    STLFILE << "\tfacet normal " << triangle.getNx() << " " << triangle.getNy() <<
               " " << triangle.getNz() << "\n";
    STLFILE << "\t\touter loop \n";
    for (int j=0; j<3; j++) {
      vertex = model.getMesh(i).get(j);
      STLFILE << "\t\t\tvertex " << vertex->getX() << " " <<
                 vertex->getY() << " " << vertex->getZ() << "\n";
    }
    STLFILE << "\t\tendloop\n";
    STLFILE << "\tendfacet\n";
    printUpdate(i, size);
  }
  // End File
  STLFILE << "endsolid Sphere_Model";
  STLFILE.close();
}

void printUpdate(int num, float den) {
  uint bar_size = 20;
  float percent = (num/den)*bar_size;
  std::cout << "[";
  for (uint i=0; i<bar_size; i++) {
    if (i<percent) {std::cout << "*";}
    else {std::cout << "_";}
  }
  std::cout << "]\r";
}