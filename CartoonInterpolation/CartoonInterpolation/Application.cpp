#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <windowsx.h>

#include "LBS.h"

namespace Application
{
	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;
	ID3DXSprite* sprite = NULL;
	IDirect3DTexture9* texture = NULL;

	D3DSURFACE_DESC desc;

	std::vector<VERTEX_2D_TEX> vertices;
	std::vector<WORD> indices;
	std::vector<WORD> mesh;
	UINT numFaces;
	UINT verticesSize;

	IDirect3DVertexBuffer9* vertex_buffer = NULL;
	IDirect3DIndexBuffer9* index_buffer = NULL;
	IDirect3DIndexBuffer9* mesh_buffer = NULL;

	LBS lbs;

	int state = -1;
	int mouse_state = 0; // 0 = nothing, 1 = left down, 2 = right down, no both down
	POINT pos;
	POINT last_pos;

	bool show_mesh = true;

	bool Initialize(HWND hwnd)
	{
		d3d = Direct3DCreate9(D3D_SDK_VERSION);

		if (!d3d)
		{
			std::cout << "Direct3D9 unavailable\n";
			return false;
		}

		D3DPRESENT_PARAMETERS d3dpp = { 0 };

		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hwnd;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
		//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev)))
		{
			std::cout << "Failed to create Direct3D device\n";
			return false;
		}

		if (FAILED(D3DXCreateSprite(d3ddev, &sprite)))
		{
			std::cout << "Failed to create Direct3D sprite device\n";
			return false;
		}

		// default alpha blend states 
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// disable culling
		d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		// set linear interpolation
		d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		lbs.draw = new Draw();

		return true;
	}

	void Uninitialize()
	{
		if (texture)
			texture->Release();
		if (vertex_buffer)
			vertex_buffer->Release();
		if (index_buffer)
			index_buffer->Release();

		delete lbs.draw;

		sprite->Release();
		d3ddev->Release();
		d3d->Release();
	}

	bool LoadTexture(std::string path)
	{
		if (FAILED(D3DXCreateTextureFromFileExA(d3ddev, path.c_str(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, D3DX_FROM_FILE, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN,
			D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &texture)))
		{
			std::cout << "Failed to load png as texture\n";
			return false;
		}

		texture->GetLevelDesc(0, &desc);

		if (desc.Width > SCREEN_WIDTH || desc.Height > SCREEN_HEIGHT)
		{
			std::cout << "Image dimensions are too large\n";
			return false;
		}

		return true;
	}

	/**
	No error-checking. Assumes valid obj with only 'v' and 'f'
	**/
	bool LoadMesh(std::string path)
	{
		std::ifstream ifs(path);

		VERTEX_2D_TEX vert;
		VERTEX_2D_DIF vert_dif;
		float z;

		float w = desc.Width;
		float h = desc.Height;

		WORD v0, v1, v2;

		while (!ifs.eof())
		{
			char c;
			ifs >> c;

			if (ifs.good())
			{
				switch (c)
				{
				case 'v':
					ifs >> vert.x >> vert.y >> z;
					vert.y = h - vert.y;
					vert.u = vert.x / w;
					vert.v = vert.y / h;
					vertices.push_back(vert);
					break;
				case 'f':
					// obj vertices are not 0-indexed
					ifs >> v0 >> v1 >> v2;
					indices.push_back(v0 - 1);
					indices.push_back(v1 - 1);
					indices.push_back(v2 - 1);
					mesh.push_back(v0 - 1);
					mesh.push_back(v1 - 1);
					mesh.push_back(v1 - 1);
					mesh.push_back(v2 - 1);
					mesh.push_back(v2 - 1);
					mesh.push_back(v0 - 1);
					break;
				default:
					std::cout << "obj format incorrect\n";
					return false;
				}
			}
		}
		ifs.close();

		if (vertices.size() > 65536)
		{
			std::cout << "Too many vertices in mesh\n";
			return false;
		}

		numFaces = indices.size() / 3;

		// Create/populate vertex and index buffers
		if (FAILED(d3ddev->CreateVertexBuffer(vertices.size() * sizeof(VERTEX_2D_TEX), D3DUSAGE_WRITEONLY, VERTEX_2D_TEX::TEX2DFVF, D3DPOOL_DEFAULT, &vertex_buffer, NULL)) ||
			FAILED(d3ddev->CreateIndexBuffer(indices.size() * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &index_buffer, NULL)) ||
			FAILED(d3ddev->CreateIndexBuffer(mesh.size() * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &mesh_buffer, NULL)))
		{
			std::cout << "Failed to create vertex and/or index buffers\n";
			return false;
		}

		void* buffer;

		verticesSize = vertices.size() * sizeof(VERTEX_2D_TEX);
		if (FAILED(vertex_buffer->Lock(0, verticesSize, &buffer, 0)))
		{
			std::cout << "Failed to secure vertex buffer\n";
			return false;
		}
		memcpy(buffer, vertices.data(), verticesSize);
		vertex_buffer->Unlock();

		UINT size = indices.size() * sizeof(WORD);
		if (FAILED(index_buffer->Lock(0, size, &buffer, 0)))
		{
			std::cout << "Failed to secure index buffer\n";
			return false;
		}
		memcpy(buffer, indices.data(), size);
		index_buffer->Unlock();

		size = mesh.size() * sizeof(WORD);
		if (FAILED(mesh_buffer->Lock(0, size, &buffer, 0)))
		{
			std::cout << "Failed to secure mesh buffer\n";
			return false;
		}
		memcpy(buffer, mesh.data(), size);
		mesh_buffer->Unlock();

		state = 0;

		return true;
	}

	void LeftDown(WPARAM wParam, LPARAM lParam)
	{
		last_pos = pos;
		pos.x = GET_X_LPARAM(lParam);
		pos.y = GET_Y_LPARAM(lParam);

		if (~(MK_RBUTTON & wParam))
		{
			switch (state)
			{
			case 0:
				lbs.SelectPoint(pos.x, pos.y, MK_SHIFT & wParam, MK_CONTROL & wParam);
				break;
			case 1:
				lbs.SelectPoint(pos.x, pos.y, false);
				break;
			default:
				break;
			}
		}
	}

	void LeftUp(WPARAM wParam, LPARAM lParam)
	{
		last_pos = pos;
		pos.x = GET_X_LPARAM(lParam);
		pos.y = GET_Y_LPARAM(lParam);

		void* buffer;

		switch (state)
		{
		case 0:
			lbs.DragPoint(pos.x - last_pos.x, pos.y - last_pos.y, pos.x, pos.y);
			lbs.ReleasePoint();
			break;
		case 1:
			lbs.DragPoint(pos.x - last_pos.x, pos.y - last_pos.y, pos.x, pos.y);
			lbs.ReleasePoint();

			vertex_buffer->Lock(0, verticesSize, &buffer, 0);
			lbs.UpdatePositions((VERTEX_2D_TEX*)buffer, vertices.data(), vertices.size());
			vertex_buffer->Unlock();
			break;
		default:
			break;
		}
	}

	void RightDown(WPARAM wParam, LPARAM lParam)
	{
		last_pos = pos;
		pos.x = GET_X_LPARAM(lParam);
		pos.y = GET_Y_LPARAM(lParam);

		if (~(MK_LBUTTON & wParam))
		{
			switch (state)
			{
			case 1:
				lbs.SelectPoint(pos.x, pos.y, true);
				break;
			default:
				break;
			}
		}
	}

	void RightUp(WPARAM wParam, LPARAM lParam)
	{
		last_pos = pos;
		pos.x = GET_X_LPARAM(lParam);
		pos.y = GET_Y_LPARAM(lParam);

		void* buffer;

		switch (state)
		{
		case 1:
			lbs.DragPoint(pos.x - last_pos.x, pos.y - last_pos.y, pos.x, pos.y);
			lbs.ReleasePoint();

			vertex_buffer->Lock(0, verticesSize, &buffer, 0);
			lbs.UpdatePositions((VERTEX_2D_TEX*)buffer, vertices.data(), vertices.size());
			vertex_buffer->Unlock();
			break;
		default:
			break;
		}
	}

	void MouseMove(WPARAM wParam, LPARAM lParam)
	{
		last_pos = pos;
		pos.x = GET_X_LPARAM(lParam);
		pos.y = GET_Y_LPARAM(lParam);

		void* buffer;

		switch (state)
		{
		case 0:
			lbs.DragPoint(pos.x - last_pos.x, pos.y - last_pos.y, pos.x, pos.y);
			break;
		case 1:
			lbs.DragPoint(pos.x - last_pos.x, pos.y - last_pos.y, pos.x, pos.y);
			vertex_buffer->Lock(0, verticesSize, &buffer, 0);
			lbs.UpdatePositions((VERTEX_2D_TEX*)buffer, vertices.data(), vertices.size());
			vertex_buffer->Unlock();
			break;
		default:
			break;
		}
	}

	void KillFocus()
	{
		lbs.DeselectAll();
	}

	void ToggleMesh()
	{
		show_mesh = !show_mesh;
	}

	void Update()
	{
		if (state == 0)
		{
			lbs.CalculateWeights(vertices, indices);
			lbs.pose = true;
			state = 1;
		}
	}

	void Render()
	{
		d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);
		d3ddev->BeginScene();

		// Draw image
		d3ddev->SetFVF(VERTEX_2D_TEX::TEX2DFVF);
		d3ddev->SetStreamSource(0, vertex_buffer, 0, sizeof(VERTEX_2D_TEX));
		d3ddev->SetIndices(index_buffer);
		d3ddev->SetTexture(0, texture);

		d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertices.size(), 0, numFaces);

		// Draw mesh - renders white lines since texture isn't set
		if (show_mesh)
		{
			d3ddev->SetIndices(mesh_buffer);
			d3ddev->SetTexture(0, NULL);

			d3ddev->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, vertices.size(), 0, numFaces * 3);
		}

		// Draw controls
		lbs.Render();

		d3ddev->EndScene();
		d3ddev->Present(NULL, NULL, NULL, NULL);
	}
}