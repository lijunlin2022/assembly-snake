#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

char globalMapArr[25][25];

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

int globalInitialSnakeHeadX;
int globalInitialSnakeHeadY;

int globalFoodX;
int globalFoodY;

void enterGame();
void showMainMenu();
void handleIllegalSelection();
void startGame();
void endGame();
void initMapData();
void setWall();
void setFoodPosition();
void generateRandomFood();
void setSnakePosition();
void generateRandomSnakeHead();
void drawMap();
void drawSnake();
void moveSnake();
void judgeMovementDirection();
void judgeDetection();
void die();
void addSnakeLen();

void clearScreenUtil();
void gotoxyUtil(int pos);

void main() {
	__asm {
		// 第一个线程
		// 获取键盘输入
		// 操控蛇的移动
		push 0
		push 0
		push 0
		lea eax, dword ptr ds : [judgeMovementDirection]
		push eax
		push 0
		push 0
		call CreateThread

		// 第二个线程
		// 打印游戏画面
		call enterGame
	}
}

void enterGame() {
	int selection;
	const char* paramater = "%d";
	__asm {
	enter_game:
		call showMainMenu

		// 获取用户选择
		lea eax, dword ptr ds : [selection]
		push eax
		mov ecx, dword ptr ds : [paramater]
		push ecx
		call scanf
		add esp, 8

		// 判断用户的选择
		mov eax, dword ptr ds : [selection]
		cmp eax, 1
		je start_game
		cmp eax, 2
		je end_game

		// 处理非法选择
		call handleIllegalSelection
		jmp enter_game

	start_game :
		call startGame

	end_game :
		call endGame
	}
}

void showMainMenu() {
	const char* dividingLine = "-------------------------------------------------------------------\n";
	const char* authorInfo = "作者: 李俊霖\n";
	const char* operationGuide = "按 1 开始游戏\n按 2 结束游戏\n按 W 向上移动\n按 S 向下移动\n按 A 向左移动\n按 D 向右移动\n";
	__asm {
		// 打印分割字符
		mov eax, dword ptr ds : [dividingLine]
		push eax
		call printf
		add esp, 4

		// 打印作者信息
		mov eax, dword ptr ds : [authorInfo]
		push eax
		call printf
		add esp, 4

		// 打印提示信息
		mov eax, dword ptr ds : [operationGuide]
		push eax
		call printf
		add esp, 4

		// 打印分割字符
		mov eax, dword ptr ds : [dividingLine]
		push eax
		call printf
		add esp, 4
	}
}

void handleIllegalSelection() {
	const char* errMsg = "输入的编号不正确，请在 2s 之后重新输入\n";
	__asm {
		mov eax, dword ptr ds : [errMsg]
		push eax
		call printf
		add esp, 4

		// 延时函数
		push 2000
		call dword ptr ds : [Sleep]

		// 清屏
		call clearScreenUtil
	}
}

void startGame() {
	__asm {
		call initMapData
		call setWall
		call setFoodPosition
		call setSnakePosition

	go_on_game :
		call drawMap
		call drawSnake
		call moveSnake
		call judgeDetection

		push 250
		call Sleep
		jmp go_on_game
	}
}

void endGame() {
	const char* endGameTip = "程序在两秒钟之后即将退出......";
	__asm {
		// 打印提示信息
		mov eax, dword ptr ds : [endGameTip]
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

void initMapData() {
	__asm {
		// 清零 g_MapDataArr
		push 625
		push 0
		lea eax, dword ptr ds : [globalMapArr]
		push eax
		call memset
		add esp, 12
	}
}

void setWall() {
	int i;
	__asm {
		mov dword ptr ds : [i] , 0
		mov ecx, 25;
	set_wall:
		// 顶墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, dword ptr ds : [i]
		mov byte ptr ds : [eax + ebx] , 0bh
		// 底墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, 24
		imul ebx, ebx, 25
		add eax, ebx
		mov ebx, dword ptr ds : [i]
		mov byte ptr ds : [eax + ebx] , 0bh
		// 左墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, dword ptr ds : [i]
		imul ebx, ebx, 25
		mov byte ptr ds : [eax + ebx] , 0bh
		// 右墙
		lea eax, dword ptr ds : [globalMapArr]
		mov ebx, dword ptr ds : [i]
		imul ebx, ebx, 25
		add ebx, 24
		mov byte ptr ds : [eax + ebx] , 0bh
		// i 自减
		mov ebx, dword ptr ds : [i]
		inc ebx
		mov dword ptr ds : [i] , ebx
		loop set_wall
	}
}

void setFoodPosition() {
	__asm {
	set_food_pos:
		call generateRandomFood

		// 判断食物是不是和墙壁重叠
		lea eax, dword ptr ds : [globalMapArr]
		mov ecx, dword ptr ds : [globalFoodX]
		imul ecx, ecx, 25
		add eax, ecx
		mov edx, dword ptr ds : [globalFoodY]
		add eax, edx

		mov cl, byte ptr ds : [eax]
		cmp cl, 0xB

		// 如果和墙重叠, 则回到开始位置, 重新生成随机数
		je set_food_pos

		// 如果没有和墙壁重叠, 在在该位置设置食物
		mov byte ptr ds : [eax] , 0ch
	}
}

void generateRandomFood() {
	__asm {
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
		mov ecx, 25
		idiv ecx
		mov dword ptr ds : [globalFoodX] , edx
		// 获取 y 坐标值
		call rand
		cdq
		mov ecx, 25
		idiv ecx
		mov dword ptr ds : [globalFoodY] , edx
	}
}

void setSnakePosition() {
	__asm {
	set_snake_pos:
		call generateRandomSnakeHead

		lea eax, dword ptr ds : [globalMapArr]
		mov ecx, dword ptr ds : [globalInitialSnakeHeadX]
		imul ecx, ecx, 25
		add eax, ecx
		mov edx, dword ptr ds : [globalInitialSnakeHeadY]
		add eax, edx

		mov cl, byte ptr ds : [eax]
		cmp cl, 0xB
		// 如果和墙重叠, 则回到开始位置, 重新生成蛇头
		je set_snake_pos

		// 生成的蛇头满足要求, 则写入蛇的结构体
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [globalSnakeLen]
		imul ecx, ecx, 8
		add eax, ecx

		mov ecx, dword ptr ds : [globalInitialSnakeHeadX]
		mov dword ptr ds : [eax] , ecx

		mov ecx, dword ptr ds : [globalInitialSnakeHeadY]
		mov dword ptr ds : [eax + 4] , ecx

		// 设置蛇的长度为 1
		mov dword ptr ds : [globalSnakeLen] , 1
	}
}

void generateRandomSnakeHead() {
	__asm {
		// 取当前时间
		push 0
		call time
		add esp, 4
		// 设置随机数种子
		// 为了避免和食物的位置重叠，给蛇头的随机数加上一个固定值
		add eax, 5
		push eax
		call srand
		add esp, 4
		// 设置蛇头 x 的随机数坐标
		call rand
		cdq
		mov ecx, 25
		idiv ecx
		mov dword ptr ds : [globalInitialSnakeHeadX] , edx
		// 设置蛇头 y 的随机数坐标
		call rand
		cdq
		mov ecx, 25
		idiv ecx
		mov dword ptr ds : [globalInitialSnakeHeadY] , edx
	}
}

void drawMap() {
	int i;
	int j;
	const char* wall = "#";
	const char* food = "*";
	const char* nullCh = " ";
	const char* changeLine = "\n";

	__asm {
		// 清屏
		call clearScreenUtil

		// 双重循环打印 g_MapDataArr
		mov dword ptr ds : [i] , 0
		jmp first_cmp
		
	first_inc :
		mov eax, dword ptr ds : [i]
		inc eax
		mov dword ptr ds : [i] , eax
		
	first_cmp :
		mov eax, dword ptr ds : [i]
		cmp eax, 25
		jge first_end
		
		mov dword ptr ds : [j] , 0
		jmp second_cmp

	second_inc :
		mov eax, dword ptr ds : [j]
		inc eax
		mov dword ptr ds : [j] , eax
	second_cmp :
		mov eax, dword ptr ds : [j]
		cmp eax, 25
		jge second_end


		lea eax, dword ptr ds : [globalMapArr]
		mov ecx, dword ptr ds : [i]
		imul ecx, ecx, 25
		add eax, ecx
		mov ecx, dword ptr ds : [j]
		add eax, ecx

		mov al, byte ptr ds : [eax]
		cmp al, 0xB
		je draw_wall
		cmp al, 0xC
		je draw_food

		// 打印空格
		mov eax, dword ptr ds : [nullCh]
		push eax
		call printf
		add esp, 4
		jmp second_inc

		// 打印墙壁
	draw_wall :
		mov eax, dword ptr ds : [wall]
		push eax
		call printf
		add esp, 4
		jmp second_inc

		// 打印食物
	draw_food :
		mov eax, dword ptr ds : [food]
		push eax
		call printf
		add esp, 4
		jmp second_inc



	second_end:
		mov eax, dword ptr ds : [changeLine]
		push eax
		call printf
		add esp, 4
		jmp first_inc

	first_end :
		nop
	}
}

void drawSnake() {
	int i;
	const char* format = "O";
	__asm {
		mov dword ptr ds : [i] , 0
		jmp print_snake_cmp

	print_snake_inc :
		mov eax, dword ptr ds : [i]
		inc eax
		mov dword ptr ds : [i] , eax
	print_snake_cmp :
		mov eax, dword ptr ds : [i]
		mov ecx, dword ptr ds : [globalSnakeLen]
		// 如果画好的长度等于储存的贪吃蛇的长度, 则画图结束
		cmp eax, ecx	
		jge print_snake_end

		// 开始画图
		lea eax, dword ptr ds : [globalSnakeArr]	
		mov ecx, dword ptr ds : [i]
		imul ecx, ecx, 8
		add eax, ecx

		// 得到蛇的坐标, 拼接后调用 gotoxy
		mov ecx, dword ptr ds : [eax]
		// 左移 16 位, 用以将 x 和 y 两个 dword 的值拼接为 1 个 word
		shl ecx, 16													
		mov edx, dword ptr ds : [eax + 4]
		or ecx, edx
		push ecx
		call gotoxyUtil
		add esp, 4

		// 打印蛇的身体
		mov eax, dword ptr ds : [format]
		push eax
		call printf
		add esp, 4
		jmp print_snake_inc

	print_snake_end :
		nop
	}
}

void moveSnake() {
	int i;
	__asm {
		mov eax, dword ptr ds : [globalSnakeLen]
		sub eax, 2
		mov dword ptr ds : [i] , eax
		jmp snake_cmp

	snake_dec :
		mov eax, dword ptr ds : [i]
		dec eax
		mov dword ptr ds : [i] , eax

	snake_cmp :
		mov eax, dword ptr ds : [i]
		cmp eax, 0
		jl snake_end

		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [i]
		imul ecx, ecx, 8
		add eax, ecx
		mov ecx, dword ptr ds : [eax]		// x
		mov edx, dword ptr ds : [eax + 4]	// y

		// 放到 i + 1 的下标中
		add eax, 8
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp snake_dec

	snake_end :

		// 确定蛇头位置
		mov eax, dword ptr ds : [globalMovementDirection]
		cmp eax, 1
		je move_up
		cmp eax, 2
		je move_down
		cmp eax, 3
		je move_left
		cmp eax, 4
		je move_right

		// 向上移动
	move_up :
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]
		mov edx, dword ptr ds : [eax + 4]
		dec ecx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	fun_end
		// 向下移动
	move_down :
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]
		mov edx, dword ptr ds : [eax + 4]
		inc ecx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	fun_end
		// 向左移动
	move_left :
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]
		mov edx, dword ptr ds : [eax + 4]
		dec edx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	fun_end
		// 向右移动
	move_right :
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]
		mov edx, dword ptr ds : [eax + 4]
		inc edx
		mov dword ptr ds : [eax] , ecx
		mov dword ptr ds : [eax + 4] , edx
		jmp 	fun_end

	fun_end :
		nop
	}
}

void  judgeMovementDirection() {
	while (true) {
		__asm {
	back_while:
		// 获取 w 键
		push 87
		call GetAsyncKeyState
		// 与操作，获取第 15 位的值
		and ax, 0ff00h
		// 如果为 0 表示没有被按下
		cmp ax, 0
		jne w_press

		// 获取 s 键
		push 83
		call GetAsyncKeyState
		and ax, 0ff00h
		cmp ax, 0
		jne s_press

		// 获取 a 键
		push 65
		call GetAsyncKeyState
		and ax, 0ff00h
		cmp ax, 0
		jne a_press

		// 获取 d 键
		push 68
		call GetAsyncKeyState
		and ax, 0ff00h
		cmp ax, 0
		jne d_press
		jmp back_while

		// 如果 w 键被按下
	w_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		// 如果当前移动方向向下, 则忽略此次事件
		cmp eax, 2
		je w_back
		mov dword ptr ds : [globalMovementDirection] , 1
		w_back :
		jmp back_while

		// 如果 s 键被按下
	s_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		// 如果当前移动方向向上, 则忽略此次事件
		cmp eax, 1
		je s_back
		mov dword ptr ds : [globalMovementDirection] , 2
		s_back :
		jmp back_while

		// 如果 a 键被按下
	a_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		// 如果当前移动方向向右, 则忽略此次事件
		cmp eax, 4
		je a_back
		mov dword ptr ds : [globalMovementDirection] , 3
		a_back :
		jmp back_while

		// 如果 d 键被按下
	d_press :
		mov eax, dword ptr ds : [globalMovementDirection]
		// 如果当前移动方向向左, 则忽略此次事件
		cmp eax, 3
		je d_back
		mov dword ptr ds : [globalMovementDirection] , 4
		d_back :
		jmp back_while
		}
	}
}

void judgeDetection() {
	int x;
	int y;
	__asm {
		// 取出蛇头的 xy 坐标
		lea eax, dword ptr ds : [globalSnakeArr]
		mov ecx, dword ptr ds : [eax]
		mov edx, dword ptr ds : [eax + 4]
		mov dword ptr ds : [x] , ecx
		mov dword ptr ds : [y] , edx

		// 是否撞墙
		lea eax, dword ptr ds : [globalMapArr]
		imul ecx, ecx, 25
		add eax, ecx
		add eax, edx
		// 从二维数组里面取指定下标值
		mov cl, byte ptr ds : [eax]

		// 判断是否撞墙
		cmp cl, 0bh
		je snake_dead


		// 判断是否吃到食物
		cmp cl, 0ch
		je snake_add_len

		// 啥都没做
		jmp fun_end

	snake_dead :
		call die

	snake_add_len :

		call addSnakeLen
		call initMapData
		call setWall
		call setFoodPosition

	fun_end :
		nop
	}
}

void die() {
	const char* dieTip = "你死了，游戏结束\n";
	__asm {
		call clearScreenUtil
		mov eax, dword ptr ds : [dieTip]
		push eax
		call printf
		add eax, 4

		call endGame
	}
}

void addSnakeLen() {
	__asm {
		mov eax, dword ptr ds : [globalSnakeLen]
		inc eax
		mov dword ptr ds : [globalSnakeLen] , eax
	}
}

void clearScreenUtil() {
	const char* clearScreen = "cls";
	__asm {
		mov eax, dword ptr ds : [clearScreen]
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
		call GetStdHandle
		push eax

		call SetConsoleCursorPosition
	}
}
