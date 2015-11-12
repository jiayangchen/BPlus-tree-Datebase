#include "BPtree.h"
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <windows.h> 
#include <stdio.h>
using namespace std;

int main()
{
	cout << "***************" << endl;
	cout << "欢迎使用数据库" << endl;
	SYSTEMTIME sys;                               //显示系统时间
	GetLocalTime(&sys);
	printf("%4d/%02d/%02d", sys.wYear, sys.wMonth, sys.wDay); cout << endl;
	printf("%02d:%02d:%02d.%03d 星期%1d",  sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, sys.wDayOfWeek); cout << endl;

	cout << "***************" << endl;
	flag:
	cout << "1——打开数据库" << endl;
	cout << "2——新建数据库" << endl;
	cout << "3——设置" << endl;
	cout << "4——退出" << endl;
	char ch;
	string name,test;
	BPtree * Test = NULL;
	while (cin >> ch)                //主菜单
	{
		if (ch == '1')
		{
			cout << "请输入要打开的数据名称" << endl;
			cin >> name;
			
			Test = new BPtree(name.c_str(), false);
		}
		else if (ch == '2')
		{
			cout << "请输入要新建的数据文件名称" << endl;
			cin >> name;
			
			Test = new BPtree(name.c_str());
		}
		else if (ch == '3')
		{
			cout << "1——修改系统时间" << endl;
			cout << "2——修改数据库名称" << endl;
			cout << "3——返回上一级" << endl;
			cin >> ch;
			if (ch == '1')
			{
				system("time");   //有bug
			}
			if (ch == '2')
			{
				cout << "XXX" << endl;
			}
			if (ch == '3')
			{
				goto flag;
			}
		}
		else if (ch == '4')
		{
			cout << "成功退出" << endl;
			return 0;
		}
		else
		{
			cout << "命令错误" << endl;
			continue;
		}
		break;
	}
	cout << "**************************" << endl;         //加载数据库
	cout << "成功加载数据库——" << name << endl;
	cout << "**************************" << endl;
	cout << "1——插入数据" << endl;
	cout << "2——删除数据" << endl;
	cout << "3——修改数据" << endl;
	cout << "4——查找数据" << endl;
	cout << "5——测试" << endl;
	cout << "6——退出" << endl;
	
	char op;
	string key1, key2;
	while (cin >> op)
	{
		if (op == '1')
		{                             //插入记录
			//cout << "请输入要插入的Key和Value值，以空格隔开" << endl;
			ifstream ist("test001.txt");
			while (!ist.eof())
			{
				ist >> key1 >> key2;
				if (Test->Insert(key1.c_str(), key2.c_str()) == false)
				{
					cout << "插入失败" << endl;
				}
				/*cin >> key1 >> key2;
				if (Test->Insert(key1.c_str(), key2.c_str()) == false)
				{
				cout << "插入失败" << endl;
				}*/
				//cout << "插入完成" << endl;
				
			}
		}
		
		if (op == '2')
		{                             //删除记录
			cout << "请输入要删除数据的Key值" << endl;
			cin >> key1;
			Test->Delete(key1);
			cout << "删除成功" << endl;
		}
		if (op == '3')                   //修改记录
		{
			cout << "请输入要修改的Key和Value值，以空格隔开" << endl;
			cin >> key1 >> key2;
			if (Test->Replace(key1.c_str(), key2.c_str()) == true)
			{
				cout << "修改成功" << endl;
			}
			else
			{
				cout << "修改失败" << endl;
			}
		}
		if (op == '4')                    //查找记录
		{
			cout << "请输入要查找的数据的Key值" << endl;
			cin >> key1;
			string *tag = Test->Find(key1.c_str());  //查找目标记录
			if (tag == NULL)
			{
				cout << "记录不存在" << endl;
			}
			else
			{
				cout << *tag << endl; //反之输出value值
			}
		}
		if (op == '5')                                    //测试数据库
		{   
			/*cout << "1——插入测试" << endl;
			cout << "2——删除测试" << endl;
			cout << "3——查找测试" << endl;
			cout << "4——替换测试" << endl;*/

			clock_t start, finish,start1,finish1;
			ifstream ist("test002.dat");
			start = clock();
			while (!ist.eof())
			{
				ist >> key1 >> key2;
				if (Test->Insert(key1.c_str(), key2.c_str()) == false)
				{
					cout << "插入失败" << endl;
				}
				//cout << "插入完成" << endl;
				/*else
				{
				cout << "插入成功" << endl;
				}*/
			}
			finish = clock();
			cout << "全部插入成功" << endl;
			cout << "Running time is: " << static_cast<double>(finish - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
			
			//test
			/*ifstream istd("test08.dat");
			start1 = clock();
			while (!istd.eof())
			{
				istd >> key1 >> key2;
				Test->Replace(key1.c_str(), key2.c_str()) == true;
			}
			finish1 = clock();
			cout << "替换成功" << endl;
			cout << "Running time is: " << static_cast<double>(finish1 - start1) / CLOCKS_PER_SEC * 1000 << "ms" << endl;*/
		}
		if (op == '6')
		{            //退出
			cout << "程序正在退出，请稍等..." << endl;
			break;
		}
		
		//Test->printT();
	}
	delete Test;
	return 0;
}