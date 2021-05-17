#include "CSceneGame.h"
//
#include "CCamera.h"
//
#include "CText.h"
#include"CRes.h"
#include"CKey.h"

CMatrix Matrix;

CSceneGame::~CSceneGame() {

}


void CSceneGame::Init() {
	//3Dモデルファイルの読み込み
	CRes::sModelX.Load(MODEL_FILE);
	//テキストフォントの読み込みと設定
	CText::mFont.Load("FontG.tga");
	CText::mFont.SetRowCol(1, 4096 / 64);
}


void CSceneGame::Update() {
	//カメラのパラメータを作成する
	CVector e, c, u;//視点、注視点、上方向
	//視点を求める
	e = CVector(1.0f, 2.0f, 10.0f);
	//注視点を求める
	c = CVector();
	//上方向を求める
	u = CVector(0.0f, 1.0f, 0.0f);

	Camera.Set(e, c, u);
	Camera.Render();
	if (CKey::Push('K')){
		Matrix = Matrix*CMatrix().RotateX(1);
	}
	if (CKey::Push('L')){
		Matrix = Matrix*CMatrix().RotateY(1);
	}
	if (CKey::Push('I')){
		Matrix = Matrix*CMatrix().RotateX(1);
	}
	if (CKey::Push('J')){
		Matrix = Matrix*CMatrix().RotateY(1);
	}

	glMultMatrixf(Matrix.mF);
	CRes::sModelX.Render();

	//カメラの設定
	Camera3D(e.mX, e.mY, e.mZ, c.mX, c.mY, c.mZ, u.mX, u.mY, u.mZ);




	//2D描画開始
	Start2D(0, 800, 0, 600);

	CText::DrawString("3D PROGRAMMING", 20, 20, 10, 12);

	//2D描画終了
	End2D();

	return;
}

