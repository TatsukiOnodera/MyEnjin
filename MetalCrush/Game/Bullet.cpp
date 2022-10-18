#include "Bullet.h"

using namespace DirectX;

Bullet::Bullet(const XMFLOAT3& pos, const XMFLOAT3& vel, const bool& alive)
{
	m_object.reset(Object3d::Create("Bullet", true));
	Initialize(pos, vel, alive);
}

Bullet::~Bullet()
{

}

void Bullet::Initialize(const XMFLOAT3& pos, const XMFLOAT3& vel, const bool& alive)
{
	if (m_object == nullptr)
	{
		assert(0);
	}

	m_pos = pos;
	m_vel = vel;
	m_alive = alive;

	m_object->SetPosition(m_pos);
	m_object->Update();
}

void Bullet::Update()
{
	if (m_alive == true)
	{
		// ���W�ɑ��x�����Z
		m_pos.x += m_vel.x;
		m_pos.y += m_vel.y;
		m_pos.z += m_vel.z;

		m_object->SetPosition(m_pos);
	}
}

void Bullet::Draw()
{
	if (m_alive == true)
	{
		m_object->Draw();
	}
}