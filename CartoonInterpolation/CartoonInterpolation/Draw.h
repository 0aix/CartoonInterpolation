#pragma once

#include <d3dx9.h>
#include <vector>

struct VERTEX_2D_DIF
{
	float x, y, z = 0.0f, rhw = 1.0f;
	DWORD color = 0xFFFFFFFF;

	static const DWORD DIF2DFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
};

struct VERTEX_2D_TEX
{
	float x, y, z = 0.0f, rhw = 1.0f;
	float u, v;

	static const DWORD TEX2DFVF = D3DFVF_XYZRHW | D3DFVF_TEX1;
};

class Draw
{
public:
	Draw();
	~Draw();

	void DrawCircle(D3DXVECTOR2 center, float radius, DWORD color);
	void DrawSolidCircle(D3DXVECTOR2 center, float radius, DWORD color);
	void DrawSegment(D3DXVECTOR2 p1, D3DXVECTOR2 p2, DWORD color);

	void Render();

private:
	static const int MAX_VERTICES = 1024;

	IDirect3DDevice9* d3ddev;
	VERTEX_2D_DIF vertices[MAX_VERTICES];

	struct DPI //DrawPrimitive Info
	{
		D3DPRIMITIVETYPE PrimitiveType;
		UINT StartVertex;
		UINT PrimitiveCount;
	};
	IDirect3DVertexBuffer9* vertex_buffer;
	std::vector<DPI> dpi_list;
	int vertex_count = 0;
};