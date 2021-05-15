#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

// 地图数组
char g_MapDataArr[20][20];

// 移动方向
int g_nMoveFlag;

// 撞墙标记, 0 表示未撞墙，1 表示撞墙
int g_nIsDead;

// 蛇的数组
struct SnakePos {
	int x;
	int y;
} g_sSnakePosArray[100];

// 蛇的长度
int g_nSnakeLength;

// 工具函数, 用于清屏
void ClearScreen();
// 工具函数, 用于在指定位置打印
void gotoxy(int pos);

// 显示主菜单
void ShowMainMenu();	
// 获取用户输入
void GetUserInput(int nSelect);
// 结束游戏
void EndGame();
// 初始化地图数据
void InitGameMapData();
// 随机设置食物的位置
void RandSetFoodPosition();
// 随机设置蛇的位置
void RandSetSnakePosition();
// 画地图
void DrawMap();
// 画蛇
void DrawSnake();
// 判断此次移动状态
void JudgeMoveFlag();
// 移动蛇
void MoveSnake();
// 死亡
void Death();	
// 得分
void GetScore();
// 增加蛇的长度
void AddSnakeLength();

void ClearScreen() {
	const char* szClearScreen = "cls";
	__asm {
		mov eax, dword ptr ds : [szClearScreen]
		push eax
		call system
		add esp, 4
	}
}

void gotoxy(int pos) {
	__asm {
		mov eax, dword ptr ds : [pos]
		push eax
		push - 11
		call dword ptr ds : [GetStdHandle]
		push eax
		call dword ptr ds : [SetConsoleCursorPosition]
	}
}

void ShowMainMenu() {
	int nSelect;
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

		// 打印分割字符
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
		mov dword ptr ds:[g_nIsDead], 0		// 游戏开始， 0 表示未死亡
		call InitGameMapData						// 初始化地图数据
		call RandSetFoodPosition					// 随机设置食物位置
		call RandSetSnakePosition				// 随机设置蛇的位置
	
	go_on_game:
		call DrawMap									// 画地图、食物
		call DrawSnake									// 画蛇
		call MoveSnake									// 移动蛇
		
		push 300
		call dword ptr ds:[Sleep]
		mov eax, dword ptr ds:[g_nIsDead]
		cmp eax, 0										// 判断有没有撞墙死亡，未死亡则重新绘制地图，死亡则回到主菜单
		je go_on_game

		jmp show_menu_begin						// 回到主菜单
	    nop

	dispatch_end_game:
		call EndGame
	    nop
	}
}

void GetUserInput(int nSelect) {
	const char *format1 = "请输入你要选择的编号:\n";
	const char* format2 = "%d";
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

void InitGameMapData() {
	const char* format1 = "内存已经清理完毕, 即将开始游戏......\n";
	int i;
	__asm {
		// 清零 g_MapDataArr
		push 400
		push 0
		lea eax, dword ptr ds:[g_MapDataArr]
		push eax
		call memset
		add esp, 12

		// 使用循环设置地图边界 0xB
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

void RandSetFoodPosition() {
	const char* format1 = "正在随机生成食物的位置......\n";
	int x;
	int y;
	__asm {
		// 打印提示信息
		mov eax, dword ptr ds : [format1]
		push eax
		call printf
		add esp, 4

	rand_set_food_pos :
		// 获取时间	
		push 0
		call time
		add esp, 4

		// 设置随机数种子
		push eax
		call srand
		add esp, 4

		// 获取 x 坐标值
		call rand
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds : [x] , edx

		// 获取 y 坐标值
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
		cmp cl, 0xB
		je rand_set_food_pos				// 如果和墙重叠, 则回到开始位置, 重新生成随机数
		mov byte ptr ds : [eax] , 0xC		// 如果没有墙, 则设置食物
	}
}

void RandSetSnakePosition() {
	int x;
	int y;
	int flag;
	__asm {
		// 清零 g_sSnakePosArray
		push 800
		push 0
		lea eax, dword ptr ds : [g_sSnakePosArray]
		push eax
		call memset
		add esp, 12

		// 初始化长度和 flag 标志
		mov dword ptr ds : [g_nSnakeLength] , 0;
		mov dword ptr ds : [flag] , 0

	set_snake_pos_begin :
		// 取当前时间
		push 0
		call time
		add esp, 4
		
		// 设置随机数种子
		add eax, 23								// 为了避免和食物的位置重叠，给随机数加上一个固定值
		push eax
		call srand
		add esp, 4

		// 取 x 的随机数坐标
		call rand
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds : [x] , edx
		
		// 取 y 的随机数坐标
		call rand
		cdq
		mov ecx, 20
		idiv ecx
		mov dword ptr ds : [y] , edx
		
	// 判断是否和墙壁重叠
	set_snake_judge_iswall :

		lea eax, dword ptr ds : [g_MapDataArr]
		mov ecx, dword ptr ds : [x]
		imul ecx, ecx, 20
		add eax, ecx
		mov ecx, dword ptr ds : [y]
		add eax, ecx

		mov al, byte ptr ds : [eax]
		mov ecx, dword ptr ds : [flag]	// 判断是否已经确定蛇头位置
		cmp ecx, 1
		je set_snake_pos_second_judge
		jne set_snake_pos_first_judge

	set_snake_pos_second_judge :

		cmp al, 0xB							// 是否已经撞墙
		je set_snake_pos_end
		jne set_snake_pos_write

	set_snake_pos_first_judge :

		cmp al, 0xb
		je set_snake_pos_begin
		mov dword ptr ds : [flag] , 1	// 证明确定蛇头位置

		// 写入到位置数组
		set_snake_pos_write :
		lea eax, dword ptr ds : [g_sSnakePosArray]
		mov ecx, dword ptr ds : [g_nSnakeLength]
		imul ecx, ecx, 8
		add eax, ecx

		mov ecx, dword ptr ds : [x]	// 写入 x
		mov dword ptr ds : [eax] , ecx
		mov ecx, dword ptr ds : [y]
		mov dword ptr ds : [eax + 4] , ecx

		// 判断是否继续循环
		mov eax, dword ptr ds : [g_nSnakeLength]
		inc eax
		mov dword ptr ds : [g_nSnakeLength] , eax
		cmp eax, 4
		jg set_snake_pos_end
		mov eax, dword ptr ds : [y]
		inc eax
		mov dword ptr ds : [y] , eax
		jmp set_snake_judge_iswall

	set_snake_pos_end :
		nop
	}
}

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

		// 打印墙壁
	print_map_draw_wall:
		mov eax, dword ptr ds:[szWall]
		push eax
		mov ecx, dword ptr ds:[format1]
		push ecx
		call printf
		add esp, 8
		jmp print_second_for_inc

		// 打印食物
	print_map_draw_food:
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

void DrawSnake() {
	int i;
	const char *format = "M";
	__asm {
		mov dword ptr ds:[i], 0
		jmp print_snake_cmp

	print_snake_inc:
		mov eax, dword ptr ds:[i]
		inc eax
		mov dword ptr ds:[i], eax
	print_snake_cmp:
		mov eax, dword ptr ds:[i]
		mov ecx, dword ptr ds:[g_nSnakeLength]
		cmp eax, ecx													// 如果画好的长度等于储存的贪吃蛇的长度, 则画图结束
		jge print_snake_end

		// 执行代码
		lea eax, dword ptr ds:[g_sSnakePosArray]		// 计算地址
		mov ecx, dword ptr ds:[i]
		imul ecx, ecx, 8
		add eax, ecx

		// 得到蛇的坐标, 拼接后调用 gotoxy
		mov ecx, dword ptr ds:[eax]							// 取出 x
		shl ecx, 16													// 左移 16 位
		mov edx, dword ptr ds:[eax + 4]					// 取出 y
		or ecx, edx													// 拼接完成
		push ecx														// 设置光标位置
		call gotoxy
		add esp, 4

		// 打印蛇的身体
		mov eax, dword ptr ds:[format]
		push eax
		call printf
		add esp, 4
		jmp print_snake_inc

	print_snake_end:
		nop
	}
}

void JudgeMoveFlag() {
	int flag;
	while (true) {
		__asm {
	back_while:
		// 获取 w 键
		push 87
		call dword ptr ds : [GetAsyncKeyState]
		and ax, 0xff00												// 与操作，获取第 15 位的值
		cmp ax, 0														// 如果为 0 表示没有被按下
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
		jmp back_while
		
	// 如果 w 键被按下
	w_press :
		mov eax, dword ptr ds : [g_nMoveFlag]
		cmp eax, 2												// 当前移动方向是否向下
		je w_back
		mov dword ptr ds : [g_nMoveFlag] , 1
	w_back :
		jmp back_while

	// 如果 s 键被按下
	s_press :
		mov eax, dword ptr ds : [g_nMoveFlag]
		cmp eax, 1										// 当前移动方向是否向上
		je s_back
		mov dword ptr ds : [g_nMoveFlag] , 2
	s_back :
		jmp back_while
		
	// 如果 a 键被按下
	a_press :
		mov eax, dword ptr ds : [g_nMoveFlag]
		cmp eax, 4										// 当前移动方向是否向右
		je a_back
		mov dword ptr ds : [g_nMoveFlag] , 3
	a_back :
		jmp back_while
		
	// 如果 d 键被按下
	d_press :
		mov eax, dword ptr ds : [g_nMoveFlag]
		cmp eax, 3									// 当前移动方向是否向左
		je d_back
		mov dword ptr ds : [g_nMoveFlag] , 4
	d_back :
		jmp back_while
		}
	}
}

void MoveSnake() {
	int i;
	__asm {
		// 给循环变量赋值
		mov eax, dword ptr ds:[g_nSnakeLength]
		sub eax, 2
		mov dword ptr ds:[i], eax
		jmp snake_move_cmp

	snake_move_dec:
		mov eax, dword ptr ds:[i]
		dec eax
		mov dword ptr ds:[i], eax

	snake_move_cmp:
		mov eax, dword ptr ds:[i]
		cmp eax, 0
		jl snake_move_end
		
		lea eax, dword ptr ds:[g_sSnakePosArray]
		mov ecx, dword ptr ds:[i]
		imul ecx, ecx, 8
		add eax, ecx
		mov ecx, dword ptr ds:[eax]		// x
		mov edx, dword ptr ds:[eax + 4]	// y

		// 放到 i + 1 的下标中
		add eax, 8
		mov dword ptr ds:[eax], ecx
		mov dword ptr ds:[eax + 4], edx
		jmp snake_move_dec

	snake_move_end:
		
		// 确定蛇头位置
		mov eax, dword ptr ds:[g_nMoveFlag]
		cmp eax, 1
		je move_up
		cmp eax, 2
		je move_down
		cmp eax, 3
		je move_left
		cmp eax, 4
		je move_right

	// 向上移动
	move_up:
		lea eax, dword ptr ds:[g_sSnakePosArray]
		mov ecx, dword ptr ds:[eax]		// x
		mov edx, dword ptr ds:[eax + 4]	// y
		dec ecx
		mov dword ptr ds:[eax], ecx
		mov dword ptr ds:[eax + 4], edx
		jmp 	snake_move_fun_end
	// 向下移动
	move_down:
		lea eax, dword ptr ds : [g_sSnakePosArray]
		mov ecx, dword ptr ds : [eax]		// x
		mov edx, dword ptr ds : [eax + 4]	// y
		inc ecx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	snake_move_fun_end
	// 向左移动
	move_left:
		lea eax, dword ptr ds : [g_sSnakePosArray]
		mov ecx, dword ptr ds : [eax]		// x
		mov edx, dword ptr ds : [eax + 4]	// y
		dec edx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	snake_move_fun_end
	// 向右移动
	move_right:
		lea eax, dword ptr ds : [g_sSnakePosArray]
		mov ecx, dword ptr ds : [eax]		// x
		mov edx, dword ptr ds : [eax + 4]	// y
		inc edx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	snake_move_fun_end

	snake_move_fun_end :
		nop
	}
}

void JudgeMoveState() {

}

void Death() {

}

void GetScore() {

}

void AddSnakeLength() {
	
}

void main() {

	__asm {
		push 0
		push 0
		push 0
		lea eax, dword ptr ds:[JudgeMoveFlag]
		push eax
		push 0
		push 0
		call dword ptr ds:[CreateThread]
	}
	__asm {
		call ShowMainMenu
	}
}
