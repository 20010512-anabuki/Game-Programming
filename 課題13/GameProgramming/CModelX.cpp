#include"glut.h"
#include<stdio.h>
#include<string.h>
#include"CModelX.h"
#include"CMaterial.h"

void CModelX::Load(char*file){
	//�t�@�C���T�C�Y�̊l��
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
	//�C�e���[�^�̍쐬
	std::vector<CModelXFrame*>::iterator itr;
	//�擪����Ō�܂ŌJ��Ԃ�
	for (itr = mFrame.begin(); itr != mFrame.end(); itr++){
		//���O����v�������ǂ���
		if (strcmp(name, (*itr)->mpName) == 0){
		//��v������A�h���X��Ԃ�
			return *itr;
		}
	}
	//��v���Ȃ��ꍇ��NULL��Ԃ�
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
	mFaceNum = model->GetIntToken(); //�ʐ��ǂݍ���
	mpVertexIndex = new int[mFaceNum * 3]; //���_����1�̖ʂ�3��
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
			//�@���f�[�^�̎擾
			mNormalNum = model->GetIntToken();
			//�@���f�[�^��z���
			CVector* pNormal = new CVector[mNormalNum];
			for (int i = 0; i < mNormalNum; i++) {
				pNormal[i].mX = model->GetFloatToken();
				pNormal[i].mY = model->GetFloatToken();
				pNormal[i].mZ = model->GetFloatToken();
			}
			//�@����=�ʐ�*3
			mNormalNum = model->GetIntToken() * 3;//FaceNum
			int ni;
			//���_���Ƃɖ@���f�[�^��ݒ�
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
			//�}�e���A���̐�
			mMaterialNum = model->GetIntToken();
			mMaterialIndexNum = model->GetIntToken();
			mpMaterialIndex = new int[mMaterialIndexNum];
			for (int i = 0; i < mMaterialIndexNum; i++) {
				mpMaterialIndex[i] = model->GetIntToken();
			}
			//�}�e���A���f�[�^�̍쐬
			for (int i = 0; i < mMaterialNum; i++) {
				model->GetToken();
				if (strcmp(model->mToken, "Material") == 0) {
					mMaterial.push_back(new CMaterial(model));
				}
			}
			model->GetToken();	//}//End of MeshMaterialList
		}
		//SkinWeights�̎�
		else if (strcmp(model->mToken, "SkinWeights") == 0){
			mSkinWeights.push_back(new CSkinWeights(model));
		}
		else {
			//�m�[�h�̓ǂݔ�΂�
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
	//�t���[�����G���A�m�ۂƐݒ�
	mpFrameName = new char[strlen(model->mToken) + 1];
	strcpy(mpFrameName, model->mToken);
	//���_�ԍ��擾
	mIndexNum = model->GetIntToken();
	//���_�ԍ���0�𒴂����ꍇ
	if (mIndexNum > 0){
		mpIndex = new int[mIndexNum];
		mpWeight = new float[mIndexNum];
		for (int i = 0; i < mIndexNum; i++)
			mpIndex[i] = model->GetIntToken();
		for (int i = 0; i < mIndexNum; i++)
			mpWeight[i] = model->GetFloatToken();
	}
	//�I�t�Z�b�g�s��擾
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
	//�A�j���[�V�����Z�b�g����ޔ�
	mpName = new char[strlen(model->mToken) + 1];
	strcpy(mpName, model->mToken);
	model->GetToken();//{
	while (*model->mpPointer != '\0'){
		model->GetToken();// } or Animation
		if (strchr(model->mToken, '}'))break;
		if (strcmp(model->mToken, "Animation") == 0){
			//Animation�v�f�̓ǂݍ���
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
	//�L�[�̔z���ۑ����Ă����z��
	CMatrix*key[4] = { 0, 0, 0, 0 };
	//���Ԃ̔z���ۑ����Ă����z��
	float*time[4] = { 0, 0, 0, 0 };
	while (*model->mpPointer != '\0'){
		model->GetToken(); // } or AnimationKey
		if (strchr(model->mToken, '}'))break;
		if (strcmp(model->mToken, "AnimationKey") == 0){
			model->GetToken(); // {
			//�f�[�^�^�C�v�擾
			int type = model->GetIntToken();
			//���Ԑ��擾
			mKeyNum = model->GetIntToken();
			switch (type){
			case 0://Rotation Quaternion
				//�s��̔z������Ԑ����m��
				key[type] = new CMatrix[mKeyNum];
				time[type] = new float[mKeyNum];
				//���Ԃ��J��Ԃ�
				for (int i = 0; i < mKeyNum; i++){
					//���Ԏ擾
					time[type][i] = model->GetFloatToken();
					model->GetToken();//4��ǂݔ�΂�
					//w,x,y,z���擾
					float w = model->GetFloatToken();
					float x = model->GetFloatToken();
					float y = model->GetFloatToken();
					float z = model->GetFloatToken();
					//�N�H�[�^�j�I�������]�s��ɕϊ�
					key[type][i].SetQuaternion(x, y, z, w);
				}
				break;
			case 1://�g��k���̍s��쐬
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
			case 2://�ړ��̍s��쐬
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
			case 4://�s��f�[�^���擾
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
	//�s��f�[�^�ł͂Ȃ��Ƃ�
	if (mpKey == 0){
		//�����ԕ��L�[���쐬
		mpKey = new CAnimationKey[mKeyNum];
		for (int i = 0; i < mKeyNum; i++){
			//���Ԑݒ�
			mpKey[i].mTime = time[2][i];//Time
			//�s��쐬 Size*Rotation*Position
			mpKey[i].mMatrix = key[1][i] * key[0][i] * key[2][i];
		}
	}
	//�m�ۂ����G���A�J��
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
	//���_�f�[�^�A�@���f�[�^�̗L��
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	//�f�[�^�̏ꏊ�w��
	glVertexPointer(3, GL_FLOAT, 0, mpVertex);
	glNormalPointer(GL_FLOAT, 0, mpNormal);
	//���_�̃C���f�b�N�X�̏ꏊ���w�肵�A�}�`��`��
	for (int i = 0; i < mFaceNum; i++){
		//�}�e���A���̓K�p
		mMaterial[mpMaterialIndex[i]]->Enabled();
		//�}�`�̕`��
		glDrawElements(GL_TRIANGLES, 3,
			GL_UNSIGNED_INT, (mpVertexIndex + i * 3));
	}
	//�f�[�^�z��̖���
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
