#ifndef CMODELX_H
#define CMODELX_H
#include<vector>
#include"CMatrix.h"
#include"CVector.h"
class CModelX;

#define MODEL_FILE "sample.blend.x"
//領域開放をマクロ化
#define SAFE_DELETE_ARRAY(a){if(a)delete[]a;a=0;}
#define ARRY_SIZE(a)(sizeof(a)/sizeof(a[0]))

class CMesh{
public:
	int mVertexNum; //頂点数
	CVector*mpVertex; //頂点データ
	int mFaceNum; //面積
	int *mpVertexIndex; //面を構成する頂点番号
	int mNormalNum; //法線数
	CVector *mpNormal;
	//コンストラクタ
	CMesh()
		:mVertexNum(0)
		, mpVertex(0)
	    ,mFaceNum(0)
	    ,mpVertexIndex(nullptr)
		,mNormalNum(0)
		, mpNormal(nullptr)
	{}
	//デストラクタ
	~CMesh(){
		SAFE_DELETE_ARRAY(mpVertex);
		SAFE_DELETE_ARRAY(mpVertexIndex);
		SAFE_DELETE_ARRAY(mpNormal);
	}
	void Init(CModelX*model);
	void CMesh::Render(){
		//頂点データと法線データの配列を有効
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		//頂点データと法線データの場所を指定
		glVertexPointer(3,GL_FLOAT,0,mpVertex);
		glNormalPointer(GL_FLOAT,0,mpNormal);

		//頂点のインデックスの場所を指定して図形を描画
		glDrawElements(GL_TRIANGLES, 3 * mFaceNum, 
			GL_UNSIGNED_INT, mpVertexIndex);

		//頂点データ、法線データの配列無効
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
};


class CModelXFrame{
public:
	std::vector<CModelXFrame*>mChild;
	CMatrix mTransformMatrix;
	char* mpName;
	int mIndex;
	//Meshデータ
	CMesh mMesh;
	//コンストラクタ
	CModelXFrame(CModelX*model);
	//デストラクタ
	~CModelXFrame(){
		std::vector<CModelXFrame*>::iterator itr;
		for (itr = mChild.begin(); itr != mChild.end(); itr++){
			delete *itr;
		}
		SAFE_DELETE_ARRAY(mpName);
	}
	void CModelXFrame::Render(){
		if (mMesh.mFaceNum != 0)
			mMesh.Render();
	}
};


class CModelX{
public:
	char*mpPointer;
	char mToken[1024];

	std::vector<CModelXFrame*>mFrame;
	CModelX() :mpPointer(0)
	{}
	~CModelX(){
		if (mFrame.size() > 0){
			delete mFrame[0];
		}
	}
	void Load(char*file);
	void GetToken();
	//浮動小数点データの取得
	float GetFloatToken();
	void SkipNode();
	//整数データの取得
	int CModelX::GetIntToken(){
		GetToken();
		return atoi(mToken);
	}
	void CModelX::Render(){
		for (int i = 0; i < mFrame.size(); i++){
			mFrame[i]->Render();
		}
	}
};

#endif