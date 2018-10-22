#include "stdafx.h"
#include "Game.h"
#include "Animation.h"
#include "Camera.h"
#include "engineMath.h"
#include "Graphics.h"
#include "jsonUtil.h"
#include "mesh.h"
#include "Profiler.h"
#include "RenderCube.h"
#include "RenderObj.h"
#include "Shader.h"
#include "Skeleton.h"
#include "SkinnedObj.h"
#include "stringUtil.h"
#include "texture.h"
#include "VertexBuffer.h"
#include "Components\Character.h"
#include "Components\CollisionComponent.h"
#include "Components\followCam.h"
#include "Components\player.h"
#include "Components\PointLight.h"
#include "Components\SimpleRotate.h"
#include "rapidjson\include\rapidjson\rapidjson.h"
#include "rapidjson\include\rapidjson\document.h"
#include <fstream>
#include <sstream>


Game::Game()
    : mTopLeftX(0.0f)
	, mTopLeftY(0.0f)
	, mShaderCache(this)
    , mTextureCache(this)
    , mMeshCache(this)
    , mSkeletonCache(this)
    , mAnimationCache(this)
{
}

Game::~Game()
{
}

void Game::Init(HWND hWnd, float width, float height)
{
	mWidth = width;
	mHeight = height;

    mJobs.Begin();

    mGraphics.InitD3D(hWnd, width, height);
	mCamera = new Camera(&mGraphics);
    {
        VertexPosColor vert[] =
        {
            { Vector3(0.0f,   0.5f, 0.0f), Graphics::Color4(1.0f, 0.0f, 0.0f, 1.0f) },
            { Vector3(0.45f, -0.5,  0.0f), Graphics::Color4(0.0f, 1.0f, 0.0f, 1.0f) },
            { Vector3(-0.45f, -0.5f, 0.0f), Graphics::Color4(0.0f, 0.0f, 1.0f, 1.0f) }
        };
        uint16_t index[] = { 0, 1, 2 };

        // TODO Lab 02f
        D3D11_INPUT_ELEMENT_DESC inputElem[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosColor, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        Shader *pShader = new Shader(&mGraphics);
        pShader->Load(L"Shaders/Mesh.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
        mShaderCache.Cache(L"Mesh", pShader);

        VertexBuffer* pVertBuff = new VertexBuffer(&mGraphics,
            vert, sizeof(vert) / sizeof(vert[0]), sizeof(vert[0]),
            index, sizeof(index) / sizeof(index[0]), sizeof(index[0])
            );
        Mesh* pMesh = new Mesh(this, pVertBuff, mShaderCache.Get(L"Mesh"));
        mMeshCache.Cache(L"Triangle", pMesh);
        //RenderObj* pObj = new RenderObj(this, pMesh);
        //mRenderObj.push_back(pObj);
    }
    {
        D3D11_INPUT_ELEMENT_DESC inputElem[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormColorUV, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormColorUV, norm), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormColorUV, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormColorUV, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        Shader* pShader = new Shader(&mGraphics);
        pShader->Load(L"Shaders/BasicMesh.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
        mShaderCache.Cache(L"BasicMesh", pShader);
    }
    {
        D3D11_INPUT_ELEMENT_DESC inputElem[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormUV, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormUV, norm), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormUV, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        Shader* pPhong = new Shader(&mGraphics);
        pPhong->Load(L"Shaders/Phong.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
        mShaderCache.Cache(L"Phong", pPhong);
        Shader* pUnlit = new Shader(&mGraphics);
        pUnlit->Load(L"Shaders/Unlit.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
        mShaderCache.Cache(L"Unlit", pUnlit);
    }
    {
        D3D11_INPUT_ELEMENT_DESC inputElem[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormBoneUV, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormBoneUV, norm), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BONES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, offsetof(VertexPosNormBoneUV, boneIndex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "WEIGHTS", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexPosNormBoneUV, boneWeight), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormBoneUV, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        Shader* pShader = new Shader(&mGraphics);
        pShader->Load(L"Shaders/Skinned.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
        mShaderCache.Cache(L"Skinned", pShader);
    }
	{
		D3D11_INPUT_ELEMENT_DESC inputElem[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormTangentUV, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormTangentUV, norm), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormTangentUV, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormTangentUV, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		Shader* pShader = new Shader(&mGraphics);
		pShader->Load(L"Shaders/Normal.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
		mShaderCache.Cache(L"Normal", pShader);
	}
	{
		// Post effect shaders
		D3D11_INPUT_ELEMENT_DESC inputElem[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosUV, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosUV, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		Shader* pShader = new Shader(&mGraphics);
		pShader->Load(L"Shaders/Copy.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
		mShaderCache.Cache(L"Copy", pShader);

		pShader = new Shader(&mGraphics);
		pShader->Load(L"Shaders/BloomMask.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
		mShaderCache.Cache(L"BloomMask", pShader);
	
		pShader = new Shader(&mGraphics);
		pShader->Load(L"Shaders/HorizontalPass.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
		mShaderCache.Cache(L"HorizontalPass", pShader);

		pShader = new Shader(&mGraphics);
		pShader->Load(L"Shaders/VerticalPass.hlsl", inputElem, sizeof(inputElem) / sizeof(inputElem[0]));
		mShaderCache.Cache(L"VerticalPass", pShader);
	}

	LoadLevel(L"Assets/Levels/Level10.itplevel");

    mLightingBuffer = mGraphics.CreateGraphicsBuffer(&mLightData, sizeof(mLightData), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, D3D11_USAGE_DYNAMIC);

	// Create the off-screen buffer
	int texWidth = mGraphics.GetScreenWidth();
	int texHeight = mGraphics.GetScreenHeight();
	Texture* texture = new Texture(&mGraphics);
	mRTFull = texture->CreateRenderTarget(texWidth, texHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mTextureCache.Cache(L"FullOSTexture", texture);
	// Half-by-half sized target
	texture = new Texture(&mGraphics);
	mRTHalf = texture->CreateRenderTarget(texWidth/2, texHeight/2, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mTextureCache.Cache(L"HalfOSTexture", texture);
	// Two quarter-by-quarter sized targets
	texture = new Texture(&mGraphics);
	mRTQuarter1 = texture->CreateRenderTarget(texWidth/4, texHeight/4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mTextureCache.Cache(L"QuarterOSTexture1", texture);
	texture = new Texture(&mGraphics);
	mRTQuarter2 = texture->CreateRenderTarget(texWidth/4, texHeight/4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mTextureCache.Cache(L"QuarterOSTexture2", texture);

	{   
		// create blend states
		mOpaqueBlend = mGraphics.CreateBlendState(false, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ZERO);
		mBloomBlend = mGraphics.CreateBlendState(true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);
	}

	VertexPosUV offScreenVert[] = {
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)}
	};
	uint16_t offScreenIndex[] = { 
		3, 0, 1,
		3, 1, 2
	
	};

	mOffScreenBuff = new VertexBuffer(&mGraphics, offScreenVert, sizeof(offScreenVert) / sizeof(offScreenVert[0]), sizeof(VertexPosUV),
		offScreenIndex, sizeof(offScreenIndex) / sizeof(offScreenIndex[0]), sizeof(offScreenIndex[0]));
}

void Game::Shutdown()
{
	mJobs.End();
	for (RenderObj* pObj : mRenderObj)
		delete pObj;
	mRenderObj.clear();
	mLightingBuffer->Release();
	delete mCamera;
	mAnimationCache.Clear();
	mSkeletonCache.Clear();
	mMeshCache.Clear();
	mTextureCache.Clear();
	mShaderCache.Clear();
	mGraphics.CleanD3D();
	delete mOffScreenBuff;
	mRTFull->Release();
	mRTHalf->Release();
	mRTQuarter1->Release();
	mRTQuarter2->Release();
	mBloomBlend->Release();
	mOpaqueBlend->Release();
}

class TestJob : public JobManager::Job
{
public:
    void DoIt() override
    {
        printf("Hello World\n");
    }
};
static TestJob s_testJob;

void Game::Update(float deltaTime)
{
//    mJobs.AddJob(&s_testJob);
    for (RenderObj* pObj : mRenderObj)
    {
        pObj->Update(deltaTime);
    }
}

void Game::FullScreenPass(Shader* shader, Texture* texture) {
	shader->SetActive();
	texture->SetActive(Graphics::TEXTURE_SLOT_DIFFUSE);
	mOffScreenBuff->SetActive();
	mOffScreenBuff->Draw();
}

void Game::RenderFrame()
{
	{
		PROFILE_SCOPE(WaitForJobs);
		mJobs.WaitForJobs();
	}
	
	// Set up off-screen buffer
	mGraphics.SetRenderTarget(mRTFull, mGraphics.GetDepthBuffer());

    {
        PROFILE_SCOPE(Render_SetUp);
        Graphics::Color4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
        mGraphics.BeginFrame(clearColor);

		mGraphics.SetBlendState(mOpaqueBlend);

        mCamera->SetActive();
        mGraphics.UploadBuffer(mLightingBuffer, &mLightData, sizeof(mLightData));
        mGraphics.GetDeviceContext()->PSSetConstantBuffers(Graphics::CONSTANT_BUFFER_LIGHTING, 1, &mLightingBuffer);
    }
    {
        PROFILE_SCOPE(Render_Objects);
        for (RenderObj* pObj : mRenderObj)
            pObj->Draw();
    }

	float width = mGraphics.GetScreenWidth();
	float height = mGraphics.GetScreenHeight();

	// Draw BloomMask at half resolution
	mGraphics.SetRenderTarget(mRTHalf, nullptr);
	mGraphics.SetViewport(mTopLeftX, mTopLeftY, width / 2, height / 2);
	FullScreenPass(GetShader(L"BloomMask"), GetTexture(L"FullOSTexture"));
	//mGraphics.SetActiveTexture(Graphics::TEXTURE_SLOT_DIFFUSE, nullptr);

	// Copy half resolution BloomMask to quarter1 resolution
	mGraphics.SetRenderTarget(mRTQuarter1, nullptr);
	mGraphics.SetViewport(mTopLeftX, mTopLeftY, width / 4, height / 4);
	FullScreenPass(GetShader(L"Copy"), GetTexture(L"HalfOSTexture"));
	//mGraphics.SetActiveTexture(Graphics::TEXTURE_SLOT_DIFFUSE, nullptr);

	// Horizontal Pass from quarter1 to quarter2
	mGraphics.SetRenderTarget(mRTQuarter2, nullptr);
	mGraphics.SetViewport(mTopLeftX, mTopLeftY, width / 4, height / 4);
	FullScreenPass(GetShader(L"HorizontalPass"), GetTexture(L"QuarterOSTexture1"));
	
	mGraphics.SetActiveTexture(Graphics::TEXTURE_SLOT_DIFFUSE, nullptr);
	// VerticalPass Pass from quarter2 to quarter1
	mGraphics.SetRenderTarget(mRTQuarter1, nullptr);
	mGraphics.SetViewport(mTopLeftX, mTopLeftY, width / 4, height / 4);
	FullScreenPass(GetShader(L"VerticalPass"), GetTexture(L"QuarterOSTexture2"));
	//mGraphics.SetActiveTexture(Graphics::TEXTURE_SLOT_DIFFUSE, nullptr);

	// Copy original off-screen buffer to the back buffer
	mGraphics.SetRenderTarget(mGraphics.GetBackBuffer(), nullptr);
	mGraphics.SetViewport(mTopLeftX, mTopLeftY, width, height);
	FullScreenPass(GetShader(L"Copy"), GetTexture(L"FullOSTexture"));

	// Copy quarter resolution BloomMask to the back buffer for presentation
	mGraphics.SetBlendState(mBloomBlend);
	FullScreenPass(GetShader(L"Copy"), GetTexture(L"QuarterOSTexture1"));

	// Reset the active texture for next iteration
	//mGraphics.SetActiveTexture(Graphics::TEXTURE_SLOT_DIFFUSE, nullptr);

    {
        PROFILE_SCOPE(Render_End);
        mGraphics.EndFrame();
    }
}

void Game::OnKeyDown(uint32_t key)
{
	m_keyIsHeld[key] = true;
}

void Game::OnKeyUp(uint32_t key)
{
	m_keyIsHeld[key] = false;
}

bool Game::IsKeyHeld(uint32_t key) const
{
	const auto find = m_keyIsHeld.find(key);
	if (find != m_keyIsHeld.end())
		return find->second;
	return false;
}

Game::PointLightData* Game::AllocateLight()
{
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if (false == mLightData.c_pointLight[i].isEnabled)
        {
            mLightData.c_pointLight[i].isEnabled = true;
            return &mLightData.c_pointLight[i];
        }
    }
    return nullptr;
}

void Game::FreeLight(PointLightData* pLight)
{
    pLight->isEnabled = false;
}

Shader* Game::GetShader(const std::wstring& shaderName)
{
    return mShaderCache.Get(shaderName);
}

Texture* Game::GetTexture(const std::wstring& textureName) {
	return mTextureCache.Get(textureName);
}

Texture* Game::LoadTexture(const std::wstring& fileName)
{
    return mTextureCache.Load(fileName);
}

Mesh* Game::LoadMesh(const std::wstring& fileName)
{
    return mMeshCache.Load(fileName);
}

Skeleton* Game::LoadSkeleton(const std::wstring& fileName)
{
    return mSkeletonCache.Load(fileName);
}

Animation* Game::LoadAnimation(const std::wstring& fileName)
{
    return mAnimationCache.Load(fileName);
}

bool Game::LoadLevel(const WCHAR* fileName)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		return false;
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		return false;
	}

	std::string str = doc["metadata"]["type"].GetString();
	int ver = doc["metadata"]["version"].GetInt();

	// Check the metadata
	if (!doc["metadata"].IsObject() ||
		str != "itplevel" ||
		ver != 2)
	{
		return false;
	}

    const rapidjson::Value& camera = doc["camera"];
    if (camera.IsObject())
    {
        Vector3 pos;
        GetVectorFromJSON(camera, "position", pos);
        Quaternion rot;
        GetQuaternionFromJSON(camera, "rotation", rot);
        Matrix4 mat = Matrix4::CreateFromQuaternion(rot) * Matrix4::CreateTranslation(pos);
        mat.Invert();
        mCamera->SetViewMat(mat);
    }

    const rapidjson::Value& lighting = doc["lightingData"];
    if (lighting.IsObject())
    {
        Vector3 light;
        GetVectorFromJSON(lighting, "ambient", light);
        SetAmbientLight(light);
    }

    const rapidjson::Value& renderObjects = doc["renderObjects"];
    if (renderObjects.IsArray())
    {
        for (rapidjson::SizeType i = 0; i < renderObjects.Size(); ++i)
        {
            const rapidjson::Value& obj = renderObjects[i];
            Vector3 pos;
            GetVectorFromJSON(obj, "position", pos);
            Quaternion rot;
            GetQuaternionFromJSON(obj, "rotation", rot);
            float scale;
            GetFloatFromJSON(obj, "scale", scale);
            std::wstring mesh;
            GetWStringFromJSON(obj, "mesh", mesh);
            Mesh* pMesh = LoadMesh(mesh);
            RenderObj* pObj = nullptr;
            SkinnedObj* pSkin = nullptr;
            if (nullptr != pMesh && pMesh->IsSkinned())
                pObj = pSkin = new SkinnedObj(this, pMesh);
            else
                pObj = new RenderObj(this, pMesh);
            pObj->mObjectData.c_modelToWorld = 
                Matrix4::CreateScale(scale)
                * Matrix4::CreateFromQuaternion(rot)
                * Matrix4::CreateTranslation(pos);

            const rapidjson::Value& components = obj["components"];
            if (components.IsArray())
            {
                for (rapidjson::SizeType j = 0; j < components.Size(); ++j)
                {
                    Component* pComp = nullptr;
                    const rapidjson::Value& comp = components[j];
                    std::string type;
                    GetStringFromJSON(comp, "type", type);
                    if (type == "PointLight")
                    {
                        pComp = new PointLight(pObj);
                    }
                    else if (type == "Character")
                    {
                        pComp = new Character(pSkin);
                    }
                    else if (type == "Player")
                    {
                        pComp = new Player(pSkin);
                    }
                    else if (type == "Collision")
                    {
                        pComp = new CollisionComponent(pObj);
                    }
                    else if (type == "FollowCam")
                    {
                        pComp = new FollowCam(pObj);
                    }
					else if (type == "SimpleRotate") {
						pComp = new SimpleRotate(pObj);

					}
                    if (nullptr != pComp)
                    {
                        pComp->LoadProperties(comp);
                        pObj->AddComponent(pComp);
                    }
                }
            }

            mRenderObj.push_back(pObj);
        }
    }


	return true;
}