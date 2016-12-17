#include "LBS.h"

void LBS::AddPointHandle(int x, int y)
{
	CONTROL_HANDLE ch;
	ch.pos.x = x;
	ch.pos.y = y;
	ch.k = point_handles.size();

	POINT_HANDLE ph;
	ph.p = control_handles.size();
	p = ph.p;

	control_handles.push_back(ch);
	var_handles.push_back(ch);
	point_handles.push_back(ph);
}

void LBS::RotatePointHandle(float f)
{
	if (p >= 0 && var_handles[p].k >= 0)
		var_handles[p].rotation += f;
}

void LBS::SetBoneRoot(int x, int y)
{
	bone_root.x = x;
	bone_root.y = y;
	set_bone_root = true;

	bone_joint.x = x;
	bone_joint.y = y;
	set_bone_joint = true;

	p = -1;
}

void LBS::AddBoneEdge(int x, int y)
{
	int n = control_handles.size();

	if (set_bone_root)
	{
		CONTROL_HANDLE ch;
		ch.pos.x = bone_root.x;
		ch.pos.y = bone_root.y;
		control_handles.push_back(ch);
		var_handles.push_back(ch);

		ch.pos.x = x;
		ch.pos.y = y;
		control_handles.push_back(ch);
		var_handles.push_back(ch);

		BONE_EDGE be;
		be.p0 = n;
		be.p1 = n + 1;
		bone_edges.push_back(be);

		p = n + 1;
		set_bone_root = false;
	}
	else if (p >= 0 && control_handles[p].k < 0) // should always be true
	{
		CONTROL_HANDLE ch;
		ch.pos.x = x;
		ch.pos.y = y;
		control_handles.push_back(ch);
		var_handles.push_back(ch);

		BONE_EDGE be;
		be.p0 = p;
		be.p1 = n;
		bone_edges.push_back(be);

		p = n;
	}
}

int LBS::HoverPoint(int x, int y)
{
	int n = var_handles.size();

	for (int i = 0; i < n; i++)
	{
		int dx = var_handles[i].pos.x - x;
		int dy = var_handles[i].pos.y - y;
		
		if (dx * dx + dy * dy <= HANDLE_RADIUS * HANDLE_RADIUS)
			return i;
	}

	return -1;
}

void LBS::SelectPoint(int x, int y, bool right)
{
	if (pose)
	{
		if (!right)
			dragging = true;
		else
			rotating = true;
		p = HoverPoint(x, y);
	}
}

void LBS::SelectPoint(int x, int y, bool ph, bool be)
{
	if (ph && be)
		return;

	int q = HoverPoint(x, y);

	if (q >= 0)
	{
		if (!ph && !be && (!set_bone_joint || control_handles[q].k < 0))
		{
			dragging = true;
			p = q;
		}
	}
	else if (ph && !set_bone_joint)
		AddPointHandle(x, y);
	else if (be)
	{
		if (set_bone_joint)
			AddBoneEdge(x, y);
		else
			SetBoneRoot(x, y);
	}
}

void LBS::DragPoint(int x, int y, int ax, int ay)
{
	if (p >= 0 && dragging)
	{
		if (!pose)
		{
			control_handles[p].pos.x += x;
			control_handles[p].pos.y += y;
		}
		var_handles[p].pos.x += x;
		var_handles[p].pos.y += y;
	}
	else if (p >= 0 && rotating)
		RotatePointHandle(x / 100.0f);
	else if (set_bone_joint)
	{
		bone_joint.x = ax;
		bone_joint.y = ay;
	}
}

void LBS::ReleasePoint()
{
	dragging = false;
	rotating = false;

	if (!pose && p >= 0 && control_handles[p].k < 0)
		set_bone_joint = true;
}

void LBS::DeselectAll()
{
	p = -1;
	set_bone_root = false;
	set_bone_joint = false;
}

void LBS::CalculateWeights(std::vector<VERTEX_2D_TEX> vertices, std::vector<WORD> indices)
{
	int m = vertices.size();
	int n = point_handles.size();
	for (int i = 0; i < n; i++)
	{
		D3DXVECTOR2 pos = control_handles[point_handles[i].p].pos;
		for (int j = 0; j < m; j++)
		{
			float dx = vertices[j].x - pos.x;
			float dy = vertices[j].y - pos.y;
			float d = sqrt(dx * dx + dy * dy);

			point_handles[i].weights.push_back(1.0f / d);
		}
	}

	n = bone_edges.size();
	for (int i = 0; i < n; i++)
	{
		D3DXVECTOR2 pos = control_handles[bone_edges[i].p0].pos;
		D3DXVECTOR2 axis = control_handles[bone_edges[i].p1].pos - pos;

		float norm = sqrt(axis.x * axis.x + axis.y * axis.y);
		D3DXVECTOR2 unit = axis / norm;

		bone_edges[i].length = norm;

		for (int j = 0; j < m; j++)
		{
			D3DXVECTOR2 vpos = D3DXVECTOR2(vertices[j].x, vertices[j].y) - pos;

			float t = (vpos.x * unit.x + vpos.y * unit.y) / norm;

			float d;

			if (0.0 <= t && t <= 1.0)
				d = abs(unit.x * vpos.y - unit.y * vpos.x);
			else
			{
				D3DXVECTOR2 other = vpos - axis;
				d = sqrt(min(vpos.x * vpos.x + vpos.y * vpos.y, other.x * other.x + other.y * other.y));
			}

			bone_edges[i].weights.push_back(1.0f / d);
		}
	}
}

void LBS::UpdatePositions(VERTEX_2D_TEX* buffer, VERTEX_2D_TEX* data, int size)
{
	std::vector<D3DXMATRIX> point_trans;
	std::vector<D3DXMATRIX> bone_trans;

	int n = point_handles.size();
	for (int i = 0; i < n; i++)
	{
		CONTROL_HANDLE ch = control_handles[point_handles[i].p];
		CONTROL_HANDLE vh = var_handles[point_handles[i].p];

		D3DXMATRIX mat1, mat2, mat3;
		D3DXMatrixTranslation(&mat1, -ch.pos.x, -ch.pos.y, 0.0f);
		D3DXMatrixRotationZ(&mat2, vh.rotation);
		D3DXMatrixTranslation(&mat3, vh.pos.x, vh.pos.y, 0.0f);

		point_trans.push_back(mat1 * mat2 * mat3);
	}

	n = bone_edges.size();
	for (int i = 0; i < n; i++)
	{
		CONTROL_HANDLE ch0 = control_handles[bone_edges[i].p0];
		CONTROL_HANDLE ch1 = control_handles[bone_edges[i].p1];
		CONTROL_HANDLE vh0 = var_handles[bone_edges[i].p0];
		CONTROL_HANDLE vh1 = var_handles[bone_edges[i].p1];

		D3DXVECTOR2 caxis = ch1.pos - ch0.pos;
		D3DXVECTOR2 axis = vh1.pos - vh0.pos;
		float norm = sqrt(axis.x * axis.x + axis.y * axis.y);
		float scale = norm / bone_edges[i].length;

		D3DXMATRIX mat1, mat2, mat3, mat4;
		D3DXMatrixTranslation(&mat1, -ch0.pos.x, -ch0.pos.y, 0.0f);
		D3DXMatrixScaling(&mat2, scale, scale, 1.0f);
		D3DXMatrixRotationZ(&mat3, atan2(axis.y, axis.x) - atan2(caxis.y, caxis.x));
		D3DXMatrixTranslation(&mat4, vh0.pos.x, vh0.pos.y, 0.0f);

		//bone_trans.push_back(mat1 * mat2 * mat3 * mat4);
		bone_trans.push_back(mat1 * mat2 * mat4);
	}

	for (int i = 0; i < size; i++)
	{
		D3DXVECTOR3 vec(data[i].x, data[i].y, 0.0f);
		D3DXVECTOR2 newpos(0.0f, 0.0f);
		float total = 0.0f;

		n = point_handles.size();
		for (int j = 0; j < n; j++)
		{
			D3DXVECTOR3 result;

			D3DXVec3TransformCoord(&result, &vec, &point_trans[j]);

			float f = point_handles[j].weights[i];
			newpos.x += result.x * f;
			newpos.y += result.y * f;
			total += f;
		}

		n = bone_edges.size();
		for (int j = 0; j < n; j++)
		{
			D3DXVECTOR3 result;

			D3DXVec3TransformCoord(&result, &vec, &bone_trans[j]);

			float f = bone_edges[j].weights[i];
			newpos.x += result.x * f;
			newpos.y += result.y * f;
			total += f;
		}

		if (total > 0.0f)
		{
			newpos /= total;

			buffer[i].x = newpos.x;
			buffer[i].y = newpos.y;
		}
	}

}

void LBS::Render()
{
	if (!pose)
	{
		int n = control_handles.size();
		for (int i = 0; i < n; i++)
			if (i != p)
				draw->DrawSolidCircle(control_handles[i].pos, HANDLE_RADIUS, D3DCOLOR_ARGB(150, 255, 255, 0));

		if (p >= 0)
			draw->DrawSolidCircle(control_handles[p].pos, HANDLE_RADIUS, D3DCOLOR_ARGB(150, 0, 0, 255));
		else if (set_bone_root)
			draw->DrawSolidCircle(bone_root, HANDLE_RADIUS, D3DCOLOR_ARGB(150, 0, 0, 255));

		if (set_bone_joint)
		{
			draw->DrawSolidCircle(bone_joint, HANDLE_RADIUS, D3DCOLOR_ARGB(150, 255, 0, 0));
			if (set_bone_root)
				draw->DrawSegment(bone_root, bone_joint, D3DCOLOR_ARGB(150, 255, 0, 0));
			else if (p >= 0)
				draw->DrawSegment(control_handles[p].pos, bone_joint, D3DCOLOR_ARGB(150, 255, 0, 0));
		}

		int m = bone_edges.size();
		for (int i = 0; i < m; i++)
			draw->DrawSegment(control_handles[bone_edges[i].p0].pos, control_handles[bone_edges[i].p1].pos, D3DCOLOR_ARGB(150, 0, 255, 255));
	}
	else
	{
		int n = var_handles.size();
		for (int i = 0; i < n; i++)
			if (i != p)
				draw->DrawSolidCircle(var_handles[i].pos, HANDLE_RADIUS, D3DCOLOR_ARGB(150, 255, 255, 0));

		if (p >= 0)
			draw->DrawSolidCircle(var_handles[p].pos, HANDLE_RADIUS, D3DCOLOR_ARGB(150, 0, 0, 255));

		int m = bone_edges.size();
		for (int i = 0; i < m; i++)
			draw->DrawSegment(var_handles[bone_edges[i].p0].pos, var_handles[bone_edges[i].p1].pos, D3DCOLOR_ARGB(150, 0, 255, 255));
	}

	draw->Render();
}