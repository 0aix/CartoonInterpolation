#include "stdafx.h"

Draw::Draw()
{
	d3ddev = Application::d3ddev;

	if (FAILED(d3ddev->CreateVertexBuffer(MAX_VERTICES * sizeof(VERTEX_2D_DIF), D3DUSAGE_WRITEONLY, VERTEX_2D_DIF::DIF2DFVF, D3DPOOL_DEFAULT, &vertex_buffer, NULL)))
		exit(1);
}

Draw::~Draw()
{
	vertex_buffer->Release();
}

void Draw::DrawCircle(D3DXVECTOR2 center, float radius, DWORD color)
{
	const int vertexCount = 36;
	if (vertex_count + vertexCount + 1 > MAX_VERTICES)
	{
		Render();
		if (vertexCount + 1 > MAX_VERTICES)
			return;
	}

	DPI dpi;
	dpi.PrimitiveCount = vertexCount;
	dpi.StartVertex = vertex_count;
	dpi.PrimitiveType = D3DPT_LINESTRIP;
	dpi_list.push_back(dpi);

	VERTEX_2D_DIF* buffer = this->vertices;

	int n;
	float step = 2.0f * D3DX_PI / vertexCount;
	float radian = 0.0f;
	for (int i = 0; i < vertexCount; i++)
	{
		n = dpi.StartVertex + i;
		buffer[n].x = center.x + radius * cos(radian);
		buffer[n].y = center.y + radius * sin(radian);
		buffer[n].color = color;
		radian += step;
	}
	n++;
	buffer[n].x = center.x + radius * cos(radian);
	buffer[n].y = center.y + radius * sin(radian);
	buffer[n].color = color;
	//update the vertex count
	vertex_count += vertexCount + 1;
}

void Draw::DrawSolidCircle(D3DXVECTOR2 center, float radius, DWORD color)
{
	const int vertexCount = 36;
	if (vertex_count + vertexCount > MAX_VERTICES)
	{
		Render();
		if (vertexCount > MAX_VERTICES)
			return;
	}

	DPI dpi;
	dpi.PrimitiveCount = vertexCount - 2;
	dpi.StartVertex = vertex_count;
	dpi.PrimitiveType = D3DPT_TRIANGLEFAN;
	dpi_list.push_back(dpi);

	VERTEX_2D_DIF* buffer = this->vertices;
	int n;
	float step = 2.0f * D3DX_PI / vertexCount;
	float radian = 0.0f;
	for (int i = 0; i < vertexCount; i++)
	{
		n = dpi.StartVertex + i;
		buffer[n].x = center.x + radius * cos(radian);
		buffer[n].y = center.y + radius * sin(radian);
		buffer[n].color = color;
		radian += step;
	}

	//update the vertex count
	vertex_count += vertexCount;

	//draw circle outline
	DrawCircle(center, radius, color);
}

void Draw::DrawSegment(D3DXVECTOR2 p1, D3DXVECTOR2 p2, DWORD color)
{
	if (vertex_count + 2 > MAX_VERTICES)
		Render();

	DPI dpi;
	dpi.PrimitiveCount = 1;
	dpi.StartVertex = vertex_count;
	dpi.PrimitiveType = D3DPT_LINESTRIP;
	dpi_list.push_back(dpi);

	VERTEX_2D_DIF* buffer = this->vertices;
	int n = dpi.StartVertex;
	buffer[n].x = p1.x;
	buffer[n].y = p1.y;
	buffer[n].color = color;
	n++;
	buffer[n].x = p2.x;
	buffer[n].y = p2.y;
	buffer[n].color = color;

	//update the vertex count
	vertex_count += 2;
}

void Draw::Render()
{
	//Set vertex format and stream source
	d3ddev->SetFVF(VERTEX_2D_DIF::DIF2DFVF);
	d3ddev->SetStreamSource(0, vertex_buffer, 0, sizeof(VERTEX_2D_DIF));
	d3ddev->SetTexture(0, NULL);

	//Copy over vertex data
	void* buffer;
	UINT size = vertex_count * sizeof(VERTEX_2D_DIF);
	if (FAILED(vertex_buffer->Lock(0, size, &buffer, 0)))
		return;
	memcpy(buffer, vertices, size);
	vertex_buffer->Unlock();

	//Iterate through the vector and draw the primitives
	DPI dpi;
	auto end = dpi_list.end();
	for (auto it = dpi_list.begin(); it != end; it++)
	{
		dpi = *it;
		d3ddev->DrawPrimitive(dpi.PrimitiveType, dpi.StartVertex, dpi.PrimitiveCount);
	}
	//Clear primitives info
	dpi_list.clear();
	vertex_count = 0;
}