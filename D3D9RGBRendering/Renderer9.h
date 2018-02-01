#ifndef _RENDERER9_H_
#define _RENDERER9_H_

#include <d3d9.h>
#include <iostream>

class RGBFrame
{
public:
	UINT width;
	UINT height;
	UINT pitch;
	BYTE* data;
};

class Renderer9
{
private:
	//vars
	IDirect3D9*			g_pD3D = NULL;
	IDirect3DDevice9*	g_pd3dDevice = NULL;
	IDirect3DSurface9*	g_pSurface = NULL;
	HWND				ghWnd = NULL;
	bool				g_NeedsResize = false;
	RECT				gScreenRect = { 0,0,0,0 };
	UINT				gPitch = 0;
	D3DPRESENT_PARAMETERS	d3dpp;

public:
	//methods
	Renderer9();
	~Renderer9();
	HRESULT	InitD3D(UINT width, UINT height, HWND hWnd = NULL);
	UINT GetPitch();
	UINT GetHeight();
	UINT GetWidth();
	void WindowResize();
	HRESULT Fill32bppImageToSurface(RGBFrame* rgbFrame);
	HRESULT Fill24bppImageToSurface(RGBFrame* rgbFrame);
	HRESULT Render();
	HRESULT Reset();
	void Cleanup();
};


#endif // !_RENDERER9_H_