//
//  PhongShader.cpp
//  ogl4
//
//  Created by Philipp Lensing on 16.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "PhongShader.h"
#include <string>

#ifdef WIN32
#define ASSET_DIRECTORY "../../assets/"
#else
#define ASSET_DIRECTORY "../assets/"
#endif

const char *VertexShaderCode =
"#version 400\n"
"layout(location=0) in vec4 VertexPos;"
"layout(location=1) in vec4 VertexNormal;"
"layout(location=2) in vec2 VertexTexcoord;"
"out vec3 Position;"
"out vec3 Normal;"
"out vec2 Texcoord;"
"uniform mat4 ModelMat;"
"uniform mat4 ModelViewProjMat;"
"void main()"
"{"
"    Position = (ModelMat * VertexPos).xyz;"
"    Normal =  (ModelMat * VertexNormal).xyz;"
"    Texcoord = VertexTexcoord;"
"    gl_Position = ModelViewProjMat * VertexPos;"
"}";


const char *FragmentShaderCode =
"#version 400\n"
"uniform vec3 EyePos;"
"uniform vec3 LightPos;"
"uniform vec3 LightColor;"
"uniform vec3 DiffuseColor;"
"uniform vec3 SpecularColor;"
"uniform vec3 AmbientColor;"
"uniform float SpecularExp;"
"uniform sampler2D DiffuseTexture;"
"in vec3 Position;"
"in vec3 Normal;"
"in vec2 Texcoord;"
"out vec4 FragColor;"
"float sat( in float a)"
"{"
"    return clamp(a, 0.0, 1.0);"
"}"
"void main()"
"{"
"    vec4 DiffTex = texture( DiffuseTexture, Texcoord);"
"    if(DiffTex.a <0.3f) discard;"
"    vec3 N = normalize(Normal);"
"    vec3 L = normalize(LightPos-Position);"
"    vec3 E = normalize(EyePos-Position);"
"    vec3 R = reflect(-L,N);"
"    vec3 DiffuseComponent = LightColor * DiffuseColor * sat(dot(N,L));"
"    vec3 SpecularComponent = LightColor * SpecularColor * pow( sat(dot(R,E)), SpecularExp);"
"    FragColor = vec4((DiffuseComponent + AmbientColor)*DiffTex.rgb + SpecularComponent ,DiffTex.a);"
"}";

PhongShader::PhongShader(bool LoadStaticShaderCode) :
 DiffuseColor(0.8f,0.8f,0.8f),
 SpecularColor(0.5f,0.5f,0.5f),
 AmbientColor(0.2f,0.2f,0.2f),
 SpecularExp(20.0f),
 LightPos(20.0f,20.0f,20.0f),
 LightColor(1,1,1),
 DiffuseTexture(Texture::defaultTex()),
 NormalTexture(Texture::defaultNormalTex()),
 m_opacityTexture(nullptr),
 m_alphaMultiplier(1.0f),
 UpdateState(0xFFFFFFFF)
{
    if(!LoadStaticShaderCode)
        return;
    //ShaderProgram = createShaderProgram(VertexShaderCode, FragmentShaderCode);
    bool loaded = load(ASSET_DIRECTORY "shaders/phong/vsphong.glsl",
                       ASSET_DIRECTORY "shaders/phong/fsphong.glsl");
	if (!loaded)
		throw std::exception();
    assignLocations();
	
}
void PhongShader::assignLocations()
{
    DiffuseColorLoc = glGetUniformLocation(ShaderProgram, "DiffuseColor");
    AmbientColorLoc = glGetUniformLocation(ShaderProgram, "AmbientColor");
    SpecularColorLoc = glGetUniformLocation(ShaderProgram, "SpecularColor");
    SpecularExpLoc = glGetUniformLocation(ShaderProgram, "SpecularExp");
    DiffuseTexLoc = glGetUniformLocation(ShaderProgram, "DiffuseTexture");
    NormalTexLoc = glGetUniformLocation(ShaderProgram, "NormalTexture");
    OpacityTexLoc = glGetUniformLocation(ShaderProgram, "OpacityTex");
    AlphaMultiplierLoc = glGetUniformLocation(ShaderProgram, "AlphaMultiplier");
    UseOpacityTextureLoc = glGetUniformLocation(ShaderProgram, "UseOpacityTexture");
    LightPosLoc = glGetUniformLocation(ShaderProgram, "LightPos");
    LightColorLoc = glGetUniformLocation(ShaderProgram, "LightColor");
    EyePosLoc = glGetUniformLocation(ShaderProgram, "EyePos");
    ModelMatLoc = glGetUniformLocation(ShaderProgram, "ModelMat");
    ModelViewProjLoc  = glGetUniformLocation(ShaderProgram, "ModelViewProjMat");

	for (int i = 0; i < MaxLightCount; ++i)
	{
		std::string smt = "ShadowMapTexture[" + std::to_string(i) + "]";
		std::string smm = "ShadowMapMat[" + std::to_string(i) + "]";
		ShadowMapTextureLoc[i] = getParameterID(smt.c_str());
		ShadowMapMatLoc[i] = getParameterID(smm.c_str());
		ShadowMapTexture[i] = NULL;
		ShadowMapMat[i].identity();
	}
}
void PhongShader::activate(const BaseCamera& Cam) const
{
    BaseShader::activate(Cam);
   
    // update uniforms if necessary
    if(UpdateState&DIFF_COLOR_CHANGED)
        glUniform3f(DiffuseColorLoc, DiffuseColor.R, DiffuseColor.G, DiffuseColor.B);
    if(UpdateState&AMB_COLOR_CHANGED)
        glUniform3f(AmbientColorLoc, AmbientColor.R, AmbientColor.G, AmbientColor.B);
    if(UpdateState&SPEC_COLOR_CHANGED)
        glUniform3f(SpecularColorLoc, SpecularColor.R, SpecularColor.G, SpecularColor.B);
    if(UpdateState&SPEC_EXP_CHANGED)
        glUniform1f(SpecularExpLoc, SpecularExp);
    
	int TexSlotIdx = 0;
    DiffuseTexture->activate(TexSlotIdx++);
    if(UpdateState&DIFF_TEX_CHANGED && DiffuseTexture)
        glUniform1i(DiffuseTexLoc, TexSlotIdx-1);
	NormalTexture->activate(TexSlotIdx++);
	if (UpdateState&NORM_TEX_CHANGED && NormalTexture)
		glUniform1i(NormalTexLoc, TexSlotIdx-1);

    if (m_opacityTexture != nullptr)
    {
        glUniform1i(UseOpacityTextureLoc, 1);
        m_opacityTexture->activate(TexSlotIdx);
        glUniform1i(OpacityTexLoc, TexSlotIdx);
        TexSlotIdx++;
    }
    else
    {
        glUniform1i(UseOpacityTextureLoc, 0);
    }

    // Der zusätzliche Faktor ermöglicht zeitabhängiges Verblassen ohne Austausch der Textur.
    glUniform1f(AlphaMultiplierLoc, m_alphaMultiplier);

    if(UpdateState&LIGHT_COLOR_CHANGED)
        glUniform3f(LightColorLoc, LightColor.R, LightColor.G, LightColor.B);
    if(UpdateState&LIGHT_POS_CHANGED)
        glUniform3f(LightPosLoc, LightPos.X, LightPos.Y, LightPos.Z);

    // always update matrices
    Matrix ModelViewProj = Cam.getProjectionMatrix() * Cam.getViewMatrix() * modelTransform();
    glUniformMatrix4fv(ModelMatLoc, 1, GL_FALSE, modelTransform().m);
    glUniformMatrix4fv(ModelViewProjLoc, 1, GL_FALSE, ModelViewProj.m);
    
    Vector EyePos = Cam.position();
    glUniform3f(EyePosLoc, EyePos.X, EyePos.Y, EyePos.Z );

	for (int i = 0; i < MaxLightCount; ++i)
	{
		if (ShadowMapTexture[i] && (ShadowMapMatLoc[i] != -1))
		{
			ShadowMapTexture[i]->activate(TexSlotIdx);
			setParameter(ShadowMapTextureLoc[i], TexSlotIdx++);
			setParameter(ShadowMapMatLoc[i], ShadowMapMat[i]);
		}
	}
    
    UpdateState = 0x0;
}
void PhongShader::shadowMap(unsigned int slot, const Texture* pTex, const Matrix& Mtx)
{
	if (slot >= MaxLightCount)
		return;

	ShadowMapTexture[slot] = pTex;
	ShadowMapMat[slot] = Mtx;
}
void PhongShader::diffuseColor( const Color& c)
{
    DiffuseColor = c;
    UpdateState |= DIFF_COLOR_CHANGED;
}
void PhongShader::ambientColor( const Color& c)
{
    AmbientColor = c;
    UpdateState |= AMB_COLOR_CHANGED;
}
void PhongShader::specularColor( const Color& c)
{
    SpecularColor = c;
    UpdateState |= SPEC_COLOR_CHANGED;
}
void PhongShader::specularExp( float exp)
{
    SpecularExp = exp;
    UpdateState |= SPEC_EXP_CHANGED;
}
void PhongShader::lightPos( const Vector& pos)
{
    LightPos = pos;
    UpdateState |= LIGHT_POS_CHANGED;
}
void PhongShader::lightColor(const Color& c)
{
    LightColor = c;
    UpdateState |= LIGHT_COLOR_CHANGED;
}

void PhongShader::diffuseTexture(const Texture* pTex)
{
    DiffuseTexture = pTex;
    if(!DiffuseTexture)
        DiffuseTexture = Texture::defaultTex();
    
    UpdateState |= DIFF_TEX_CHANGED;
}

void PhongShader::normalTexture(const Texture* pTex)
{
	NormalTexture = pTex;
	if (!NormalTexture)
		NormalTexture = Texture::defaultNormalTex();

	UpdateState |= NORM_TEX_CHANGED;
}

void PhongShader::opacityTexture(const Texture* texture)
{
    m_opacityTexture = texture;
}

void PhongShader::alphaMultiplier(float alpha)
{
    if (alpha < 0.0f)
    {
        alpha = 0.0f;
    }
    else if (alpha > 1.0f)
    {
        alpha = 1.0f;
    }

    m_alphaMultiplier = alpha;
}

