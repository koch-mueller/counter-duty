#include "ShadowMapGenerator.h"
#include "ShaderLightMapper.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "PhongShader.h"

#ifdef WIN32
#define ASSET_DIRECTORY "../../assets/"
#else
#define ASSET_DIRECTORY "../assets/"
#endif

ShadowMapShader::ShadowMapShader()
{
    bool loaded = load(ASSET_DIRECTORY "shaders/light_mapper/vslightdummy.glsl",
                              ASSET_DIRECTORY "shaders/light_mapper/fslightdummy.glsl");
	if (!loaded)
		throw std::exception();

	EyePosLoc = getParameterID("EyePos");
	//assert(EyePosLoc != -1);
	ModelMatLoc = getParameterID("ModelMat");
	//assert(ModelMatLoc != -1);
	ModelViewProjMatLoc = getParameterID("ModelViewProjMat");
	//assert(ModelViewProjMatLoc != -1);
}

void ShadowMapShader::activate(const BaseCamera& Cam) const
{
	BaseShader::activate(Cam);
	// always update matrices
	Matrix ModelViewProj = Cam.getProjectionMatrix() * Cam.getViewMatrix() * modelTransform();
	setParameter(ModelMatLoc, modelTransform());
	setParameter(ModelViewProjMatLoc, ModelViewProj);

	Vector EyePos = Cam.position();
	setParameter(EyePosLoc, EyePos);

}

ShadowMapGenerator::ShadowMapGenerator(unsigned int ShadowMapWidth, unsigned int ShadowMapHeight)
{
	for (int i = 0; i < MaxLightCount; ++i)
	{
		bool created = ShadowMaps[i].create(ShadowMapWidth, ShadowMapHeight, GL_R32F, GL_RED, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, false);
		if (!created)
			throw std::exception();
	}

	bool created = FrameBuffer.create(true, ShadowMapWidth, ShadowMapHeight);
	if (!created)
		throw std::exception();
}

ShadowMapGenerator::~ShadowMapGenerator()
{

}

Matrix ShadowMapGenerator::calcProjection(BaseLight* pLight, const AABB& BBox, const Matrix& View ) const
{
	assert(pLight->type() != BaseLight::POINT);
	Matrix Proj;

	Matrix InvView = View;
	InvView.invert();

	if (pLight->type() == BaseLight::SPOT)
	{
		SpotLight* sp = dynamic_cast<SpotLight*>(pLight);
		Proj.perspective(sp->outerRadius()*2.0f / 180.0f*(float)M_PI, 1.0f, 0.001f, BBox.size().length() + (BBox.center()-InvView.translation()).length() );
	}
	else if (pLight->type() == BaseLight::DIRECTIONAL)
	{

		Vector v[8];
		BBox.corners(v);

		float MaxU=-1e6, MaxV=-1e6;

		for (int i = 0; i < 8; ++i)
		{
			Vector p = v[i] - InvView.translation();

			float u = fabs(p.dot(InvView.right()));
			float v = fabs(p.dot(InvView.up()));

			if (u > MaxU)
				MaxU = u;
			if (v > MaxV)
				MaxV = v;
		}
		Proj.orthographic(MaxU * 2 +0.1f, MaxV * 2 + 0.1f, 0.001f, BBox.size().length() + 5.0f);
	}

	return Proj;

}

Matrix ShadowMapGenerator::calcView(BaseLight* pLight, const AABB& BBox) const
{
	assert(pLight->type() != BaseLight::POINT);

	Matrix view;

	if (pLight->type() == BaseLight::SPOT)
	{
		SpotLight* sp = dynamic_cast<SpotLight*>(pLight);
		view.lookAt(sp->position() + sp->direction(), Vector(0, 1, 0), sp->position());

	}
	else if (pLight->type() == BaseLight::DIRECTIONAL)
	{
		DirectionalLight* dl = dynamic_cast<DirectionalLight*>(pLight);
		Vector Pos = BBox.center();
		float diag = BBox.size().length();
		Pos = Pos - dl->direction()* 0.5f*diag;
		view.lookAt(Pos + dl->direction(), Vector(0, 1, 0), Pos);
		dl->position(Pos);
	}

	return view;
}


AABB ShadowMapGenerator::calcSceneBoundingBox(std::list<BaseModel*>& Models) const
{
	if(Models.size()<=0)
		return AABB(Vector(-5, -5, -5), Vector(5, 5, 5));
	
	AABB OverallBox;

	BaseModel* FirstModel = *Models.begin();
	OverallBox = FirstModel->boundingBox().transform(FirstModel->transform());

	bool ShadowCasterFound = false;

	for (BaseModel* pModel : Models)
	{
		if (pModel->shadowCaster())
		{
			AABB Box = pModel->boundingBox().transform(pModel->transform());
			if (ShadowCasterFound)
				OverallBox.merge(Box);
			else
				OverallBox = Box;
			ShadowCasterFound = true;
		}
	}

	if(!ShadowCasterFound)
		return AABB(Vector(-5, -5, -5), Vector(5, 5, 5));

	return OverallBox;
}

bool ShadowMapGenerator::shadowCasterInScene() const
{
	for (BaseLight* pLight : ShaderLightMapper::instance().lights())
	{
		if (pLight->castShadows()) 
			return true;
	}

	return false;
}

void ShadowMapGenerator::generate(std::list<BaseModel*>& Models)
{
	if (!shadowCasterInScene())
		return;

	AABB SceneBoundingBox = calcSceneBoundingBox(Models);

	int ShadowMapCount = 0;
	for (BaseLight* pLight : ShaderLightMapper::instance().lights())
	{
		if (!pLight->castShadows())
			continue;

		Matrix View = calcView(pLight, SceneBoundingBox);
		Matrix Proj = calcProjection(pLight, SceneBoundingBox, View);

		ShadowCams[ShadowMapCount].setViewMatrix(View);
		ShadowCams[ShadowMapCount++].setProjectionMatrix(Proj);
	}

	
	glClearColor(1.0f, 0.0, 0.0f, 1);
	glCullFace(GL_FRONT);

	GLint PrevViewport[4];
	glGetIntegerv(GL_VIEWPORT, PrevViewport);
	glViewport(0, 0, ShadowMaps[0].width(), ShadowMaps[0].height());
	
	int i = 0;
	for (BaseLight* pLight : ShaderLightMapper::instance().lights())
	{
		if (!pLight->castShadows())
			continue;
		
		FrameBuffer.attachColorTarget(ShadowMaps[i]);
		FrameBuffer.activate();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		for (BaseModel* pModel : Models)
		{
			if (!pModel->shadowCaster())
				continue;
			
			BaseShader* PrevShader = pModel->shader();
			
			pModel->shader(&Shader);
			pModel->draw(ShadowCams[i]);
			pModel->shader(PrevShader);
			
			PhongShader* pPhong = dynamic_cast<PhongShader*>(PrevShader);
			if (pPhong)
			{
				Matrix ShadowMapMat = ShadowCams[i].getProjectionMatrix() * ShadowCams[i].getViewMatrix();
				pPhong->shadowMap(i, &ShadowMaps[i], ShadowMapMat);
			}
		}
		
		FrameBuffer.deactivate();
		FrameBuffer.detachColorTarget();
		++i;
	}
	
	glClearColor(0, 0, 0, 1);
	glCullFace(GL_BACK);
	glViewport(PrevViewport[0], PrevViewport[1], PrevViewport[2], PrevViewport[3]);



}
