//
//  TriangleBoxModel.cpp
//  CGXcode
//
//  Created by Philipp Lensing on 10.10.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "TriangleBoxModel.h"

TriangleBoxModel::TriangleBoxModel(float Width, float Height, float Depth)
{
	VB.begin();
	// Front
	VB.addNormal(0, 0, 1);
	VB.addTexcoord0(0.5, 0.5f); VB.addVertex(-Width / 2, -Height / 2, Depth / 2);
	VB.addTexcoord0(1, 0.5f); VB.addVertex(Width / 2, -Height / 2, Depth / 2);
	VB.addTexcoord0(0.5, 0); VB.addVertex(-Width / 2, Height / 2, Depth / 2);
	VB.addTexcoord0(1, 0); VB.addVertex(Width / 2, Height / 2, Depth / 2);
	// Back
	VB.addNormal(0, 0, -1);
	VB.addTexcoord0(1, 1); VB.addVertex(-Width / 2, -Height / 2, -Depth / 2);
	VB.addTexcoord0(0, 1); VB.addVertex(Width / 2, -Height / 2, -Depth / 2);
	VB.addTexcoord0(1, 0); VB.addVertex(-Width / 2, Height / 2, -Depth / 2);
	VB.addTexcoord0(0, 0); VB.addVertex(Width / 2, Height / 2, -Depth / 2);
	// Left
	VB.addNormal(-1, 0, 0);
	VB.addTexcoord0(1, 1); VB.addVertex(-Width / 2, -Height / 2, Depth / 2);
	VB.addTexcoord0(0, 1); VB.addVertex(-Width / 2, -Height / 2, -Depth / 2);
	VB.addTexcoord0(1, 0); VB.addVertex(-Width / 2, Height / 2, Depth / 2);
	VB.addTexcoord0(0, 0); VB.addVertex(-Width / 2, Height / 2, -Depth / 2);
	// Right
	VB.addNormal(1, 0, 0);
	VB.addTexcoord0(0, 1); VB.addVertex(Width / 2, -Height / 2, Depth / 2);
	VB.addTexcoord0(1, 1); VB.addVertex(Width / 2, -Height / 2, -Depth / 2);
	VB.addTexcoord0(0, 0); VB.addVertex(Width / 2, Height / 2, Depth / 2);
	VB.addTexcoord0(1, 0); VB.addVertex(Width / 2, Height / 2, -Depth / 2);
	// Bottom
	VB.addNormal(0, -1, 0);
	VB.addTexcoord0(1, 1); VB.addVertex(-Width / 2, -Height / 2, Depth / 2);
	VB.addTexcoord0(0, 1); VB.addVertex(Width / 2, -Height / 2, Depth / 2);
	VB.addTexcoord0(1, 0); VB.addVertex(-Width / 2, -Height / 2, -Depth / 2);
	VB.addTexcoord0(0, 0); VB.addVertex(Width / 2, -Height / 2, -Depth / 2);
	// Top
	VB.addNormal(0, 1, 0);
	VB.addTexcoord0(0, 1); VB.addVertex(-Width / 2, Height / 2, Depth / 2);
	VB.addTexcoord0(1, 1); VB.addVertex(Width / 2, Height / 2, Depth / 2);
	VB.addTexcoord0(0, 0); VB.addVertex(-Width / 2, Height / 2, -Depth / 2);
	VB.addTexcoord0(1, 0); VB.addVertex(Width / 2, Height / 2, -Depth / 2);
	VB.end();

	IB.begin();
	// Front
	IB.addIndex(0); IB.addIndex(1); IB.addIndex(3);
	IB.addIndex(0); IB.addIndex(3); IB.addIndex(2);
	// Back
	IB.addIndex(4); IB.addIndex(7); IB.addIndex(5);
	IB.addIndex(4); IB.addIndex(6); IB.addIndex(7);
	// Left
	IB.addIndex(8); IB.addIndex(11); IB.addIndex(9);
	IB.addIndex(8); IB.addIndex(10); IB.addIndex(11);
	// Right
	IB.addIndex(12); IB.addIndex(13); IB.addIndex(15);
	IB.addIndex(12); IB.addIndex(15); IB.addIndex(14);
	// Bottom
	IB.addIndex(16); IB.addIndex(19); IB.addIndex(17);
	IB.addIndex(16); IB.addIndex(18); IB.addIndex(19);
	// Top
	IB.addIndex(20); IB.addIndex(21); IB.addIndex(23);
	IB.addIndex(20); IB.addIndex(23); IB.addIndex(22);
	IB.end();
}

void TriangleBoxModel::draw(const BaseCamera& Cam)
{
	BaseModel::draw(Cam);

	VB.activate();
	IB.activate();
	glDrawElements(GL_TRIANGLES, IB.indexCount(), IB.indexFormat(), 0);
	IB.deactivate();
	VB.deactivate();
}
