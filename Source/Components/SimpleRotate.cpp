#include "stdafx.h"
#include "SimpleRotate.h"
#include "jsonUtil.h"
#include "RenderObj.h"
#include "stringUtil.h"

SimpleRotate::SimpleRotate(RenderObj* pObj)
    : Component(pObj)
{
}

void SimpleRotate::LoadProperties(const rapidjson::Value& properties)
{
	Component::LoadProperties(properties);
	GetFloatFromJSON(properties, "speed", mSpeed);
}

void SimpleRotate::Update(float deltaTime) {
	mObj->mObjectData.c_modelToWorld =  mObj->mObjectData.c_modelToWorld * Matrix4::CreateRotationZ(mSpeed*deltaTime);
}