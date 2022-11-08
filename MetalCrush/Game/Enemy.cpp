#include "Enemy.h"

Enemy::Enemy()
{
	// モデルの読み込み
	m_object.reset(Object3d::Create("Enemy", true));

	// 初期化
	Initialize();
}

Enemy::~Enemy()
{

}

void Enemy::Initialize()
{
	// nullチェック
	if (m_object == nullptr)
	{
		assert(0);
	}

	// 座標
	m_pos = { static_cast<float>(rand() % 21 - 10), -900, 100 };
	// 速度
	//m_vel = { static_cast<float>(rand() % 21 - 10) / 10, -9.8f, static_cast<float>(rand() % 21 - 10) / 10 };
	m_vel = { 0, -0.98f, 0 };
	// 生存フラグ
	m_alive = true;
	// 発射間隔
	m_bulletInterval = 0;

	// オブジェクト
	m_object->SetPosition(m_pos);
	m_object->SetScale({ 3, 3, 3 });
	m_object->SetColor({ 1.0f, 0.1f, 0.1f, 1.0f });
	m_object->Update();
}

void Enemy::Update()
{
	// エネミーの更新
	if (m_alive == true)
	{
		// 加速度
		//XMFLOAT3 acc = {};

		// 速度に加速度を加算
		//m_vel.x += acc.x;
		//m_vel.y += acc.y;
		//m_vel.z += acc.z;

		// 座標に速度を加算
		m_pos.x += m_vel.x;
		m_pos.y += m_vel.y;
		m_pos.z += m_vel.z;

		m_object->SetPosition(m_pos);

		// 弾の間隔
		m_bulletInterval++;
		if (90 < m_bulletInterval)
		{
			m_bulletInterval = 90;
		}
	}

	// 弾の更新
	for (auto& m : enemyBullets)
	{
		m->Update();
	}
}

void Enemy::Draw(ID3D12GraphicsCommandList* cmdList)
{
	// OBJオブジェクト描画
	Object3d::PreDraw(cmdList);
	
	if (m_alive == true)
	{
		m_object->Draw();
	}
	if (0 < enemyBullets.size())
	{
		for (auto& m : enemyBullets)
		{
			m->Draw();
		}
	}

	Object3d::PostDraw();
}

void Enemy::ShotBullet(const XMFLOAT3& targetPos)
{
	if (90 <= m_bulletInterval)
	{
		m_bulletInterval = 0;

		XMFLOAT3 vel = {};
		// 標的の座標を取得
		vel.x = targetPos.x - m_pos.x;
		vel.y = targetPos.y - m_pos.y;
		vel.z = targetPos.z - m_pos.z;

		// 長さを1にして10倍する
		float len = sqrtf(powf(vel.x, 2) + powf(vel.y, 2) + powf(vel.z, 2));
		vel.x = vel.x / len * 2;
		vel.y = vel.y / len * 2;
		vel.z = vel.z / len * 2;

		if (CheckNoUsingBullet() == true)
		{
			for (auto& m : enemyBullets)
			{
				if (m->GetAlive() == false)
				{
					m->Initialize(m_pos, vel, XMFLOAT3(0, 0, 0), true);
					break;
				}
			}
		}
		else
		{
			enemyBullets.emplace_back(new Bullet(m_pos, vel));
		}
	}
}

bool Enemy::CheckNoUsingBullet()
{
	if (0 < enemyBullets.size())
	{
		// 使っていないのがあるか
		bool hit = false;
		for (const auto& m : enemyBullets)
		{
			if (m->GetAlive() == false)
			{
				hit = true;
				break;
			}
		}

		return hit;
	}
	else
	{
		return false;
	}
}
