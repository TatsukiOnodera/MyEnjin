#include "GamePlayScene.h"
#include "SceneManager.h"
#include <time.h>
#include <stdlib.h>
#include <SafeDelete.h>

using namespace DirectX;
using namespace Microsoft::WRL;

GamePlayScene::~GamePlayScene()
{
	safe_delete(particle);
	safe_delete(demo_back);
	safe_delete(demo_spr);
	safe_delete(chr);
	safe_delete(obj);
}

void GamePlayScene::Initialize()
{
	dx_cmd = DirectXCommon::GetInstance();
	input = Input::GetInstance();
	audio = Audio::GetInstance();
	camera = Camera::GetInstance();
	
	//スプライトテクスチャ読み込み
	Sprite::LoadTexture(fontNumber, L"Resources/DebugFont/DebugFont.png");
	Sprite::LoadTexture(1, L"Resources/texture.png");
	Sprite::LoadTexture(2, L"Resources/background.png");

	//前景スプライト
	debugText.Initialize(fontNumber);

	//スプライト
	demo_spr = Sprite::CreateSprite(1);
	demo_back = Sprite::CreateSprite(2);

	//オブジェクト
	chr = Object3d::Create("chr_sword");
	obj = Object3d::Create("Bullet");

	//パラメーター
	ResetVariable();

	//オーディオ
	audio->Initialize();
}

void GamePlayScene::ResetVariable()
{
	chr->SetScale({5.0f, 5.0f, 5.0f});
	chr->Update();

	obj->SetPosition({10.0f, 0.0f, 0.0f});
	obj->SetScale({ 5.0f, 5.0f, 5.0f });
	obj->SetColor({1, 1, 1, 0.8f});
	obj->Update();
}

void GamePlayScene::Update()
{
	XMFLOAT3 pos = { input->LeftStickAngle().x / 2, input->LeftStickAngle().y / 2, 0 };
	camera->MoveCamera(pos);

	chr->Update();
	obj->Update();
}

void GamePlayScene::Draw()
{
	DrawBackSprite();
	DrawObject();
	DrawParticles();
	DrawUI();
	DrawDebugText();
}

void GamePlayScene::DrawBackSprite()
{
	ID3D12GraphicsCommandList* cmdList = dx_cmd->GetCmdList();

	//前景スプライト描画
	Sprite::PreDraw(cmdList);

	demo_back->Draw();

	Sprite::PostDraw();
	dx_cmd->ClearDepth();
}

void GamePlayScene::DrawObject()
{
	ID3D12GraphicsCommandList* cmdList = dx_cmd->GetCmdList();

	//オブジェクト描画
	Object3d::PreDraw(cmdList);

	chr->Draw();
	obj->Draw();

	Object3d::PostDraw();

	//スプライト描画
	Sprite::PreDraw(cmdList);

	demo_spr->Draw();

	Sprite::PostDraw();
}

void GamePlayScene::DrawUI()
{
	ID3D12GraphicsCommandList* cmdList = dx_cmd->GetCmdList();

	//UI描画
	Sprite::PreDraw(cmdList);

	

	Sprite::PostDraw();
}

void GamePlayScene::DrawParticles()
{
	ID3D12GraphicsCommandList* cmdList = dx_cmd->GetCmdList();

	//パーティクル描画
	//particle->Draw(cmdList);
}

void GamePlayScene::DrawDebugText()
{
	ID3D12GraphicsCommandList* cmdList = dx_cmd->GetCmdList();

	//スプライト描画
	Sprite::PreDraw(cmdList);

	//debugText.Draw();

	Sprite::PostDraw();
}