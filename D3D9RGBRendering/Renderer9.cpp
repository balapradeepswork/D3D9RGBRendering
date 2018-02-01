#include "stdafx.h"
#include "Renderer9.h"


Renderer9::Renderer9()
{
}


Renderer9::~Renderer9()
{
}

HRESULT	Renderer9::InitD3D(UINT width, UINT height, HWND hWnd)
{
	D3DDISPLAYMODE	ddm;

	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		std::cout << "Unable to Create Direct3D\n";
		return E_FAIL;
	}

	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))
	{
		std::cout << "Unable to Get Adapter Display Mode\n";
		return E_FAIL;
	}

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.Windowed = true;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat = ddm.Format;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferWidth = width;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
	{
		std::cout << "Unable to Create Device\n";
		return E_FAIL;
	}

	ghWnd = hWnd;

	if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &g_pSurface, NULL)))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	D3DLOCKED_RECT	lockedRect;

	if (FAILED(g_pSurface->LockRect(&lockedRect, NULL, 0)))					// compute the required buffer size
	{
		std::cout << "Unable to Lock Surface\n";
		return E_FAIL;
	}
	gPitch = lockedRect.Pitch;
	if (FAILED(g_pSurface->UnlockRect()))
	{
		std::cout << "Unable to Unlock Surface\n";
		return E_FAIL;
	}

	return S_OK;
}

UINT Renderer9::GetPitch()
{
	return gPitch;
}

UINT Renderer9::GetHeight()
{
	return gScreenRect.bottom;
}

UINT Renderer9::GetWidth()
{
	return gScreenRect.right;
}

//
// Indicates that window has been resized.
//
void Renderer9::WindowResize()
{
	g_NeedsResize = true;
}

HRESULT Renderer9::Fill32bppImageToSurface(RGBFrame* rgbFrame)
{
	IDirect3DSurface9 * pBackBuffer = NULL;
	if (FAILED(g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		std::cout << "Unable to get BackBuffer.\n";
		return E_FAIL;
	}
	D3DLOCKED_RECT d3d_rect;
	if (FAILED(pBackBuffer->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT)))
	{
		std::cout << "Unable to Lock BackBuffer.\n";
		return E_FAIL;
	}
	
	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;

	for (int i = 0; i < rgbFrame->height; i++)
	{
		if (memcpy_s(pDest + i * stride, rgbFrame->pitch, rgbFrame->data + (rgbFrame->height - 1 - i) * rgbFrame->pitch, rgbFrame->width * 4))
		{
			std::cout << "Unable to fill BackBuffer with the image data.\n";
			return E_FAIL;
		}
	}

	if (FAILED(pBackBuffer->UnlockRect()))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	pBackBuffer->Release();

	return S_OK;
}


HRESULT Renderer9::Fill24bppImageToSurface(RGBFrame* rgbFrame)
{
	IDirect3DSurface9 * pBackBuffer = NULL;
	if (FAILED(g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		std::cout << "Unable to get BackBuffer.\n";
		return E_FAIL;
	}
	D3DLOCKED_RECT d3d_rect;
	if (FAILED(pBackBuffer->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT)))
	{
		std::cout << "Unable to Lock BackBuffer.\n";
		return E_FAIL;
	}

	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;

	for (int h = 0; h < rgbFrame->height; h++)
	{
		for (int w = 0; w < rgbFrame->width; w++)
		{
			if (memcpy_s(pDest + h * stride + w * 4, 3, rgbFrame->data + (rgbFrame->height - 1 - h) * rgbFrame->pitch + w * 3, 3))
			{
				std::cout << "Unable to fill BackBuffer with the image data.\n";
				return E_FAIL;
			}
		}
		
	}

	if (FAILED(pBackBuffer->UnlockRect()))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	pBackBuffer->Release();

	return S_OK;
}

HRESULT Renderer9::Render()
{
	HRESULT	hr;
	if (g_pd3dDevice)
	{
		hr = g_pd3dDevice->TestCooperativeLevel();//Check Device Status - if Alt+tab or some such thing have caused any trouble
		if (hr != D3D_OK)
		{
			if (hr == D3DERR_DEVICELOST)	return hr;	//Device is lost - Do not render now 
			if (hr == D3DERR_DEVICENOTRESET)		//Device is ready to be acquired
			{
				if (FAILED(Reset()))
				{
					DestroyWindow(ghWnd);		//If Unable to Reset Device - Close the Application
					return hr;
				}
			}
		}
		g_pd3dDevice->BeginScene();
		g_pd3dDevice->EndScene();
		g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
	}

	return S_OK;
}

HRESULT Renderer9::Reset()
{
	D3DDISPLAYMODE	ddm;

	if (g_pSurface)															//Release the Surface - we need to get the latest surface
	{
		g_pSurface->Release();
		g_pSurface = NULL;
	}

	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))	//User might have changed the mode - Get it afresh
	{
		std::cout << "Unable to Get Adapter Display Mode";
		return E_FAIL;
	}

	HRESULT hr = g_pd3dDevice->Reset(&d3dpp);
	if (FAILED(hr))
	{
		std::cout << "Unable to Reset Device";
		return E_FAIL;
	}

	if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &g_pSurface, NULL)))
	{
		std::cout << "Unable to Recreate Surface";
		return E_FAIL;
	}
	g_NeedsResize = false;
	return S_OK;
}

void Renderer9::Cleanup()
{
	if (g_pSurface)
	{
		g_pSurface->Release();
		g_pSurface = NULL;
	}
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}
	if (g_pD3D)
	{
		g_pD3D->Release();
		g_pD3D = NULL;
	}
}