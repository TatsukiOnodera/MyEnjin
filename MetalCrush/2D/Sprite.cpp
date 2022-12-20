#include "Sprite.h"
#include "WinApp.h"

#include <cassert>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <DirectXTex.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

ID3D12Device* Sprite::s_dev = nullptr;
UINT Sprite::s_descriptorHandleIncrementSize;
ID3D12GraphicsCommandList* Sprite::s_cmdList = nullptr;
ComPtr<ID3D12RootSignature> Sprite::s_rootsignature;
ComPtr<ID3D12PipelineState> Sprite::s_pipelinestate;
XMMATRIX Sprite::s_matProjection;
ComPtr<ID3D12DescriptorHeap> Sprite::s_descHeap;
ComPtr<ID3D12Resource> Sprite::s_texBuff[c_spriteSRVCount];

bool Sprite::StaticInitialize(ID3D12Device* device)
{
	if (device == nullptr)
	{
		return false;
	}

	Sprite::s_dev = device;

	// デスクリプタサイズを取得
	s_descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateGraphicsPipeline();

	CreateSpriteCommon();

	return true;
}

void Sprite::CreateGraphicsPipeline()
{
	HRESULT result;

	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;	// ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/Shaders/Sprite/SpriteVS.hlsl",	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,

		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		assert(0);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/Shaders/Sprite/SpritePS.hlsl",	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		assert(0);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv座標(1行で書いたほうが見やすい)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// グラフィックスパイプラインの流れを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// サンプルマスク
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	// ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	// デプスステンシルステート
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // 常に上書きルール

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA全てのチャンネルを描画
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	// ブレンドステートの設定
	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	// 深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定（三角形）
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	// 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0〜255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[2];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT); // s0 レジスタ

	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ルートシグネチャの生成
	result = s_dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&s_rootsignature));

	gpipeline.pRootSignature = s_rootsignature.Get();

	// グラフィックスパイプラインの生成
	result = s_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&s_pipelinestate));
}

void Sprite::CreateSpriteCommon()
{
	HRESULT result = S_FALSE;
	
	//並行行列の射影行列生成
	s_matProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, static_cast<float>(WinApp::window_width), static_cast<float>(WinApp::window_height), 0.0f, 0.0f, 1.0f);

	//デスクリプタヒープを設定
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = c_spriteSRVCount;
	//デスクリプタヒープを生成
	result = s_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&s_descHeap));
	assert(SUCCEEDED(result));
}

void Sprite::LoadTexture(const UINT texNumber, const wchar_t* filename)
{
	//異常な番号の指定を検出
	assert(texNumber <= c_spriteSRVCount - 1);

	// 既にあるなら
	if (s_texBuff[texNumber])
	{
		return;
	}

	HRESULT result;

	//WICテクスチャのロード
	TexMetadata metadate{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(
		filename, //画像
		WIC_FLAGS_NONE,
		&metadate, scratchImg);

	const Image* img = scratchImg.GetImage(0, 0, 0); //生データ抽出

	//リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadate.format,
		metadate.width,
		(UINT)metadate.height,
		(UINT16)metadate.arraySize,
		(UINT16)metadate.mipLevels);

	//テクスチャバッファの生成
	result = s_dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&s_texBuff[texNumber])
	);

	//テクスチャバッファにデータ転送
	result = s_texBuff[texNumber]->WriteToSubresource(
		0,
		nullptr, //全領域にコピー
		img->pixels, //元データアドレス
		(UINT)img->rowPitch, //1ラインサイズ
		(UINT)img->slicePitch //全ラインサイズ
	);

	//シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; //設定構造体
	srvDesc.Format = metadate.format; //RGBA
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //２Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	//ヒープのtexnumber番目にシェーダリソースビュー作成
	s_dev->CreateShaderResourceView(
		s_texBuff[texNumber].Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		CD3DX12_CPU_DESCRIPTOR_HANDLE(s_descHeap->GetCPUDescriptorHandleForHeapStart(), texNumber, s_descriptorHandleIncrementSize)
	);
}

void Sprite::PreDraw(ID3D12GraphicsCommandList* cmdList)
{
	Sprite::s_cmdList = cmdList;

	//パイプラインとルートシグネチャの設定
	cmdList->SetPipelineState(s_pipelinestate.Get());
	cmdList->SetGraphicsRootSignature(s_rootsignature.Get());

	//プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::PostDraw()
{
	s_cmdList = nullptr;
}

Sprite *Sprite::Create(const UINT texNumber, const XMFLOAT2& texLeftTop, const XMFLOAT2& anchorpoint, const bool isFilpX, const bool isFilpY, const bool isInvisible)
{
	XMFLOAT2 size = {10, 10};

	if (s_texBuff[texNumber])
	{
		// テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = s_texBuff[texNumber]->GetDesc();
		// スプライトのサイズをテクスチャのサイズに設定
		size = { (float)resDesc.Width, (float)resDesc.Height };
	}

	///新しいスプライトを作る
	Sprite *sprite = new Sprite(size, texNumber, texLeftTop, anchorpoint, isFilpX, isFilpY, isInvisible);

	sprite->Initialize();

	sprite->Update();

	return sprite;
}

Sprite::Sprite(const XMFLOAT2& size, const UINT texNumber, const XMFLOAT2& texLeftTop, const XMFLOAT2& anchorpoint, const bool isFilpX, const bool isFilpY, const bool isInvisible)
{
	this->m_size = size;
	this->m_texNumber = texNumber;
	this->m_texLeftTop = texLeftTop;
	this->m_anchorpoint = anchorpoint;
	this->m_texSize = size;
	this->m_isFilpX = isFilpX;
	this->m_isFilpY = isFilpY;
	this->m_isInvisible = isInvisible;
	this->m_matWorld = XMMatrixIdentity();
}

void Sprite::Initialize()
{
	HRESULT result = S_FALSE;

	//頂点バッファの生成
	result = s_dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv) * c_vertNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertBuff));

	//頂点バッファの転送
	SpriteTransferVertexBuffer();

	// 頂点バッファビューの作成
	m_vbView.BufferLocation = m_vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = sizeof(VertexPosUv) * c_vertNum;
	m_vbView.StrideInBytes = sizeof(VertexPosUv);

	//定数バッファの生成
	result = s_dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constBuff)
	);

	//定数バッファ転送
	ConstBufferData* constMap = nullptr;
	result = m_constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = m_color;
	constMap->mat = s_matProjection;
	m_constBuff->Unmap(0, nullptr);
}

void Sprite::Update()
{
	//ワールド行列の更新
	m_matWorld = XMMatrixIdentity();
	//Z軸回転
	m_matWorld *= XMMatrixRotationZ(XMConvertToRadians(m_rotation));
	//平行移動
	m_matWorld *= XMMatrixTranslation(m_position.x, m_position.y, 0);

	//定数バッファの転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = m_constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = m_matWorld * s_matProjection;
	constMap->color = m_color;
	m_constBuff->Unmap(0, nullptr);
}

void Sprite::Draw()
{
	//非表示フラグがtrueなら
	if (m_isInvisible)
	{
		//描画せず抜ける
		return;
	}

	//デスクリプタヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { s_descHeap.Get() };
	s_cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//頂点バッファをセット
	s_cmdList->IASetVertexBuffers(0, 1, &m_vbView);

	//頂点バッファをセット
	s_cmdList->SetGraphicsRootConstantBufferView(0, m_constBuff->GetGPUVirtualAddress());

	//シェーダリソースビューをセット
	s_cmdList->SetGraphicsRootDescriptorTable(1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
			s_descHeap->GetGPUDescriptorHandleForHeapStart(),
			m_texNumber,
			s_descriptorHandleIncrementSize));

	//ポリゴンの描画（４頂点で四角形）
	s_cmdList->DrawInstanced(c_vertNum, 1, 0, 0);
}

void Sprite::SpriteTransferVertexBuffer()
{
	HRESULT result = S_FALSE;

	// 左下、左上、右下、右上
	enum { LB, LT, RB, RT };

	float left = (0.0f - m_anchorpoint.x) * m_size.x;
	float right = (1.0f - m_anchorpoint.x) * m_size.x;
	float top = (0.0f - m_anchorpoint.y) * m_size.y;
	float bottom = (1.0f - m_anchorpoint.y) * m_size.y;
	if (m_isFilpX)
	{// 左右入れ替え
		left = -left;
		right = -right;
	}

	if (m_isFilpY)
	{// 上下入れ替え
		top = -top;
		bottom = -bottom;
	}

	// 頂点データ
	VertexPosUv vertices[c_vertNum];

	vertices[LB].pos = { left, bottom, 0.0f }; // 左下
	vertices[LT].pos = { left, top, 0.0f }; // 左上
	vertices[RB].pos = { right, bottom, 0.0f }; // 右下
	vertices[RT].pos = { right, top, 0.0f }; // 右上

	// テクスチャ情報取得
	if (s_texBuff[m_texNumber])
	{
		D3D12_RESOURCE_DESC resDesc = s_texBuff[m_texNumber]->GetDesc();

		float tex_left = m_texLeftTop.x / resDesc.Width;
		float tex_right = (m_texLeftTop.x + m_texSize.x) / resDesc.Width;
		float tex_top = m_texLeftTop.y / resDesc.Height;
		float tex_bottom = (m_texLeftTop.y + m_texSize.y) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom }; // 左下
		vertices[LT].uv = { tex_left,	tex_top }; // 左上
		vertices[RB].uv = { tex_right,	tex_bottom }; // 右下
		vertices[RT].uv = { tex_right,	tex_top }; // 右上
	}

	// 頂点バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = m_vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	m_vertBuff->Unmap(0, nullptr);
}
