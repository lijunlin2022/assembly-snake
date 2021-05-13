#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

// 移动方向
int g_nMoveFlag;

// 定义一个清屏工具类函数
void ClearScreen() {
	const char* szClearScreen = "cls";
	__asm {
		mov eax, dword ptr ds : [szClearScreen]
		push eax
		call system
		add esp, 4
	}
}

// 定义一个工具类函数，在指定位置打印
void gotoxy(int pos) {
	__asm {
		mov eax, dword ptr ds:[pos]
		push eax
		push -11
		call dword ptr ds:[GetStdHandle]
		push eax
		call dword ptr ds:[SetConsoleCursorPosition]
	}
}

void JudgeUserMoveFlag() {
	int flag;
	while (true) {
		__asm {
		back_while:
			// 获取 w 键
			push 87
			call dword ptr ds:[GetAsyncKeyState]
			and ax, 0xff00		// 与操作，获取第 15 位的值
			cmp ax, 0		// 如果为 0 表示没有被按下
			jne w_press
			// 获取 s 键
			push 83
			call dword ptr ds : [GetAsyncKeyState]
			and ax, 0xff00		
			cmp ax, 0		
			jne s_press
			// 获取 a 键
			push 65
			call dword ptr ds : [GetAsyncKeyState]
			and ax, 0xff00		
			cmp ax, 0		
			jne a_press
			// 获取 d 键
			push 68
			call dword ptr ds : [GetAsyncKeyState]
			and ax, 0xff00		
			cmp ax, 0		
			jne d_press
		w_press:
			mov eax, dword ptr ds:[g_nMoveFlag]
			cmp eax, 2	// 当前移动方向是否向下
			je w_back
			mov dword ptr ds:[g_nMoveFlag], 1
		w_back:
			jmp back_while
		s_press :
			mov eax, dword ptr ds : [g_nMoveFlag]
			cmp eax, 1	// 当前移动方向是否向下
			je s_back
			mov dword ptr ds : [g_nMoveFlag] , 2
		s_back :
			jmp back_while
		a_press:
		    mov eax, dword ptr ds : [g_nMoveFlag]
			cmp eax, 4	// 当前移动方向是否向右
			je a_back
			mov dword ptr ds : [g_nMoveFlag] , 3
		a_back :
			jmp back_while
		d_press:
			mov eax, dword ptr ds : [g_nMoveFlag]
			cmp eax, 3	// 当前移动方向是否向左
			je d_back
			mov dword ptr ds : [g_nMoveFlag] , 4
		d_back :
			jmp back_while
		}
	}
}

// 判断是否死亡, 0 表示未死亡，1 表示死亡
int g_nIsDead;

// 蛇的数组
struct SnakePos {
	int x;
	int y;
} g_sSnakePosArray[100];

// 蛇的长度
int g_nSnakeLength;

// 地图的数组
char g_MapDataArr[20][20];

void ShowMainMenu();	// 显示主菜单
void GetUserInput(int nSelect);	// 获取用户输入
void DispatchFunction();		// 分发函数
void EndGame();	// 结束游戏
void StartGame();	// 开始游戏
void InitGameMapData();	// 初始化地图数据
void SetWalls();	// 设置地图边界
void RandSetSnakePosition();		// 随机设置蛇的位置
void RandSetFoodPosition();		// 随机设置食物的位置
void DrawMap(); // 画出地图
void DrawSnake();	// 画蛇
void MoveSnake();	// 移动蛇
void JudgeMoveState();	// 判断此次移动状态
void Death();	// 死亡
void GetScore();	// 得分
void AddSnakeLength();	// 增加蛇的长度


// 显示主菜单
void ShowMainMenu() {
	// 变量的定义
	int nSelect;
	// 文本的定义
	const char *format1 = "#####################\n";
	const char* format2 = "code by: LiJunLin\n";
	const char* format3 = "1. 开始游戏; 2. 结束游戏\n";
	const char* format4 = "您输入的编号不支持, 请在 2 秒之后重新输入\n";

	__asm {
	show_menu_begin:
		// 打印分割字符
		mov eax, dword ptr ds:[format1]
		push eax
		call printf
		add esp, 4

		// 打印水印
		mov eax, dword ptr ds:[format2]
		push eax
		call printf
		add esp, 4

		// 打印提示信息
		mov eax, dword ptr ds:[format3]
		push eax
		call printf
		add esp, 4

		// 打印分割用字符串
		mov eax, dword ptr ds:[format1]
		push eax
		call printf
		add esp, 4

		// 获取用户输入
		lea eax, dword ptr ds:[nSelect]
		push eax
		call GetUserInput
		add esp, 4

		// 判断并且分发
		mov eax, dword ptr ds:[nSelect]
		cmp eax, 1
		je dispatch_begin_game
		cmp eax, 2
		je dispatch_end_game

		// 输入不被支持
		mov eax, dword ptr ds:[format4]
		push eax
		call printf
		add esp, 4

		// 延时函数
		push 2000
		call dword ptr ds:[Sleep]
		
		// 清屏
		call ClearScreen

		jmp show_menu_begin
	
	dispatch_begin_game:
		mov dword ptr ds:[g_nIsDead], 0	// 游戏开始， 0 表示未死亡
		call InitGameMapData			// 初始化游戏地图数据
		call RandSetFoodPosition		// 随机设置食物位置
		call RandSetSnakePosition		// 随机设置蛇的位置
	
	go_on_game:
		call DrawMap							// 画出地图、食物
		call DrawSnake
		mov eax, dword ptr ds:[g_nIsDead]
		cmp eax, 0								// 判断有没有死亡，未死亡则重新绘制地图，死亡则回到主菜单
		je go_on_game

		jmp show_menu_begin			// 回到主菜单
	    nop

	dispatch_end_game:
		call EndGame
	    nop
	}
}

// 获取用户输入
void GetUserInput(int nSelect) {
	// 文本的定义
	const char *format1 = "请输入你要选择的编号:\n";
	const char* format2 = "%d";
	// 开始调用
	__asm {
		// 显示提示信息
		mov eax, dword ptr ds:[format1]
		push eax
		call printf
		add esp, 4
		// 获取输入
		mov eax, dword ptr ds:[nSelect]
		push eax
		mov ecx, dword ptr ds:[format2]
		push ecx
		call scanf
		add esp, 8
	}
}

// 分发函数
void DispatchFunction() {

}

// 结束游戏
void EndGame() {
	const char* format1 = "程序在两秒钟之后即将退出......";
	__asm {
		// 打印提示信息
		mov eax, dword ptr ds : [format1]
		push eax
		call printf
		add esp, 4

		// 延时 2s
		push 2000
		call Sleep
		add esp, 4

		// 退出
		push 0
		call ExitProcess
		add esp, 4
	}
}


// 初始化地图数据
void InitGameMapData() {
	const char* format1 = "内存已经清理完毕, 即将开始游戏......\n";
	int i;
	__asm {
		// 20 * 20 的数据, 清零 memset 函数
		push 400
		push 0
		lea eax, dword ptr ds:[g_MapDataArr]
		push eax
		call memset
		add esp, 12

		/* 使用循环设置地图的边界 */
	    mov dword ptr ds : [i] , 0
		mov ecx, 20;

	
	set_wall:
		// 顶墙
		lea eax, dword ptr ds : [g_MapDataArr]
		mov ebx, dword ptr ds : [i]
		mov byte ptr ds : [eax + ebx] , 0xB
		// 底墙
		lea eax, dword ptr ds : [g_MapDataArr]
		mov ebx, 19
		imul ebx, ebx, 20
		add eax, ebx
		mov ebx, dword ptr ds : [i]
		mov byte ptr ds : [eax + ebx] , 0xB
		// 左墙
		lea eax, dword ptr ds : [g_MapDataArr]
		mov ebx, dword ptr ds : [i]
		imul ebx, ebx, 20
		mov byte ptr ds : [eax + ebx] , 0xB
		// 右墙
		lea eax, dword ptr ds:[g_MapDataArr]
		mov ebx, dword ptr ds : [i]
		imul ebx, ebx, 20
		add ebx, 19
		mov byte ptr ds : [eax + ebx] , 0xB
		// i 自减
		mov ebx, dword ptr ds : [i]
		inc ebx
		mov dword ptr ds : [i] , ebx
		loop set_wall

		// 打印提示信息
		mov eax, dword ptr ds:[format1]
		push eax
		call printf
		add esp, 4
	}
}

// 设置地图边界
void SetWalls() {

}

// 随机设置蛇的位置
void RandSetSnakePosition() {
	int x;
	int y;
	int flag;
	__asm {
		// 初始化蛇 memset(0, 0, 0)
		push 800
		push 0
		lea eax, dword ptr ds:[g_sSnakePosArray]
		push eax
		call memset
		add esp, 12

		// 初始化长度
		mov dword ptr ds : [g_nSnakeLength] , 0;
		mov dword ptr ds: [flag], 0

	set_snake_pos_begin:
		// 取当前时间
		push 0
		call time
		add esp, 4
		// 设置随机数种子
		add eax, 23	// 为了避免和食物的位置重叠，给随机数加上一个固定值
		push eax
		call srand
		add esp, 4
		// 取随机数 行
		call rand	
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds:[x], edx
		// 取随机数 列
		call rand
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds:[y], edx
		// 判断是否在墙上
	set_snake_judge_iswall:

		lea eax, dword ptr ds:[g_MapDataArr]
		mov ecx, dword ptr ds:[x]
		imul ecx, ecx, 20
		add eax, ecx
		mov ecx, dword ptr ds:[y]
		add eax, ecx

		mov al, byte ptr ds:[eax]
		mov ecx, dword ptr ds:[flag]	// 判断是否已经确定蛇头位置
		cmp ecx, 1
		je set_snake_pos_second_judge
		jne set_snake_pos_first_judge

	set_snake_pos_second_judge:

		cmp al, 0xB	// 是否已经撞墙
		je set_snake_pos_end
		jne set_snake_pos_write
		
	set_snake_pos_first_judge:

		cmp al, 0xb
		je set_snake_pos_begin
		mov dword ptr ds:[flag], 1	// 证明确定蛇头位置

		// 写入到位置数组
	set_snake_pos_write:
		lea eax, dword ptr ds:[g_sSnakePosArray]
		mov ecx, dword ptr ds:[g_nSnakeLength]
		imul ecx, ecx, 8
		add eax, ecx

		mov ecx, dword ptr ds:[x]	// 写入 x
		mov dword ptr ds:[eax], ecx
		mov ecx, dword ptr ds:[y]
		mov dword ptr ds:[eax + 4], ecx

		// 判断是否继续循环
		mov eax, dword ptr ds:[g_nSnakeLength]
		inc eax
		mov dword ptr ds:[g_nSnakeLength], eax
		cmp eax, 4
		jg set_snake_pos_end
		mov eax, dword ptr ds:[y]
		inc eax
		mov dword ptr ds:[y], eax
		jmp set_snake_judge_iswall

	set_snake_pos_end:
		nop
	}
}

// 随机设置食物的位置
void RandSetFoodPosition() {
	const char* format1 = "正在随机生成食物的位置......\n";
	int x;
	int y;

	__asm {
		mov eax, dword ptr ds : [format1]
		push eax
		call printf
		add esp, 4

		rand_set_food_pos :
		push 0
		call time
		add esp, 4

		// 设置随机数种子
		push eax
		call srand
		add esp, 4

		call rand
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds : [x] , edx

		call rand
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds : [y] , edx

		// 判断食物是不是和墙壁重叠
		lea eax, dword ptr ds : [g_MapDataArr]
		mov ecx, dword ptr ds : [x]
		imul ecx, ecx, 20
		add eax, ecx
		mov edx, dword ptr ds : [y]
		add eax, edx

		mov cl, byte ptr ds : [eax]
		cmp cl, 0xB	// 判断是否是墙
		je rand_set_food_pos
		// 如果不是墙， 往里面设置食物
		mov byte ptr ds : [eax] , 0xC
	}
}

// 画出地图
void DrawMap() {

	int i;
	int j;
	const char *szWall = "-";
	const char* szFood = "*";
	const char* szNone = " ";
	const char* szChangeLine = "\n";
	const char* format1 = "%s";

	__asm {

		// 清屏
		call ClearScreen

		// 双重循环打印 g_MapDataArr
		mov dword ptr ds:[i], 0
		jmp print_first_for_cmp
	print_first_for_inc:						// 第一个循环控制变量 ++ 的地方
		mov eax, dword ptr ds:[i]
		inc eax
		mov dword ptr ds:[i], eax
	print_first_for_cmp:						// 第一个循环控制变量作对比的地方
		mov eax, dword ptr ds:[i]
		cmp eax, 20
		jge print_first_for_end
		// 第二个循环							// 第一个循环的循环代码开始的地方
		mov dword ptr ds:[j], 0
		jmp print_second_for_cmp
	print_second_for_inc:						
		mov eax, dword ptr ds:[j]
		inc eax
		mov dword ptr ds:[j], eax
	print_second_for_cmp:					
		mov eax, dword ptr ds:[j]
		cmp eax, 20
		jge print_second_for_end
		// code...

				lea eax, dword ptr ds:[g_MapDataArr]
				mov ecx, dword ptr ds:[i]
				imul ecx, ecx, 20
				add eax, ecx
				mov ecx, dword ptr ds:[j]
				add eax, ecx

				mov al, byte ptr ds:[eax]
				cmp al, 0xB
				je print_map_draw_wall
				cmp al, 0xC
				je print_map_draw_food

				// 打印空格
				mov eax, dword ptr ds : [szNone]
				push eax
				mov ecx, dword ptr ds : [format1]
				push ecx
				call printf
				add esp, 8
				jmp print_second_for_inc

			print_map_draw_wall:				// 打印墙壁

				mov eax, dword ptr ds:[szWall]
				push eax
				mov ecx, dword ptr ds:[format1]
				push ecx
				call printf
				add esp, 8
				jmp print_second_for_inc

			print_map_draw_food:		// 打印食物

				mov eax, dword ptr ds : [szFood]
				push eax
				mov ecx, dword ptr ds : [format1]
				push ecx
				call printf
				add esp, 8
				jmp print_second_for_inc



			print_second_for_end:

					mov eax, dword ptr ds : [szChangeLine]
					push eax
					mov ecx, dword ptr ds : [format1]
					push ecx
					call printf
					add esp, 8
			         jmp print_first_for_inc

	print_first_for_end:
		nop
	}
}

// 画蛇
void DrawSnake() {
	int i;
	const char* format = "*";
	__asm {
		mov dword ptr ds:[i], 0
		jmp print_snake_cmp
	print_snake_inc:
		mov eax, dword ptr ds:[i]
		inc eax
		mov dword ptr ds:[i], eax
	print_snake_cmp:
		mov eax, dword ptr ds:[i]
		inc eax
		mov ecx, dword ptr ds:[g_nSnakeLength]
		cmp eax, ecx				// 如果画好的长度等于储存的贪吃蛇的长度， 则游戏结束
		jge print_snake_end

		// 执行代码
		lea eax, dword ptr ds:[g_sSnakePosArray]		// 计算地址
		mov ecx, dword ptr ds:[i]
		imul ecx, ecx, 8
		add eax, ecx

		mov ecx, dword ptr ds:[eax]	// 取出 x
		shl ecx, 16 // 左移 16 位
		mov edx, dword ptr ds:[eax + 4] // 取出 y
		or ecx, edx	// 拼接完成
		push ecx		// 设置光标位置
		call gotoxy
		add esp, 4

		// 打印蛇的身体
		mov eax, dword ptr ds:[format]
		push eax
		call printf
		add esp, 4
		jmp print_snake_inc

	print_snake_end:
	}
}

// 移动蛇
void MoveSnake() {

}

// 判断此次移动状态
void JudgeMoveState() {

}

// 死亡
void Death() {

}

// 得分
void GetScore() {

}

// 增加蛇的长度
void AddSnakeLength() {
	// ...... 增加了长度
	
}

void main() {
	__asm {
		call ShowMainMenu
	}
}
