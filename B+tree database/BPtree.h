#ifndef _BTREE_H_
#define _BTREE_H_

#include <iostream>
#include <string>
using namespace std;

#define DATSUFFIX ".dat" //数据文件
#define IDXSUFFIX ".idx"  //索引文件

#define BT_M 16            //B+树阶数
#define BN_MID 0          //标记叶子和非叶子节点
#define BN_LEAF 1

#define MAXDATLEN 50      //数据data的最大长度
#define MAXKEYLEN 10       //索引key值得最大长度

class BPtree
{
	//主要功能函数
public:
	BPtree(string dtn, bool newfile = true);             //初始化
	~BPtree();                                           //析构函数
	bool Insert(const string &, const string &);		//判断是否插入成功
	bool Delete(const string &);                     //判断是否删除成功
	string * Find(const string &)const;              //查找的记录是否为空
	bool Replace(const string &, const string &);	    //判断是否修改成功

private:
	string name;  //数据库的名字
	FILE * Fstream;  //文件流
	
	typedef struct Bdata
	{
		int len;              //标记数据长度
		char dat[MAXDATLEN];  
	}BDAT;

	struct SNode   //储存索引选用的结构体
	{
		char type;
		int s;
		char key[BT_M + 1][MAXKEYLEN];
		long long offset[BT_M + 1];       //与key值对应
	};

	class MidNode;

	class BNode       //节点类
	{
	public:
		int species;
		int size;
		MidNode * father;
		BNode(){ size = 0; father = NULL; }
		string key[BT_M + 1];   //key值
	};

	BNode * bRoot;  

	class MidNode :public BNode //非叶节点
	{
	public:
		BNode *child[BT_M + 2];                    //孩子数组，个数比key多1
		MidNode(){ species = BN_MID; }          
	};

	class DataNode :public BNode    //叶子节点
	{
	public:
		long long offset[BT_M + 1];
		DataNode(){ species = BN_LEAF; }
	};

	void writeintofile(FILE * stream, long long offset, const BDAT * val)   //将数据写入文件
	{
		long long curpos;  
		curpos = ftell(stream); 
		fseek(stream, offset, SEEK_SET);                      //定位到写数据的地址
		fwrite(val, sizeof(BDAT), 1, stream);                  //向流里写进数据
		fseek(stream, curpos, SEEK_SET);                      //指针指向末尾           
		return;
	}

	void getintofile(FILE * stream, long long offset, BDAT * val) const  //获取数据
	{
		long long curpos;
		curpos = ftell(stream);
		fseek(stream, offset, SEEK_SET);                    //定位到读数据的地址
		fread(val, sizeof(BDAT), 1, stream);                //从流里读数据
		fseek(stream, curpos, SEEK_SET);                         
		return;
	}

	void writeindex(FILE * stream, const BNode * nNode)   //将索引写入文件
	{
		long long curpos;
		curpos = ftell(stream);
		SNode Temp;                         //文件里储存索引的node
		if (nNode->species == BN_MID)      //如果是非叶节点
		{
			fwrite(&Temp, sizeof(SNode), 1, stream);
			MidNode * tmpNode = (MidNode *)nNode;        //需要写进的索引node
			Temp.type = 0;
			Temp.s = tmpNode->size;
			int i;
			for (i = 0; i<tmpNode->size; i++)
			{
				strcpy_s(Temp.key[i], tmpNode->key[i].c_str());  //将需要写进文件的索引的key赋给储存的snode
			}
			for (i = 0; i <= tmpNode->size; i++)
			{
				Temp.offset[i] = ftell(stream);
				writeindex(stream, tmpNode->child[i]);      //递归写进文件
			}

			fseek(stream, curpos, SEEK_SET);
			fwrite(&Temp, sizeof(SNode), 1, stream);
		}
		else if (nNode->species == BN_LEAF)
		{
			DataNode * tmpNode = (DataNode *)nNode;
			Temp.type = 1;
			Temp.s = tmpNode->size;
			int i;
			for (i = 0; i<tmpNode->size; i++)
			{
				strcpy_s(Temp.key[i], tmpNode->key[i].c_str());        //如果是叶子节点直接读进，赋予key值和位置信息
				Temp.offset[i] = tmpNode->offset[i];
			}
			fwrite(&Temp, sizeof(SNode), 1, stream);
		}
		fseek(stream, 0L, SEEK_END);                          //读完指向文件末尾
		return;
	}

	BNode * getindex(FILE * stream, long long offset)       //从文件读索引，就是利用索引建树的过程
	{
		SNode Temp;
		fseek(stream, offset, SEEK_SET);
		fread(&Temp, sizeof(SNode), 1, stream);    //从流中读一个索引                 
		if (Temp.type == 0)                        //如果是非叶节点
		{
			MidNode * retNode = new MidNode;        //新建节点
			retNode->species = BN_MID;
			retNode->size = Temp.s;
			int i;
			for (i = 0; i<retNode->size; i++)
			{
				retNode->key[i] = Temp.key[i];         //赋予key值
			}
			for (i = 0; i <= retNode->size; i++)
			{
				retNode->child[i] = getindex(stream, Temp.offset[i]);  //递归继续读取索引直到叶子节点
				retNode->child[i]->father = retNode;
			}
			return retNode;
		}
		else
		{                                                         //如果是叶节点
			DataNode * retNode = new DataNode;
			retNode->species = BN_LEAF;
			retNode->size = Temp.s;
			int i;
			for (i = 0; i<retNode->size; i++)
			{
				retNode->key[i] = Temp.key[i];               //赋予key值和位置信息
				retNode->offset[i] = Temp.offset[i];
			}
			return retNode;
		}
	}

	long long adddat(FILE * stream, const BDAT * val)  //文件尾添加数据
	{                                                              
		long long curpos, length;
		curpos = ftell(stream);
		fseek(stream, 0L, SEEK_END);
		length = ftell(stream);
		fwrite(val, sizeof(BDAT), 1, stream);
		fseek(stream, curpos, SEEK_SET);
		return	length;
	}

	void freemem(BNode * fNode)        //释放节点内存的函数
	{
		if (fNode->species == BN_LEAF)   //叶子节点
		{
			delete fNode;          //删除
			return;
		}
		int i;
		for (i = 0; i <= fNode->size; i++)
			freemem(((MidNode *)fNode)->child[i]);
		delete fNode;
		return;
	}
	
};

#endif