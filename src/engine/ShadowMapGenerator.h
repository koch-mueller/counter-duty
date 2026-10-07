#ifndef SHADOWMAPGENERATOR_H
#define SHADOWMAPGENERATOR_H

#include "framebuffer.h"
#include "ShaderLightMapper.h"
#include "BaseModel.h"
#include "Texture.h"
#include "BaseShader.h"
#include <list>

class ShadowMapShader : public BaseShader
{
public:
	ShadowMapShader();
	virtual void activate(const BaseCamera& Cam) const;
protected:
	GLint EyePosLoc;
	GLint ModelMatLoc;
	GLint ModelViewProjMatLoc;
};

class ShadowMapGenerator
{
public:
	ShadowMapGenerator(unsigned int ShadowMapWidth=1024, unsigned int ShadowMapHeight=1024);
	~ShadowMapGenerator();

	void generate(std::list<BaseModel*>& Models);

protected:
	Matrix calcProjection(BaseLight* pLight, const AABB& BBox, const Matrix& View) const;
	Matrix calcView(BaseLight* pLight, const AABB& BBox) const;
	AABB calcSceneBoundingBox(std::list<BaseModel*>& Models) const;
	bool shadowCasterInScene() const;
	Framebuffer FrameBuffer;
	Texture ShadowMaps[MaxLightCount];
	SimpleCamera ShadowCams[MaxLightCount];
	ShadowMapShader Shader;
};


#endif
