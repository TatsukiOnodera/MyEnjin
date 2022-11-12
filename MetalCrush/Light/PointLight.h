#pragma once
#include <DirectXMath.h>

class PointLight
{
private: // エイリアス
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;



public: // サブクラス
	// 定数バッファ用
	struct ConstBufferData
	{
		XMFLOAT3 lightPos;
		float pad1;
		XMFLOAT3 lightColor;
		float pad2;
		XMFLOAT3 lightAtten;
		unsigned int active;
	};

private: // メンバ変数
	// ライト座標（ワールド座標）
	XMFLOAT3 m_lightPos = { 0, 0, 0 };
	// ライト色
	XMFLOAT3 m_lightColor = { 1, 1, 1 };
	// ライト距離減衰係数
	XMFLOAT3 m_lightAtten = { 1, 1, 1 };
	// 有効フラグ
	bool m_active = false;

public: // アクセッサ
	/// <summary>
	/// ライト座標を取得
	/// </summary>
	/// <returns>ライト座標</returns>
	inline const XMFLOAT3& GetLightPos() { return m_lightPos; }

	/// <summary>
	/// ライト座標をセット
	/// </summary>
	/// <param name="lightPos">ライト座標</param>
	inline void SetLightPos(const XMFLOAT3& lightPos)
	{
		m_lightPos = lightPos;
	}

	/// <summary>
	/// ライト色を取得
	/// </summary>
	/// <returns>ライト色</returns>
	inline const XMFLOAT3& GetLightColor() { return m_lightColor; }

	/// <summary>
	/// ライト色をセット
	/// </summary>
	/// <param name="lightPos">ライト色</param>
	inline void SetLightColor(const XMFLOAT3& lightColor)
	{
		m_lightColor = lightColor;
	}

	/// <summary>
	/// ライト距離減衰係数を取得
	/// </summary>
	/// <returns>ライト距離減衰係数</returns>
	inline const XMFLOAT3& GetLightAtten() { return m_lightAtten; }

	/// <summary>
	/// ライト距離減衰係数をセット
	/// </summary>
	/// <param name="lightPos">ライト距離減衰係数</param>
	inline void SetLightAtten(const XMFLOAT3& lightAtten)
	{
		m_lightAtten = lightAtten;
	}

	/// <summary>
	/// 有効フラグを取得
	/// </summary>
	/// <returns>有効フラグ</returns>
	inline bool GetActive() { return m_active; };

	/// <summary>
	/// 有効フラグをセット
	/// </summary>
	/// <param name="active">有効フラグ</param>
	inline void SetActive(const bool active)
	{
		m_active = active;
	}
};
