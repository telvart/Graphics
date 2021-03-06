// Tree.h

#ifndef TREE_H
#define TREE_H

#include "ModelView.h"
#include "ShaderIF.h"
#include "Trunk.h"
#include "TreeTop.h"

typedef float vec2[2];
typedef float vec3[3];

class Tree : public ModelView
{
public:
	// As before: you will likely want to add parameters to the constructor
	Tree(ShaderIF* sIF, double bottomx, double bottomy, double bottomz, double height, double treeRadius, vec3 color);
	virtual ~Tree();

	// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
	void getMCBoundingBox(double* xyzLimitsF) const;
	void render();
private:

	void defineTree();

	TreeTop* top;
	Trunk* trunk;
	vec3 topColor;

	float xyz[6];

	cryph::AffPoint treeBottom;
	double m_height;
	double m_baseRadius;


	ShaderIF* shaderIF;

};

#endif
