#pragma once

#include <LinearMath/btIDebugDraw.h>

#include <glm/glm.hpp>

#include <vector>
#include <memory>

#include "Camera.h"

class BulletDebugDrawer : public btIDebugDraw
{
public:
	BulletDebugDrawer();
	~BulletDebugDrawer();

	void initialize();
	void reset();
	void draw();
	void setCamera(Camera* camera);

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawSphere(const btVector3& p, btScalar radius, const btVector3& color);
	virtual void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha);
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	virtual void reportErrorWarning(const char* warningString);
	virtual void draw3dText(const btVector3& location, const char* textString);
	virtual void setDebugMode(int debugMode);
	virtual int getDebugMode() const;
private:
	DebugDrawModes debugMode;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;
	unsigned i;
	unsigned currentBufferSize;
	Camera* camera;

	struct Impl;
	std::unique_ptr<Impl> impl;

	void insert(float x, float y, float z, float cx, float cy, float cz);
};