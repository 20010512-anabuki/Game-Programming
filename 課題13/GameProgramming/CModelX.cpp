#include"glut.h"
#include<stdio.h>
#include<string.h>
#include"CModelX.h"
#include"CMaterial.h"

void CModelX::Load(char*file){
	//ファイルサイズの獲得
	FILE*fp;
	fp = fopen(file, "rb");
	if (fp == NULL){
		printf("fopen error:%s\n", file);
		return;
	}
	fseek(fp, 0L, SEEK_END);
	int  size = ftell(fp);
	char*buf = mpPointer = new char[size + 1];
	fseek(fp, 0L, SEEK_SET);
	fread(buf, size, 1, fp);
	buf[size] = '\0';
	//printf(buf);
	fclose(fp);
	while (*mpPointer != '\0'){
		GetToken();
		if (strcmp(mToken, "Frame") == 0){
				new CModelXFrame(this);
				//	printf("%s", mToken);
				//	GetToken();
				//	printf("%s\n", mToken);
			}
		else if (strcmp(mToken, "AnimationSet") == 0){
			new CAnimationSet(this);
		}
			//if (strcmp(mToken, "AnimationSet") == 0){
			//	printf("%s", mToken);
			//GetToken();
			//	printf("%s\n", mToken);
			//}

		}
	SAFE_DELETE_ARRAY(buf);
}

void CModelX::GetToken(){
	char* p = mpPointer;
	char* q = mToken;
	while (*p != '\0' && strchr(" \t\r\n,;\"", *p))p++;
	if (*p == '{' || *p == '}'){
		*q++ = *p++;
	}
	else{
		while (*p != '\0' && ! strchr(" \t\r\n,;\"}",*p))
			*q++ = *p++;
	}
	*q = '\0'; 
	mpPointer = p;
	if (!strcmp("//", mToken)){
		while (*p != '\0' && !strchr("\r\n", *p))p++;
		mpPointer = p;
		GetToken();
	}
}

void CModelX::SkipNode(){
	while (*mpPointer != '\0'){
		GetToken();
		if (strchr(mToken, '{'))break;
	}
	int count = 1;
	while (*mpPointer !='\0' && count>0){
		GetToken();
		if (strchr(mToken, '{'))count++;
		else if (strchr(mToken, '}'))count--;
	}
}

CModelXFrame::CModelXFrame(CModelX*model){
	mIndex = model->mFrame.size();
	model->mFrame.push_back(this);
	mTransformMatrix.Identity();
	model->GetToken();
	mpName = new char[strlen(model->mToken) + 1];
	strcpy(mpName, model->mToken);
	model->GetToken();
	while (*model->mpPointer != '\0'){
		model->GetToken();
		if (strchr(model->mToken, '}'))break;
		if (strcmp(model->mToken, "Frame") == 0){
			mChild.push_back(new CModelXFrame(model));
		}
		else if (strcmp(model->mToken, "FrameTransformMatrix") == 0){
			model->GetToken();//{
			for (int i = 0; i < ARRY_SIZE(mTransformMatrix.mF); i++){
				mTransformMatrix.mF[i] = model->GetFloatToken();
			}
			model->GetToken();//}
		}
		else if (strcmp(model->mToken, "Mesh") == 0){
			mMesh.Init(model);
		}
		else {
			model->SkipNode();
		}
	}
//#ifdef _DEBUG
	//printf("%s\n", mpName);
	//mTransformMatrix.Print();
//#endif
}

CModelXFrame*CModelX::FindFrame(char*name){
	//イテレータの作成
	std::vector<CModelXFrame*>::iterator itr;
	//先頭から最後まで繰り返し
	for (itr = mFrame.begin(); itr != mFrame.end(); itr++){
		//名前が一致したかどうか
		if (strcmp(name, (*itr)->mpName) == 0){
		//一致したらアドレスを返す
			return *itr;
		}
	}
	//一致しない場合はNULLを返す
	return NULL;
}

float CModelX::GetFloatToken(){
	GetToken();
	return atof(mToken);
}

void CMesh::Init(CModelX* model) {
	model->GetToken();
	if (!strchr(model->mToken, '{')) {
		model->GetToken();//{
	}
	mVertexNum = model->GetIntToken();
	mpVertex = new CVector[mVertexNum];
	for (int i = 0; i < mVertexNum; i++) {
		mpVertex[i].mX = model->GetFloatToken();
		mpVertex[i].mY = model->GetFloatToken();
		mpVertex[i].mZ = model->GetFloatToken();
	}
	mFaceNum = model->GetIntToken(); //面数読み込み
	mpVertexIndex = new int[mFaceNum * 3]; //頂点数は1つの面に3つ
	for (int i = 0; i < mFaceNum * 3; i += 3) {
		model->GetToken();
		mpVertexIndex[i] = model->GetIntToken();
		mpVertexIndex[i + 1] = model->GetIntToken();
		mpVertexIndex[i + 2] = model->GetIntToken();
	}
	while (model->mpPointer != '\0') {
		model->GetToken();
		if (strchr(model->mToken, '}'))
			break;
		if (strcmp(model->mToken, "MeshNormals") == 0) {
			model->GetToken();//{
			//法線データの取得
			mNormalNum = model->GetIntToken();
			//法線データを配列へ
			CVector* pNormal = new CVector[mNormalNum];
			for (int i = 0; i < mNormalNum; i++) {
				pNormal[i].mX = model->GetFloatToken();
				pNormal[i].mY = model->GetFloatToken();
				pNormal[i].mZ = model->GetFloatToken();
			}
			//法線数=面数*3
			mNormalNum = model->GetIntToken() * 3;//FaceNum
			int ni;
			//頂点ごとに法線データを設定
			mpNormal = new CVector[mNormalNum];
			for (int i = 0; i < mNormalNum; i += 3) {
				model->GetToken();//3
				ni = model->GetIntToken();
				mpNormal[i] = pNormal[ni];
				ni = model->GetIntToken();
				mpNormal[i + 1] = pNormal[ni];
				ni = model->GetIntToken();
				mpNormal[i + 2] = pNormal[ni];
			}
			delete[]pNormal;
			model->GetToken();//}
		}
		else if (strcmp(model->mToken, "MeshMaterialList") == 0) {
			model->GetToken();//{
			//マテリアルの数
			mMaterialNum = model->GetIntToken();
			mMaterialIndexNum = model->GetIntToken();
			mpMaterialIndex = new int[mMaterialIndexNum];
			for (int i = 0; i < mMaterialIndexNum; i++) {
				mpMaterialIndex[i] = model->GetIntToken();
			}
			//マテリアルデータの作成
			for (int i = 0; i < mMaterialNum; i++) {
				model->GetToken();
				if (strcmp(model->mToken, "Material") == 0) {
					mMaterial.push_back(new CMaterial(model));
				}
			}
			model->GetToken();	//}//End of MeshMaterialList
		}
		//SkinWeightsの時
		else if (strcmp(model->mToken, "SkinWeights") == 0){
			mSkinWeights.push_back(new CSkinWeights(model));
		}
		else {
			//ノードの読み飛ばし
			model->SkipNode();
		}
	}
}


//#ifdef _DEBUG
	//printf("VertexNum:%d\n",  mVertexNum);
	//for (int i = 0; i < mVertexNum; i++){
	//printf("%10f", mpVertex[i].mX);
	//printf("%10f", mpVertex[i].mY);
	//printf("%10f\n", mpVertex[i].mZ);
	//}

	//printf("FaceNum:%d\n", mFaceNum);
	//for (int i = 0; i < mFaceNum * 3; i += 3){
	//printf("%4d", mpVertexIndex[i]);
	//printf("%4d", mpVertexIndex[i + 1]);
	//printf("%4d\n", mpVertexIndex[i + 2]);
    //}

	//printf("NormalNum:%d\n", mNormalNum);
	//for (int i = 0; i < mNormalNum; i++){
		//printf("%10f", mpNormal[i].mX);
		//printf("%10f", mpNormal[i].mY);
		//printf("%10f\n", mpNormal[i].mZ);
//	}
//#endif

CSkinWeights::CSkinWeights(CModelX*model)
:mpFrameName(0)
, mFrameIndex(0)
, mIndexNum(0)
, mpIndex(nullptr)
, mpWeight(nullptr)
{
	model->GetToken();	//{
	model->GetToken();	//FrameName
	//フレーム名エリア確保と設定
	mpFrameName = new char[strlen(model->mToken) + 1];
	strcpy(mpFrameName, model->mToken);
	//頂点番号取得
	mIndexNum = model->GetIntToken();
	//頂点番号が0を超えた場合
	if (mIndexNum > 0){
		mpIndex = new int[mIndexNum];
		mpWeight = new float[mIndexNum];
		for (int i = 0; i < mIndexNum; i++)
			mpIndex[i] = model->GetIntToken();
		for (int i = 0; i < mIndexNum; i++)
			mpWeight[i] = model->GetFloatToken();
	}
	//オフセット行列取得
	for (int i = 0; i < 16; i++){
		mOffset.mF[i] = model->GetFloatToken();
	}
	model->GetToken();	//}

//#ifdef _DEBUG
	//printf("SkinWeights:%s\n", mpFrameName);
	//for (int i = 0; i < mIndexNum; i++) {
		//printf("%d", mpIndex[i]);
		//printf("%10f\n", mpWeight[i]);
//	}
	//mOffset.Print();
//#endif
}

CAnimationSet::CAnimationSet(CModelX*model)
:mpName(nullptr)
{
	model->mAnimationSet.push_back(this);
	model->GetToken();  // Animation Name
	//アニメーションセット名を退避
	mpName = new char[strlen(model->mToken) + 1];
	strcpy(mpName, model->mToken);
	model->GetToken();//{
	while (*model->mpPointer != '\0'){
		model->GetToken();// } or Animation
		if (strchr(model->mToken, '}'))break;
		if (strcmp(model->mToken, "Animation") == 0){
			//Animation要素の読み込み
			mAnimation.push_back(new CAnimation(model));
		}
	}
//#ifdef _DEBUG
	//printf("AnimationSet:%s\n", mpName);
//#endif

}

CAnimation::CAnimation(CModelX*model)
:mpFrameName(0)
,mFrameIndex(0)
, mKeyNum(0)
, mpKey(nullptr)
{
	model->GetToken(); // { or Animation Name
	if (strchr(model->mToken, '{')){
		model->GetToken(); // {
	}
	else
	{
		model->GetToken(); // {
		model->GetToken() ;// {
	}
	model->GetToken();//FrameName
	mpFrameName = new char[strlen(model->mToken) + 1];
	strcpy(mpFrameName, model->mToken);
	mFrameIndex =
		model->FindFrame(model->mToken)->mIndex;
	model->GetToken();//}
	//キーの配列を保存しておく配列
	CMatrix*key[4] = { 0, 0, 0, 0 };
	//時間の配列を保存しておく配列
	float*time[4] = { 0, 0, 0, 0 };
	while (*model->mpPointer != '\0'){
		model->GetToken(); // } or AnimationKey
		if (strchr(model->mToken, '}'))break;
		if (strcmp(model->mToken, "AnimationKey") == 0){
			model->GetToken(); // {
			//データタイプ取得
			int type = model->GetIntToken();
			//時間数取得
			mKeyNum = model->GetIntToken();
			switch (type){
			case 0://Rotation Quaternion
				//行列の配列を時間数分確保
				key[type] = new CMatrix[mKeyNum];
				time[type] = new float[mKeyNum];
				//時間を繰り返す
				for (int i = 0; i < mKeyNum; i++){
					//時間取得
					time[type][i] = model->GetFloatToken();
					model->GetToken();//4を読み飛ばし
					//w,x,y,zを取得
					float w = model->GetFloatToken();
					float x = model->GetFloatToken();
					float y = model->GetFloatToken();
					float z = model->GetFloatToken();
					//クォータニオンから回転行列に変換
					key[type][i].SetQuaternion(x, y, z, w);
				}
				break;
			case 1://拡大縮小の行列作成
				key[type] = new CMatrix[mKeyNum];
				time[type] = new float[mKeyNum];
				for (int i = 0; i < mKeyNum; i++){
					time[type][i] = model->GetFloatToken();
					model->GetToken();//3
					float x = model->GetFloatToken();
					float y = model->GetFloatToken();
					float z = model->GetFloatToken();
					key[type][i].mM[0][0] = x;
					key[type][i].mM[1][1] = y;
					key[type][i].mM[2][2] = z;
				}
				break;
			case 2://移動の行列作成
				key[type] = new CMatrix[mKeyNum];
				time[type] = new float[mKeyNum];
				for (int i = 0; i < mKeyNum; i++){
					
					time[type][i] = model->GetFloatToken();
					model->GetToken();//3
					float x = model->GetFloatToken();
					float y = model->GetFloatToken();
					float z = model->GetFloatToken();
					key[type][i].Translate(x, y, z);
				}
				break;
			case 4://行列データを取得
				mpKey = new CAnimationKey[mKeyNum];
				for (int i = 0; i < mKeyNum; i++){
					mpKey[i].mTime = model->GetFloatToken();//Time
					model->GetToken();//16
					for (int j = 0; j < 16; j++){
						mpKey[i].mMatrix.mF[j] = model->GetFloatToken();
					}
				}
				break;
			}
			model->GetToken();//}
		}
		else{
			model->SkipNode();
		}
	}
	//行列データではないとき
	if (mpKey == 0){
		//数時間分キーを作成
		mpKey = new CAnimationKey[mKeyNum];
		for (int i = 0; i < mKeyNum; i++){
			//時間設定
			mpKey[i].mTime = time[2][i];//Time
			//行列作成 Size*Rotation*Position
			mpKey[i].mMatrix = key[1][i] * key[0][i] * key[2][i];
		}
	}
	//確保したエリア開放
	for (int i = 0; i < ARRY_SIZE(key); i++){
		SAFE_DELETE_ARRAY(time[i]);
		SAFE_DELETE_ARRAY(key[i]);
	}
	
#ifdef _DEBUG
	printf("Animation:%s\n", mpFrameName);
	mpKey[0].mMatrix.Print();
#endif
}

void CMesh::Render(){
	//頂点データ、法線データの有効
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	//データの場所指定
	glVertexPointer(3, GL_FLOAT, 0, mpVertex);
	glNormalPointer(GL_FLOAT, 0, mpNormal);
	//頂点のインデックスの場所を指定し、図形を描画
	for (int i = 0; i < mFaceNum; i++){
		//マテリアルの適用
		mMaterial[mpMaterialIndex[i]]->Enabled();
		//図形の描画
		glDrawElements(GL_TRIANGLES, 3,
			GL_UNSIGNED_INT, (mpVertexIndex + i * 3));
	}
	//データ配列の無効
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void CModelXFrame::Render(){
	if (mMesh.mFaceNum != 0)
		mMesh.Render();
}

void CModelX::Render(){
	for (int i = 0; i < mFrame.size(); i++){
		mFrame[i]->Render();
	}

}
