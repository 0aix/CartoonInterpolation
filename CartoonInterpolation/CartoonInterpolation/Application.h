#pragma once

#include <d3dx9.h>
#include <string>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600

namespace Application
{
	extern IDirect3DDevice9* d3ddev;

	bool Initialize(HWND hwnd);
	void Uninitialize();

	bool LoadTexture(std::string path);
	bool LoadMesh(std::string path);

	void LeftDown(WPARAM wParam, LPARAM lParam);
	void LeftUp(WPARAM wParam, LPARAM lParam);
	void RightDown(WPARAM wParam, LPARAM lParam);
	void RightUp(WPARAM wParam, LPARAM lParam);
	void MouseMove(WPARAM wParam, LPARAM lParam);
	void KillFocus();

	void ToggleMesh();

	void Update();
	void Render();
}