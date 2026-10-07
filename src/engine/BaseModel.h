//
//  BaseModel.hpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef BaseModel_hpp
#define BaseModel_hpp

#include <stdio.h>
#include "camera.h"
#include "matrix.h"
#include "baseshader.h"
#include "Aabb.h"

class BaseModel
{
public:
    BaseModel();
    virtual ~BaseModel();
    virtual void draw(const BaseCamera& Cam);
    void drawWithShader(const BaseCamera& camera, BaseShader* shader);
    const Matrix& transform() const { return Transform; }
    void transform( const Matrix& m) { Transform = m; }
    virtual void shader( BaseShader* shader, bool deleteOnDestruction=false );
    virtual BaseShader* shader() const { return pShader; }
    virtual const AABB& boundingBox() const { return AABB::unitBox(); }
	bool shadowCaster() const { return ShadowCaster; }
	void shadowCaster(bool sc) { ShadowCaster = sc; }
protected:
    Matrix Transform;
    BaseShader* pShader;
    bool DeleteShader;
	bool ShadowCaster;
        
};


#endif /* BaseModel_hpp */
