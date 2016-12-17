#pragma once

#include <Windows.h>
#include <vector>
#include <d3dx9.h>

#include "Draw.h"

#define HANDLE_RADIUS 6.0f

struct CONTROL_HANDLE
{
	D3DXVECTOR2 pos;
	int k = -1; // index in point_handles
	float rotation = 0.0f;
};

struct POINT_HANDLE
{
	int p;
	std::vector<float> weights;
};

struct BONE_EDGE
{
	int p0;
	int p1;
	float length;
	std::vector<float> weights;
};

class LBS
{
public:
	void AddPointHandle(int x, int y);
	void RotatePointHandle(float f);

	void SetBoneRoot(int x, int y);
	void AddBoneEdge(int x, int y);

	int HoverPoint(int x, int y);
	void SelectPoint(int x, int y, bool right);
	void SelectPoint(int x, int y, bool ph, bool be);
	void DragPoint(int x, int y, int ax, int ay);
	void ReleasePoint();
	void DeselectAll();

	void CalculateWeights(std::vector<VERTEX_2D_TEX> vertices, std::vector<WORD> indices);
	void UpdatePositions(VERTEX_2D_TEX* buffer, VERTEX_2D_TEX* data, int size);

	void Render();

	Draw* draw = NULL;
	bool pose = false;

private:
	std::vector<CONTROL_HANDLE> control_handles;
	std::vector<CONTROL_HANDLE> var_handles;
	std::vector<POINT_HANDLE> point_handles;
	std::vector<BONE_EDGE> bone_edges;

	int p = -1;
	bool dragging = false;
	bool rotating = false;

	D3DXVECTOR2 bone_root;
	bool set_bone_root = false;

	D3DXVECTOR2 bone_joint;
	bool set_bone_joint = false;
};