#pragma once
#include "Input.h"
#include "Model.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Audio.h"
#include "DebugText.h"
#include "ParticleManager.h"
#include "Collision.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "BaseScene.h"
#include "LightGroup.h"
#include "FbxLoader.h"
#include "FbxObject3d.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "AutoLockOn.h"
#include "CollisionManager.h"
#include "MeshCollider.h"
#include "TouchableObject.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <array>

class GamePlayScene : public BaseScene
{
public: // エイリアス
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

private: // サブクラス
	// シェーダーの種類
	enum ShadersType
	{
		ADS, TOON, MONO, BLEND, SPECULAR
	};

	// 壁の配置
	enum WALLNUMBER
	{
		FRONT, BACK, RIGHT, LEFT, UP, DOWN, END
	};

private: // メモリ置き場
	// DIrectXコマンド
	DirectXCommon* dx_cmd = nullptr;
	// 操作系
	Input* input = nullptr;
	// オーディオ
	//Audio* audio = nullptr;
	// カメラ
	Camera* camera = nullptr;
	// デバッグテキスト
	DebugText debugText;

private: // インスタンス
	// ライト
	std::unique_ptr<LightGroup> lightGroup = nullptr;

	// パーティクル
	std::unique_ptr<ParticleManager> explosion = nullptr;

	// スプライト
	std::unique_ptr<Sprite> Reticle = nullptr;
	std::unique_ptr<Sprite> HPFrame = nullptr;
	std::unique_ptr<Sprite> HP = nullptr;
	std::unique_ptr<Sprite> BoosterFrame = nullptr;
	std::unique_ptr<Sprite> Booster = nullptr;
	std::unique_ptr<Sprite> Monitor = nullptr;
	std::unique_ptr<Sprite> GameOver = nullptr;
	std::unique_ptr<Sprite> BulletNum[2];
	std::unique_ptr<Sprite> BulletNumFrame = nullptr;
	std::unique_ptr<Sprite> Rader = nullptr;
	std::vector<std::unique_ptr<Sprite>> enemysMarker;

	// OBJ
	std::unique_ptr<TouchableObject> desert = nullptr;
	std::unique_ptr<Object3d> skyWall = nullptr;
	std::vector<std::unique_ptr<Enemy>> enemy;
	std::unique_ptr<Player> player = nullptr;

	// その他
	std::unique_ptr<AutoLockOn> lockList = nullptr;
	CollisionManager* collisionManager = nullptr;

private: // メンバ変数
	bool isEnd = true;

public: // メンバ関数
	/// <summary>
	/// デストラクタ
	/// </summary>
	~GamePlayScene() override;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 変数初期化
	/// </summary>
	void InitializeParameter();

	/// <summary>
	/// 背景スプライト描画
	/// </summary>
	void DrawBackSprite(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// 描画
	/// </summary>
	void DrawObjects(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// エフェクト描画
	/// </summary>
	void DrawEffect(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// UI描画
	/// </summary>
	void DrawHUD(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// デバッグテキスト描画
	/// </summary>
	void DrawDebugText(ID3D12GraphicsCommandList* cmdList);

public: // メンバ関数
	/// <summary>
	/// 衝突判定
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// プレイヤーとエネミーの衝突判定	
	/// </summary>
	void CheckPlayer2Enemy();

	/// <summary>
	/// プレイヤーの弾とエネミーの衝突判定
	/// </summary>
	void CheckPlayerBullets2Enemy();

	/// <summary>
	/// プレイヤーとエネミーの弾の衝突判定
	/// </summary>
	void CheckPlayer2EnemyBullets();
};
