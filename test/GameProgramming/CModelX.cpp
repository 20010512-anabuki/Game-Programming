#include"glut.h"
#include<stdio.h>
#include<string.h>
#include"CModelX.h"

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

float CModelX::GetFloatToken(){
	GetToken();
	return atof(mToken);
}

void CMesh::Init(CModelX*model){
	model->GetToken();
	if (!strchr(model->mToken, '{')){
		model->GetToken();//{
	}

	mVertexNum = model->GetIntToken();
	mpVertex = new CVector[mVertexNum];
	for (int i = 0; i < mVertexNum; i++){
		mpVertex[i].mX = model->GetFloatToken();
		mpVertex[i].mY = model->GetFloatToken();
		mpVertex[i].mZ = model->GetFloatToken();
	}

	mFaceNum = model->GetIntToken(); //面数読み込み
	mpVertexIndex = new int[mFaceNum * 3]; //頂点数は1つの面に3つ
	for (int i = 0; i < mFaceNum * 3; i += 3){
		model->GetToken();
		mpVertexIndex[i] = model->GetIntToken();
		mpVertexIndex[i + 1] = model->GetIntToken();
		mpVertexIndex[i + 2] = model->GetIntToken();
	}

	model->GetToken();
	if (strcmp(model->mToken, "MeshNormals") == 0){
		model->GetToken();//{
		//法線データの取得
		mNormalNum = model->GetIntToken();
		//法線データを配列へ
		CVector *pNormal = new CVector[mNormalNum];
		for (int i = 0; i < mNormalNum; i++){
			pNormal[i].mX = model->GetFloatToken();
			pNormal[i].mY = model->GetFloatToken();
			pNormal[i].mZ = model->GetFloatToken();
		}
		//法線数=面数*3
		mNormalNum = model->GetIntToken() * 3;//FaceNum
		int ni;
		//頂点ごとに法線データを設定
		mpNormal = new CVector[mNormalNum];
		for (int i = 0; i < mNormalNum; i += 3){
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


#ifdef _DEBUG
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

	printf("NormalNum:%d\n", mNormalNum);
	for (int i = 0; i < mNormalNum; i++){
		printf("%10f", mpNormal[i].mX);
		printf("%10f", mpNormal[i].mY);
		printf("%10f\n", mpNormal[i].mZ);
	}
#endif
}