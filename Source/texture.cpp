#include "stdafx.h"
#include "texture.h"
#include "game.h"
#include "Graphics.h"
#include "DirectXTK\Inc\DDSTextureLoader.h"
#include "DirectXTK\Inc\WICTextureLoader.h"
#include <string>

Texture::Texture(Graphics *graphics)
    : mGraphics(graphics)
    , mResource(nullptr)
    , mView(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

Texture::~Texture()
{
    Free();
}

void Texture::Free()
{
    if (nullptr != mView)
    {
        mView->Release();
        mView = nullptr;
    }
    if (nullptr != mResource)
    {
        mResource->Release();
        mResource = nullptr;
    }
    mWidth = 0;
    mHeight = 0;
}

bool Texture::Load(const WCHAR* fileName)
{
    Free();     // in case there was already a texture loaded here, release it

    ID3D11Device *pDev = mGraphics->GetDevice();

    std::wstring fileStr(fileName);
    std::wstring extension = fileStr.substr(fileStr.find_last_of('.'));
    HRESULT hr = -1;
    if (extension == L".dds" || extension == L".DDS")
        hr = DirectX::CreateDDSTextureFromFile(pDev, fileName, &mResource, &mView);
    else
        hr = DirectX::CreateWICTextureFromFile(pDev, fileName, &mResource, &mView);
    DbgAssert(hr == S_OK, "Problem Creating Texture From File");
    if (S_OK != hr)
        return false;

    CD3D11_TEXTURE2D_DESC textureDesc;
    ((ID3D11Texture2D*)mResource)->GetDesc(&textureDesc);
    mWidth = textureDesc.Width;
    mHeight = textureDesc.Height;

    return true;
}

#if 1	// TODO Lab 04c
Texture* Texture::StaticLoad(const WCHAR* fileName, Game* pGame)
{
    Texture* pTex = new Texture(pGame->GetGraphics());
    if (false == pTex->Load(fileName))
    {
        delete pTex;
        return nullptr;
    }
    return pTex;
}
#endif

#if 1	// TODO Lab 03k
void Texture::SetActive(int slot) const
{
	mGraphics->SetActiveTexture(slot, mView);
}
#endif

ID3D11RenderTargetView* Texture::CreateRenderTarget(int inWidth, int inHeight, DXGI_FORMAT format) {
	ID3D11Device* dev = mGraphics->GetDevice();

	// Create the 2D texture for mResource
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC descTexture;
	ZeroMemory(&descTexture, sizeof(descTexture));
	descTexture.Width = inWidth;
	descTexture.Height = inHeight;
	descTexture.MipLevels = 1;
	descTexture.ArraySize = 1;
	descTexture.Format = format;
	descTexture.SampleDesc.Count = 1;
	descTexture.Usage = D3D11_USAGE_DEFAULT;
	descTexture.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HRESULT hr = dev->CreateTexture2D(&descTexture, nullptr, &texture);
	mResource = texture;

	// Initialize the ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC descShader;
	ZeroMemory(&descShader, sizeof(descShader));
	descShader.Format = format;
	descShader.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	descShader.Texture2D.MipLevels = 1;
	hr = dev->CreateShaderResourceView(mResource, &descShader, &mView);

	// Return the RenderTargetView
	ID3D11RenderTargetView* renderTarget;
	D3D11_RENDER_TARGET_VIEW_DESC descRender;
	ZeroMemory(&descRender, sizeof(descRender));
	descRender.Format = format;
	descRender.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = dev->CreateRenderTargetView(mResource, &descRender, &renderTarget);

	return renderTarget;
}
