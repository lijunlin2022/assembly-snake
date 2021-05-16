#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

char globalMapArr[20][20];

struct SnakePos {
	int x;
	int y;
} globalSnakeArr[100];

int globalSnakeLen;

// 蛇的移动方向
// 1 代表向上移动
// 2 代表向下移动
// 3 代表向左移动
// 4 代表向右移动
int globalMovementDirection;

// 死亡标记
// 0 代表未死亡
// 1 代表死亡
int globalIsDead;

// 工具函数
// 清理屏幕
void clearScreenUtil();

// 工具函数
// 将光标移动到指定坐标处
void gotoxyUtil(int pos);

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
// 碰撞检测
void JudgeDetection();
// 增加蛇的长度
void AddSnakeLength();
// 死亡
void Death();

void clearScreenUtil() {
	const char* szClearScreen = "cls";
	__asm {
		mov eax, dword ptr ds : [szClearScreen]
		push eax
		call system
		add esp, 4
	}
}

void gotoxyUtil(int pos) {
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
		call clearScreenUtil
		jmp show_menu_begin
	
	dispatch_begin_game:
		mov dword ptr ds:[globalIsDead], 0		// 游戏开始， 0 表示未死亡
		call InitGameMapData						// 初始化地图数据
		call RandSetFoodPosition					// 随机设置食物位置
		call RandSetSnakePosition				// 随机设置蛇的位置
	
	go_on_game:
		call DrawMap									// 画地图、食物
		call DrawSnake									// 画蛇
		call MoveSnake									// 移动蛇
		call JudgeDetection							// 碰撞检测

		push 300
		call dword ptr ds:[Sleep]
		mov eax, dword ptr ds:[globalIsDead]
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
		lea eax, dword ptr ds:[globalMapArr]
		push eax
		call memset
		add esp, 12

		// 使用循环设置地图边界 0xB
	    mov dword ptr ds : [i] , 0
		mov ecx, 21;

	set_wall:
		// 顶墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, dword ptr ds : [i]
		mov byte ptr ds : [eax + ebx] , 0xB

		// 底墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, 19
		imul ebx, ebx, 20
		add eax, ebx
		mov ebx, dword ptr ds : [i]
		mov byte ptr ds : [eax + ebx] , 0xB

		// 左墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, dword ptr ds : [i]
		imul ebx, ebx, 20
		mov byte ptr ds : [eax + ebx] , 0xB

		// 右墙
		lea eax, dword ptr ds:[globalMapArr]
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
		lea eax, dword ptr ds : [globalMapArr]
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
		lea eax, dword ptr ds : [globalSnakeArr]
		push eax
		call memset
		add esp, 12

		// 初始化长度和 flag 标志
		mov dword ptr ds : [globalSnakeLen] , 0;
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

		lea eax, dword ptr ds : [globalMapArr]
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
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [globalSnakeLen]
		imul ecx, ecx, 8
		add eax, ecx

		mov ecx, dword ptr ds : [x]	// 写入 x
		mov dword ptr ds : [eax] , ecx
		mov ecx, dword ptr ds : [y]
		mov dword ptr ds : [eax + 4] , ecx

		// 判断是否继续循环
		mov eax, dword ptr ds : [globalSnakeLen]
		inc eax
		mov dword ptr ds : [globalSnakeLen] , eax
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
		call clearScreenUtil

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

		lea eax, dword ptr ds:[globalMapArr]
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
		mov ecx, dword ptr ds:[globalSnakeLen]
		cmp eax, ecx													// 如果画好的长度等于储存的贪吃蛇的长度, 则画图结束
		jge print_snake_end

		// 执行代码
		lea eax, dword ptr ds:[globalSnakeArr]		// 计算地址
		mov ecx, dword ptr ds:[i]
		imul ecx, ecx, 8
		add eax, ecx

		// 得到蛇的坐标, 拼接后调用 gotoxy
		mov ecx, dword ptr ds:[eax]							// 取出 x
		shl ecx, 16													// 左移 16 位
		mov edx, dword ptr ds:[eax + 4]					// 取出 y
		or ecx, edx													// 拼接完成
		push ecx														// 设置光标位置
		call gotoxyUtil
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
		mov eax, dword ptr ds : [globalMovementDirection]
		cmp eax, 2												// 当前移动方向是否向下
		je w_back
		mov dword ptr ds : [globalMovementDirection] , 1
	w_back :
		jmp back_while

	// 如果 s 键被按下
	s_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		cmp eax, 1										// 当前移动方向是否向上
		je s_back
		mov dword ptr ds : [globalMovementDirection] , 2
	s_back :
		jmp back_while
		
	// 如果 a 键被按下
	a_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		cmp eax, 4										// 当前移动方向是否向右
		je a_back
		mov dword ptr ds : [globalMovementDirection] , 3
	a_back :
		jmp back_while
		
	// 如果 d 键被按下
	d_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		cmp eax, 3									// 当前移动方向是否向左
		je d_back
		mov dword ptr ds : [globalMovementDirection] , 4
	d_back :
		jmp back_while
		}
	}
}

void MoveSnake() {
	int i;
	__asm {
		// 给循环变量赋值
		mov eax, dword ptr ds:[globalSnakeLen]
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
		
		lea eax, dword ptr ds:[globalSnakeArr]
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
		mov eax, dword ptr ds:[globalMovementDirection]
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
		lea eax, dword ptr ds:[globalSnakeArr]
		mov ecx, dword ptr ds:[eax]		// x
		mov edx, dword ptr ds:[eax + 4]	// y
		dec ecx
		mov dword ptr ds:[eax], ecx
		mov dword ptr ds:[eax + 4], edx
		jmp 	snake_move_fun_end
	// 向下移动
	move_down:
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]		// x
		mov edx, dword ptr ds : [eax + 4]	// y
		inc ecx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	snake_move_fun_end
	// 向左移动
	move_left:
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]		// x
		mov edx, dword ptr ds : [eax + 4]	// y
		dec edx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	snake_move_fun_end
	// 向右移动
	move_right:
		lea eax, dword ptr ds : [globalSnakeArr]
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

void JudgeDetection() {
	int x;
	int y;
	__asm {
		// 取出蛇头的 xy 坐标
		lea eax, dword ptr ds:[globalSnakeArr]
		mov ecx, dword ptr ds:[eax]	// x 的坐标
		mov edx, dword ptr ds:[eax + 4]	// y 的坐标
		mov dword ptr ds:[x], ecx
		mov dword ptr ds:[y], edx

		// 是否撞墙
		lea eax, dword ptr ds:[globalMapArr]
		imul ecx, ecx, 20
		add eax, ecx
		add eax, edx
		mov cl, byte ptr ds:[eax]	// 从二维数组里面取指定下标值
		cmp cl, 0xB	// 判断是否撞墙
		je snake_dead

		// 是否得分
		cmp cl, 0xC	// 判断是否吃到食物
		je snake_get_score

		// 啥都没做
		jmp snake_get_score_end

	snake_dead:
		call Death

	snake_get_score:
		mov eax, dword ptr ds:[globalSnakeLen]
		inc eax
		mov dword ptr ds:[globalSnakeLen], eax
		call AddSnakeLength

	snake_get_score_end:
		nop
	}
}

void Death() {
	const char *format1 = "你死了，游戏结束";
	__asm {
		call clearScreenUtil
		mov eax, dword ptr ds:[format1]
		push eax
		call printf
		add eax, 4
		call EndGame
	}
}

void GetScore() {

}

void AddSnakeLength() {
	__asm {
		lea eax, dword ptr ds:[globalSnakeArr]
		mov ecx, dword ptr ds:[globalSnakeLen]
		dec ecx
		imul ecx, ecx, 8
		add eax, ecx
		mov ecx, dword ptr ds:[eax]	// 取蛇尾的 x
		mov edx, dword ptr ds:[eax + 4]	// 取蛇尾的 y
		// 进行判断
		push ebx
		mov ebx, dword ptr ds:[globalMovementDirection]
		cmp ebx, 1
		je move_up
		cmp ebx, 2
		je move_down
		cmp ebx, 3
		je move_left
		cmp ebx, 4
		je move_right
	// 向上移动
	move_up:
		inc ecx
		add eax, 8
		mov dword ptr ds:[eax], ecx
		mov dword ptr ds:[eax + 4], edx
		jmp snake_add_length_end
	// 向下移动
	move_down:
		dec ecx
		add eax, 8
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp snake_add_length_end
	// 向左移动
	move_left:
		inc edx
		add eax, 8
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp snake_add_length_end
	// 向右移动
	move_right:
		dec edx
		add eax, 8
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp snake_add_length_end
	snake_add_length_end:
		pop ebx
		nop
	}
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
