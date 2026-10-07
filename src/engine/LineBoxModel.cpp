//
//  LineBoxModel.cpp
//  CGXcode
//
//  Created by Philipp Lensing on 10.10.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "LineBoxModel.h"

LineBoxModel::LineBoxModel( float Width, float Height, float Depth )
{
	VB.begin();

    // Front - Bottom
    VB.addVertex(-Width / 2, -Height / 2, Depth / 2);
    VB.addVertex(Width / 2, -Height / 2, Depth / 2);

    // Front - Top
	VB.addVertex(-Width / 2, Height / 2, Depth / 2);
	VB.addVertex(Width / 2, Height / 2, Depth / 2);

    // Front - Left
	VB.addVertex(-Width / 2, -Height / 2, Depth / 2);
	VB.addVertex(-Width / 2, Height / 2, Depth / 2);

	// Front - Right
	VB.addVertex(Width / 2, -Height / 2, Depth / 2);
    VB.addVertex(Width / 2, Height / 2, Depth / 2);

    // Back - Bottom
    VB.addVertex(-Width / 2, -Height / 2, -Depth / 2);
    VB.addVertex(Width / 2, -Height / 2, -Depth / 2);

	// Back - Top
	VB.addVertex(-Width / 2, Height / 2, -Depth / 2);
	VB.addVertex(Width / 2, Height / 2, -Depth / 2);

	// Back - Left
	VB.addVertex(-Width / 2, -Height / 2, -Depth / 2);
	VB.addVertex(-Width / 2, Height / 2, -Depth / 2);

	// Back - Right
	VB.addVertex(Width / 2, -Height / 2, -Depth / 2);
	VB.addVertex(Width / 2, Height / 2, -Depth / 2);

	// Left - Bottom
	VB.addVertex(-Width / 2, -Height / 2, Depth / 2);
	VB.addVertex(-Width / 2, -Height / 2, -Depth / 2);

	// Left - Top
	VB.addVertex(-Width / 2, Height / 2, Depth / 2);
	VB.addVertex(-Width / 2, Height / 2, -Depth / 2);

	// Right - Bottom
	VB.addVertex(Width / 2, -Height / 2, Depth / 2);
	VB.addVertex(Width / 2, -Height / 2, -Depth / 2);

	// Right - Top
	VB.addVertex(Width / 2, Height / 2, Depth / 2);
	VB.addVertex(Width / 2, Height / 2, -Depth / 2);

	VB.end();
}

LineBoxModel::LineBoxModel(const Vector& min, const Vector& max)
{
    VB.begin();

    // Front - Bottom
    VB.addVertex(min.X, min.Y, max.Z);
    VB.addVertex(max.X, min.Y, max.Z);

    // Front - Top
    VB.addVertex(min.X, max.Y, max.Z);
    VB.addVertex(max.X, max.Y, max.Z);

    // Front - Left
    VB.addVertex(min.X, min.Y, max.Z);
    VB.addVertex(min.X, max.Y, max.Z);

    // Front - Right
    VB.addVertex(max.X, min.Y, max.Z);
    VB.addVertex(max.X, max.Y, max.Z);

    // Back - Bottom
    VB.addVertex(min.X, min.Y, min.Z);
    VB.addVertex(max.X, min.Y, min.Z);

    // Back - Top
    VB.addVertex(min.X, max.Y, min.Z);
    VB.addVertex(max.X, max.Y, min.Z);

    // Back - Left
    VB.addVertex(min.X, min.Y, min.Z);
    VB.addVertex(min.X, max.Y, min.Z);

    // Back - Right
    VB.addVertex(max.X, min.Y, min.Z);
    VB.addVertex(max.X, max.Y, min.Z);

    // Left - Bottom
    VB.addVertex(min.X, min.Y, max.Z);
    VB.addVertex(min.X, min.Y, min.Z);

    // Left - Top
    VB.addVertex(min.X, max.Y, max.Z);
    VB.addVertex(min.X, max.Y, min.Z);

    // Right - Bottom
    VB.addVertex(max.X, min.Y, max.Z);
    VB.addVertex(max.X, min.Y, min.Z);

    // Right - Top
    VB.addVertex(max.X, max.Y, max.Z);
    VB.addVertex(max.X, max.Y, min.Z);

    VB.end();
}

void LineBoxModel::draw(const BaseCamera& Cam)
{
    BaseModel::draw(Cam);

    VB.activate();

    glDrawArrays(GL_LINES, 0, VB.vertexCount());

    VB.deactivate();
}
