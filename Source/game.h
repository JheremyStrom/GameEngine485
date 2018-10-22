#pragma once
#include "Graphics.h"
#include "assetCache.h"
#include "engineMath.h"
#include "Job.h"
#include "Physics.h"

class Animation;
class Camera;
class Mesh;
class RenderObj;
class Shader;
class Skeleton;
class Texture;
class VertexBuffer;

#define MAX_POINT_LIGHTS 8

class Game
{
public:
    struct PointLightData
    {
        Vector3 diffuseColor;
        float pad0;
        Vector3 specularColor;
        float pad1;
        Vector3 position;
        float specularPower;
        float innerRadius;
        float outerRadius;
        bool isEnabled;
        float pad2;
    };

    struct LightingData
    {
        Vector3	c_ambient;
        float pad;
        PointLightData c_pointLight[MAX_POINT_LIGHTS];
    };

    Game();
    ~Game();

    void Init(HWND hWnd, float width, float height);
    void Shutdown();
	void Update(float deltaTime);
    void RenderFrame();

	void OnKeyDown(uint32_t key);
	void OnKeyUp(uint32_t key);
	bool IsKeyHeld(uint32_t key) const;

    Graphics* GetGraphics() { return &mGraphics; }

    PointLightData* AllocateLight();
    void FreeLight(PointLightData* pLight);
    void SetAmbientLight(const Vector3& color) { mLightData.c_ambient = color; }
    const Vector3 &GetAmbientLight() const { return mLightData.c_ambient; }

    Shader* GetShader(const std::wstring& shaderName);
	Texture* Game::GetTexture(const std::wstring& textureName);
    Texture* LoadTexture(const std::wstring& fileName);
    Mesh* LoadMesh(const std::wstring& fileName);
    Skeleton* LoadSkeleton(const std::wstring& fileName);
    Animation* LoadAnimation(const std::wstring& fileName);

    void AddJob(JobManager::Job* pJob) { mJobs.AddJob(pJob); }

    Camera* GetCamera() { return mCamera; }

	void FullScreenPass(Shader* shader, Texture* texture);

    Physics mPhysics;

private:
	float mWidth;
	float mHeight;
	float mTopLeftX;
	float mTopLeftY;

	std::unordered_map<uint32_t, bool> m_keyIsHeld;
    Graphics mGraphics;

    Camera* mCamera;
    LightingData mLightData;
    ID3D11Buffer *mLightingBuffer;

    AssetCache<Shader> mShaderCache;
    AssetCache<Texture> mTextureCache;
    AssetCache<Mesh> mMeshCache;
    AssetCache<Skeleton> mSkeletonCache;
    AssetCache<Animation> mAnimationCache;
    
    std::vector<RenderObj*> mRenderObj;

	ID3D11RenderTargetView* mRTFull;
	ID3D11RenderTargetView* mRTHalf;
	ID3D11RenderTargetView* mRTQuarter1;
	ID3D11RenderTargetView* mRTQuarter2;
	VertexBuffer* mOffScreenBuff;
	ID3D11BlendState* mOpaqueBlend;
	ID3D11BlendState* mBloomBlend;

    JobManager mJobs;

	bool LoadLevel(const WCHAR* fileName);
};

struct VertexPosColor
{
    Vector3 pos;
    Graphics::Color4 color;
};

struct VertexPosColorUV
{
    Vector3 pos;
    Graphics::Color4 color;
    Vector2 uv;
};

struct VertexPosNormColorUV
{
    Vector3 pos;
    Vector3 norm;
    Graphics::Color4 color;
    Vector2 uv;
};

struct VertexPosNormUV
{
    Vector3 pos;
    Vector3 norm;
    Vector2 uv;
};

struct VertexPosNormBoneUV
{
    Vector3 pos;
    Vector3 norm;
    uint8_t boneIndex[4];
    uint8_t boneWeight[4];
    Vector2 uv;
};

struct VertexPosNormTangentUV
{
	Vector3 pos;
	Vector3 norm;
	Vector3 tangent;
	Vector2 uv;
};

struct VertexPosUV
{
	Vector3 pos;
	Vector2 uv;
};