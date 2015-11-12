
#include <iostream>
#include <string>
#include "BPtree.h"

using namespace std;

//B树的实现函数

BPtree::BPtree(string Dname, bool judgefile) :name(Dname)
{
	if (judgefile == true) 
	{
		bRoot = new DataNode;
		Fstream = fopen((name + DATSUFFIX).c_str(), "wb+"); //新建当前目录下的name文件，创建一个文件的读写
	}
	else  
	{
		FILE * Idxfile;
		Idxfile = fopen((name + IDXSUFFIX).c_str(), "rb");   
		bRoot = getindex(Idxfile, 0);                    //从文件流的开头读索引到内存
		fclose(Idxfile); //关闭文件流
		Fstream = fopen((name + DATSUFFIX).c_str(), "rb+");
	}
}

BPtree::~BPtree()
{
	FILE * Idxfile;
	Idxfile = fopen((name + IDXSUFFIX).c_str(), "wb");     //索引写入文件保存
	writeindex(Idxfile, bRoot);     //写入函数
	fclose(Idxfile);  //关闭索引文件流
	freemem(bRoot);   //内存释放
	fclose(Fstream);  //关闭流
}

bool BPtree::Insert(const string & newKey, const string & Newdat)  //插入实现
{
	BNode * nNode = bRoot;                                      //创建根节点
	while (nNode->species == BN_MID)                            //当node的类型是非叶子节点时
	{                                                          //递归寻找叶节点
		MidNode * nMid = (MidNode *)nNode;
		int i;
		for (i = 0; i<nMid->size; i++)
		{
			if (newKey<nMid->key[i])                          //循环找到比目标键值小的第一个键值
				break;                                        //若找到则跳出
		}
		nNode = nMid->child[i];                                
	}
	if (nNode->species == BN_LEAF)                             //node的类型已经是叶节点
	{
		DataNode * nLeaf = (DataNode *)nNode;                 
		int i, pos;                                             
		for (pos = 0; pos<nLeaf->size; pos++)
		{
			if (newKey < nLeaf->key[pos])                     //找到叶节点中的插入地点si
				break;
			if (newKey == nLeaf->key[pos])                    //如果本身就存在不需要插入
				return false;
		}
		for (i = nLeaf->size; i>pos; i--)
		{
			nLeaf->key[i] = nLeaf->key[i - 1];             //给要插入的数据腾出空间
			nLeaf->offset[i] = nLeaf->offset[i - 1];       //标记腾出的文件中的位置
		}
		nLeaf->key[pos] = newKey;                           //插入

		BDAT Predat;                                       
		Predat.len = int(Newdat.length());                 //获得插入数据的长度
		strcpy(Predat.dat, Newdat.c_str());                //复制写入
		nLeaf->offset[pos] = adddat(Fstream, &Predat);       //offset标记了文件里的位置
		nLeaf->size++;                                     //size+1

		//考虑插入后的平衡问题

		if (nLeaf->size == BT_M + 1)                      //如果超出leafsize
		{
			int cut = (BT_M + 1) / 2;                     //以中间的元素为分界，分裂叶节点
			DataNode * Rleaf = new DataNode;              //新建右节点   
			nLeaf->size = cut;
			Rleaf->size = BT_M + 1 - cut;                //更新右节点的size
			for (i = cut; i <= BT_M; i++){
				Rleaf->key[i - cut] = nLeaf->key[i];      //拷贝节点到新的右节点
				Rleaf->offset[i - cut] = nLeaf->offset[i];  //文件里的位置
			}
			
			MidNode * insertNode = nLeaf->father;
			string insertKey = nLeaf->key[cut];           //将中间元素上调至父节点
			BNode * lNode = nLeaf;
			BNode * rNode = Rleaf;
			
			if (insertNode == NULL)                      //如果是根节点
			{
				insertNode = new MidNode;
				bRoot = insertNode;
			}
			while (1)                                   //插入非叶节点同上
			{
				for (pos = 0; pos<insertNode->size; pos++)
				{
					if (insertKey<insertNode->key[pos]) //找到位置
						break;
				}
				for (i = insertNode->size; i>pos; i--)
				{
					insertNode->key[i] = insertNode->key[i - 1];     //拷贝
					insertNode->child[i + 1] = insertNode->child[i];
				}
				lNode->father = insertNode;         //左右指针链接到父节点
				rNode->father = insertNode;
				insertNode->key[pos] = insertKey;      
				insertNode->child[pos] = lNode;
				insertNode->child[pos + 1] = rNode;
				insertNode->size++;
				
				if (insertNode->size == BT_M + 1)                //分裂原本超出的叶节点
				{
					cut = (BT_M + 1) / 2;                      //取中间元素
					MidNode * rightMid = new MidNode;       
					insertNode->size = cut;
					rightMid->size = BT_M - cut;              //新的右节点的size
					for (i = cut + 1; i <= BT_M; i++)
					{
						rightMid->key[i - cut - 1] = insertNode->key[i];          //拷贝至新的右节点
						rightMid->child[i - cut - 1] = insertNode->child[i];
						rightMid->child[i - cut - 1]->father = rightMid;
					}
					rightMid->child[i - cut - 1] = insertNode->child[i];
					rightMid->child[i - cut - 1]->father = rightMid;

					insertKey = insertNode->key[cut];
					lNode = insertNode;
					rNode = rightMid;
					
					if (insertNode->father == NULL)                     //创建新的父节点，将原本的父节点作为新父节点的儿子
					{
						insertNode = new MidNode;
						bRoot = insertNode;
					}
					else
					{
						insertNode = insertNode->father;
					}
				}
				else
				{
					return true;
				}
			}
		}
		else
		{
			return true;
		}
	}
	return true;         //返回不需要插入
}

bool BPtree::Delete(const string & Tkey)
{
	BNode * nNode = bRoot;
	int i;
	while (1)
	{
		if (nNode->species == BN_MID)                          //首先还是找到要删的叶子节点
		{
			MidNode * nMid = (MidNode *)nNode;
			for (i = 0; i<nMid->size; i++)
			{
				if (Tkey<nMid->key[i])                        //找到就跳出
					break;
			}
			nNode = ((MidNode *)nNode)->child[i];
		}
		else if (nNode->species == BN_LEAF)                 //叶子节点里
		{
			DataNode * nLeaf = (DataNode *)nNode;
			for (i = 0; i<nLeaf->size; i++)
			{
				if (Tkey < nLeaf->key[i])
					return false;
				if (Tkey == nLeaf->key[i])                 //找打要删的目标key值
				{
					break;
				}
			}
			if (i == nLeaf->size)                          //循环到末尾还是没有找到则返回失败
				return false;
			
			//考虑删除的情况
			nLeaf->size--;                               
			for (; i<nLeaf->size; i++)                     //i指向了要删的key的位置
			{
				nLeaf->key[i] = nLeaf->key[i + 1];         //后面的key值前移
				nLeaf->offset[i] = nLeaf->offset[i + 1];   //文件里的位置标记也前移
			}
			if (nLeaf->father == NULL)                         //如果是根节点已删完
				return true;

			                                             
			int sti = BT_M / 2;                           //B+数性质的下限
			if (nLeaf->size<sti)                          //查看删除后是否小于B+数的要求
			{
				MidNode * deleteFrom = nLeaf->father;        //叶节点的父节点            
				BNode * deleteChild = nLeaf;             //删完后的叶节点

				while (1)
				{
					int deleteI;
					int i;
					
					for (i = 0; i <= deleteFrom->size; i++)     //找到要删除的那个节点的指针位置
						if (deleteFrom->child[i] == deleteChild)
							break;
					deleteI = i;                                  //记录下来
					
					//因为现在节点元素太少，需向两边的兄弟节点借一个元素
					if (deleteI > 0 && deleteFrom->child[deleteI - 1]->size>sti) //如果左边兄弟的key值个数比下限大，则可以借出
					{
						BNode * lBrother = deleteFrom->child[deleteI - 1];        //取左边节点中最大的key值
						for (i = deleteChild->size; i>0; i--)
							deleteChild->key[i] = deleteChild->key[i - 1];        //空出删除节点的最左边的位置
						
						if (deleteChild->species == BN_LEAF)
						{
							for (i = deleteChild->size; i>0; i--)
								((DataNode *)deleteChild)->offset[i] = ((DataNode *)deleteChild)->offset[i - 1];
							deleteChild->key[0] = lBrother->key[lBrother->size - 1];          //key值放入
							((DataNode *)deleteChild)->offset[0] = ((DataNode *)lBrother)->offset[lBrother->size - 1];
							deleteFrom->key[deleteI - 1] = deleteChild->key[0];  //更新父节点的索引key值
						}
						else
						{
							for (i = deleteChild->size + 1; i>0; i--)
							{
								((MidNode *)deleteChild)->child[i] = ((MidNode *)deleteChild)->child[i - 1];
								((MidNode *)deleteChild)->child[i]->father = (MidNode *)deleteChild;
							}
							deleteChild->key[0] = deleteFrom->key[deleteI - 1];
							deleteFrom->key[deleteI - 1] = lBrother->key[lBrother->size - 1];
							((MidNode*)deleteChild)->child[0] = ((MidNode *)lBrother)->child[lBrother->size];
							((MidNode*)deleteChild)->child[0]->father = (MidNode *)deleteChild;
						}
						deleteChild->size++;  //完成借key
						lBrother->size--;    
						return true;
					}
					//从右边兄弟借
					if (deleteI < deleteFrom->size && deleteFrom->child[deleteI + 1]->size>sti)   
					{
						BNode * rBrother = deleteFrom->child[deleteI + 1];
						if (deleteChild->species == BN_LEAF)
						{
							deleteChild->key[deleteChild->size] = rBrother->key[0];
							((DataNode *)deleteChild)->offset[deleteChild->size] = ((DataNode *)rBrother)->offset[0];
							deleteFrom->key[deleteI] = rBrother->key[1];
							for (i = 1; i<rBrother->size; i++)
								((DataNode *)rBrother)->offset[i - 1] = ((DataNode *)rBrother)->offset[i];
						}
						else
						{
							deleteChild->key[deleteChild->size] = deleteFrom->key[deleteI];
							deleteFrom->key[deleteI] = rBrother->key[0];
							((MidNode *)deleteChild)->child[deleteChild->size] = ((MidNode *)rBrother)->child[0];
							((MidNode *)deleteChild)->child[deleteChild->size]->father = (MidNode *)deleteChild;
							for (i = 0; i<rBrother->size; i++)
								((MidNode *)rBrother)->child[i] = ((MidNode *)rBrother)->child[i + 1];
						}
						for (i = 1; i<rBrother->size; i++)
							rBrother->key[i - 1] = rBrother->key[i];
						deleteChild->size++;  //完成借key
						rBrother->size--;
						return true;
					}

					//如果左右兄弟都不能借出，则将删除的节点里的key值合并入左节点
					if (deleteI>0)
					{
						BNode * lBrother = deleteFrom->child[deleteI - 1];
						if (lBrother->species == BN_LEAF)
						{
							for (i = 0; i<sti - 1; i++)
							{
								lBrother->key[sti + i] = deleteChild->key[i];      //并入
								((DataNode *)lBrother)->offset[sti + i] = ((DataNode *)deleteChild)->offset[i];
							}
							lBrother->size += sti - 1;   //更新左节点的size
						}
						else
						{
							lBrother->key[sti] = deleteFrom->key[deleteI - 1];
							for (i = 0; i<sti; i++)
							{
								lBrother->key[sti + 1 + i] = deleteChild->key[i];
								((MidNode *)lBrother)->child[sti + 1 + i] = ((MidNode *)deleteChild)->child[i];
								((MidNode *)lBrother)->child[sti + 1 + i]->father = (MidNode *)lBrother;
							}
							lBrother->size += sti;
						}
						for (i = deleteI; i<deleteFrom->size; i++)        //更新原本的父节点
						{
							deleteFrom->key[i - 1] = deleteFrom->key[i];
							deleteFrom->child[i] = deleteFrom->child[i + 1];
						}
					}
					else if (deleteI < deleteFrom->size)
					{
						BNode * rBrother = deleteFrom->child[deleteI + 1];      //向右合并
						if (rBrother->species == BN_LEAF)
						{
							for (i = 0; i<sti; i++)
							{
								deleteChild->key[sti - 1 + i] = rBrother->key[i];
								((DataNode *)deleteChild)->offset[sti - 1 + i] = ((DataNode *)rBrother)->offset[i];
							}
							deleteChild->size += sti;
						}
						else
						{
							deleteChild->key[sti - 1] = deleteFrom->key[deleteI];
							for (i = 0; i<sti + 1; i++)
							{
								deleteChild->key[sti + i] = rBrother->key[i];
								((MidNode *)deleteChild)->child[sti + i] = ((MidNode *)rBrother)->child[i];
								((MidNode *)deleteChild)->child[sti + i]->father = (MidNode *)deleteChild;
							}
							deleteChild->size += sti + 1;
						}
						for (i = deleteI + 1; i<deleteFrom->size; i++)
						{
							deleteFrom->key[i - 1] = deleteFrom->key[i];
							deleteFrom->child[i] = deleteFrom->child[i + 1];
						}
					}
					deleteFrom->size--;
		
					if (deleteFrom->father == NULL)       //如果父节点为根节点
					{
						if (deleteFrom->size == 0)
						{
							bRoot = deleteFrom->child[0];    //新建父节点
							bRoot->father = NULL;
							delete deleteFrom;
						}
						return true;
					}
					else if (deleteFrom->size<sti)
					{
						deleteChild = deleteFrom;
						deleteFrom = deleteFrom->father;
					}
					else
					{
						return true;
					}
				}
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

string * BPtree::Find(const string & Tkey)const
{
	BNode * nNode = bRoot;
	int i;
	while (1)
	{
		if (nNode->species == BN_MID)                //如果是非叶子结点
		{
			MidNode * nMid = (MidNode *)nNode;
			for (i = 0; i<nMid->size; i++)           //循环找到比目标键值大的第一个键值
			{
				if (Tkey<nMid->key[i])                //若找到则跳出
					break;
			}
			nNode = ((MidNode *)nNode)->child[i];
		}
		else if (nNode->species == BN_LEAF)        //如果就是叶子节点
		{
			DataNode * nLeaf = (DataNode *)nNode;
			for (i = 0; i<nLeaf->size; i++)         //循环寻找目标键值
			{
				if (Tkey < nLeaf->key[i])           //循环结束都小于的话直接返回未找到
					return NULL;
				if (Tkey == nLeaf->key[i])          //找到相应键值的话
				{
					BDAT Tdat;                   //目标查找数据
					getintofile(Fstream, nLeaf->offset[i], &Tdat);     //从文件中找出数据
					return (new string(Tdat.dat));     //返回目标数据值
				}
			}
			return NULL;      //反之，返回失败
		}
	}
}

bool BPtree::Replace(const string & Tkey, const string & Newdat)       //替换
{
	BNode * nNode = bRoot;
	int i;
	while (1)
	{
		if (nNode->species == BN_MID)                        //依旧是先查找到要替换的键值数据在哪里
		{
			MidNode * nMid = (MidNode *)nNode;
			for (i = 0; i<nMid->size; i++)
			{
				if (Tkey<nMid->key[i])
					break;
			}
			nNode = ((MidNode *)nNode)->child[i];
		}
		else if (nNode->species == BN_LEAF)               //已经是叶子节点时
		{
			DataNode * nLeaf = (DataNode *)nNode;
			for (i = 0; i<nLeaf->size; i++)
			{
				if (Tkey < nLeaf->key[i])              
					return false;
				if (Tkey == nLeaf->key[i])          //命中则改写
				{
					BDAT Predat;
					Predat.len = int(Newdat.length()); //用新数据的长度替换旧数据的长度
					strcpy(Predat.dat, Newdat.c_str());  //复制替换
					writeintofile(Fstream, nLeaf->offset[i], &Predat); //写入文件
					return true;
				}
			}
			return false;  //反之失败
		}
	}
}



