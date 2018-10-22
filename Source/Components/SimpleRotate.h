#pragma once
#include "Component.h"


class SimpleRotate : public Component
{
public:
	SimpleRotate(RenderObj* pObj);
    void LoadProperties(const rapidjson::Value& properties) override;
    void Update(float deltaTime) override;

protected:
	float mSpeed;
};