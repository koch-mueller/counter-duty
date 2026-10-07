#ifndef LIGHTS_H
#define LIGHTS_H

#include "vector.h"
#include "color.h"

class BaseLight
{
public:
	enum LightType
	{
		POINT = 0,
		DIRECTIONAL,
		SPOT
	};

	BaseLight(const Vector& p = Vector(10, 10, 10), const Color& c = ::Color(1, 1, 1)) : Position(p), Col(c), Attenuation(1,0,0), ShadowCaster(false) {}
	virtual ~BaseLight() {}

	const Vector& position() const { return Position; }
	void position(const Vector& p) { Position = p; }

	const Vector& attenuation() const { return Attenuation; }
	void attenuation(const Vector& a) { Attenuation = a; }

	const Color& color() const { return Col; }
	void color(const Color& c) { Col = c; }

	bool castShadows() const { return type() == POINT ? false : ShadowCaster;  }
	void castShadows(bool b) { type()==POINT ? ShadowCaster = false : ShadowCaster = b;  }

	virtual LightType type() const = 0;

protected:
	Vector Position;
	Vector Attenuation;
	Color Col;
	bool ShadowCaster;
};

class PointLight : public BaseLight
{
public:
	PointLight(const Vector& p = Vector(10, 10, 10), const Color& c = ::Color(1, 1, 1)) : BaseLight(p,c) {}
	virtual ~PointLight() {}

	virtual LightType type() const { return POINT; }
};

class DirectionalLight : public BaseLight
{
public:
	DirectionalLight(const Vector& d = Vector(-1, -1, -1), const Color& c = ::Color(1, 1, 1)) : BaseLight(Vector(10,10,10), c), Direction(d) {}
	virtual ~DirectionalLight() {}

	virtual LightType type() const { return DIRECTIONAL; }

	const Vector& direction() const { return Direction; }
	void direction(const Vector& d) { Direction = d; }

protected:
	Vector Direction;
};

class SpotLight : public BaseLight
{
public:
	SpotLight(const Vector& p = Vector(10, 10, 10), const Vector& d = Vector(-1, -1, -1), float InnerRadius = 30.0, float OuterRadius = 40.0f, const Color& c = ::Color(1, 1, 1)) : BaseLight(p, c), Direction(d), InnerRadius(InnerRadius), OuterRadius(OuterRadius) {}
	virtual ~SpotLight() {}

	virtual LightType type() const { return SPOT; }

	float innerRadius() const { return InnerRadius; }
	void innerRadius(float r) { InnerRadius = r; }

	float outerRadius() const { return OuterRadius; }
	void outerRadius(float r) { OuterRadius = r; }

	const Vector& direction() const { return Direction; }
	void direction(const Vector& d) { Direction = d; }

protected:
	float InnerRadius;
	float OuterRadius;
	Vector Direction;
};



#endif
