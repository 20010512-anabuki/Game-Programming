#include "CMaterial.h"
//memsetのインクルード
#include <string.h>
#include<stdio.h>
#include "glut.h"

//デフォルトコンストラクタ
CMaterial::CMaterial()
: mpTexture(0), mVertexNum(0)
{
	//名前を0で埋め
	memset(mName, 0, sizeof(mName));
	//0で埋める
	memset(mDiffuse, 0, sizeof(mDiffuse));
}
//マテリアルを有効にする
void CMaterial::Enabled() {
	//アルファブレンドを有効にする
	glEnable(GL_BLEND);
	//ブレンド方法を指定
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//拡散光の設定
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mDiffuse);
	//テクスチャが無い時、戻る
	if (mpTexture == 0)
		return;
	//テクスチャ有り
	if (mpTexture->mId) {
		//テクスチャを使用可能にする
		glEnable(GL_TEXTURE_2D);
		//テクスチャをバインドする
		glBindTexture(GL_TEXTURE_2D, mpTexture->mId);
	}
}
//マテリアルを無効にする
void CMaterial::Disabled() {
	//アルファブレンドを無効
	glDisable(GL_BLEND);
	//テクスチャが無い時、戻る
	if (mpTexture == 0)
		return;
	//テクスチャ有り
	if (mpTexture->mId) {
		//テクスチャのバインドを解く
		glBindTexture(GL_TEXTURE_2D, 0);
		//テクスチャを無効にする
		glDisable(GL_TEXTURE_2D);
	}
}

//マテリアルデータの読み込みと設定
CMaterial::CMaterial(CModelX*model)
:mpTextureFilename(nullptr)
{
	model->GetToken();//{?Name
	if (strcmp(model->mToken, "{") != 0){
		strcpy(mName, model->mToken);
		model->GetToken();//{
	}
	mDiffuse[0] = model->GetFloatToken();
	mDiffuse[1] = model->GetFloatToken();
	mDiffuse[2] = model->GetFloatToken();
	mDiffuse[3] = model->GetFloatToken();

	mPower = model->GetFloatToken();

	mSpecular[0] = model->GetFloatToken();
	mSpecular[1] = model->GetFloatToken();
	mSpecular[2] = model->GetFloatToken();

	mEmissive[0] = model->GetFloatToken();
	mEmissive[1] = model->GetFloatToken();
	mEmissive[2] = model->GetFloatToken();

	model->GetToken();//TextureFilename or }
	if (strcmp(model->mToken, "TextureFilename") == 0){
		model->GetToken();//{
		model->GetToken();//filename
		mpTextureFilename = new char[strlen(model->mToken) + 1];
		strcpy(mpTextureFilename, model->mToken);
		model->GetToken();//}
		model->GetToken();//}
	}
#ifdef _DEBUG
	printf("%s￥n", mName);
	printf("Diffuse:%f %f %f %f￥n", mDiffuse[0], mDiffuse[1], mDiffuse[2], mDiffuse[3]);
	printf("Power:%f￥n", mPower);
	printf("Specular:%f %f %f￥n", mSpecular[0], mSpecular[1], mSpecular[2]);
	printf("Emissive:%f %f %f￥n", mEmissive[0], mEmissive[1], mEmissive[2]);

#endif
}

