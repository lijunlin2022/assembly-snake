TITLE Program Snake

.386
.model flat, stdcall
option casemap:none

ExitProcess proto, dwExitCode:dword
Sleep proto, timeSpan:dword
GetStdHandle proto, :dword
SetConsoleCursorPosition proto hd:dword, pos:dword

includelib msvcrt.lib
includelib user32.lib

system proto C, :ptr sbyte, :vararg
printf proto C :ptr sbyte, :vararg
scanf proto C :ptr sbyte, :vararg
memset proto C :ptr sbyte, char:dword, len:dword
time proto C :dword
srand proto C :dword
rand proto C :dword
;	##################################################################################

.data

; 开辟 625 个字节的数组来设置地图
globalMapArr byte 625 dup(?)

; 定义结构体来存储蛇
snakePos STRUCT
	x dword ?
	y dword ?
snakePos ENDS

globalSnakeArr snakePos 100 dup(<0, 0>)

; 蛇的长度
globalSnakeLen dword ?

; 存储初始蛇头的位置
globalInitialSnakeHeadX dword ?
globalInitialSnakeHeadY dword ?

; 存储食物的位置
globalFoodX dword ?
globalFoodY dword ?

snakeBody byte "O", 0

;	showMainMenu()
;	需要打印的分割线、作者信息、操作指南
dividingLine byte "-------------------------------------------------------------------", 0ah, 0
authorInfo byte "author: LiJunLin", 0ah, 0
operationGuide byte "按 1 开始游戏", 0ah, "按 2 结束游戏", 0ah, "按 W 向上移动",  0ah, "按 S 向下移动", 0ah, "按 A 向左移动", 0ah, "按 D 向右移动", 0ah, 0

;	setWall()
;	循环设置墙壁时所用的变量
i dword ?
j dword ?
wall byte "#", 0
nullCh byte " ", 0
food byte "*", 0
changeLine byte 0ah, 0

;	handleIllegalSelection()
;	处理非法输入的提示信息
errMsg byte "输入的编号不正确，请在 2s 之后重新输入", 0ah, 0

;	enterGame()
;	scanf 读取用户输入时需要的参数 %d
paramater byte "%d", 0
selection dword ?

;	endGame()
;	退出游戏的提示信息
endGameTip byte "程序在两秒钟之后即将退出......", 0ah, 0


;	clearScreenUtil()
;	system 清理屏幕需要用到的参数 cls
clearScreen byte "cls", 0

;	##################################################################################
.code

;	----------------------------------------------------------------------------------------------------------------------------------------
enterGame proc
enter_game:
	call showMainMenu
	
	; 获取用户选择
	lea eax, dword ptr ds:[selection]
	push eax
	mov ecx, dword ptr offset paramater
	push ecx
	call scanf
	add esp, 8

	; 判断用户的选择
	mov eax, dword ptr ds:[selection]
	cmp eax, 1
	je start_game
	cmp eax, 2
	je end_game

	; 处理非法输入
	call handleIllegalSelection
	jmp enter_game

start_game:
	call startGame
end_game:
	call endGame
	ret
enterGame endp

;	----------------------------------------------------------------------------------------------------------------------------------------
startGame proc
	call initMapData
	call setWall
	call setFoodPosition
    call setSnakePosition

go_on_game:
	call drawMap
	call drawSnake

	push 250
	call Sleep
	jmp go_on_game
	ret
startGame endp

;	----------------------------------------------------------------------------------------------------------------------------------------
showMainMenu proc
	; 打印分割字符
	invoke printf, dword ptr offset dividingLine
	; 打印作者信息
	invoke printf, dword ptr offset authorInfo
	; 打印提示信息
	invoke printf, dword ptr offset operationGuide
	; 打印分割字符
	invoke printf, dword ptr offset dividingLine

	ret
showMainMenu endp


;	----------------------------------------------------------------------------------------------------------------------------------------
initMapData proc
	push 625
	push 0
	lea eax, dword ptr offset[globalMapArr]
	push eax
	call memset
	add esp, 12
	ret
initMapData endp


;	----------------------------------------------------------------------------------------------------------------------------------------
setWall proc
	mov dword ptr ds:[i], 0
	mov ecx, 25
set_wall:
	; 顶墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, dword ptr ds:[i]
	mov byte ptr ds:[eax + ebx], 0bh
	; 底墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, 24
	imul ebx, ebx, 25
	add eax, ebx
	mov ebx, dword ptr ds:[i]
	mov byte ptr ds : [eax + ebx] , 0bh
	; 左墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, dword ptr ds:[i]
	imul ebx, ebx, 25
	mov byte ptr ds : [eax + ebx] , 0bh
	; 右墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, dword ptr ds:[i]
	imul ebx, ebx, 25
	add ebx, 24
	mov byte ptr ds : [eax + ebx] , 0bh
	; i 自减
	mov ebx, dword ptr ds:[i]
	inc ebx
	mov dword ptr  ds:[i] , ebx
	loop set_wall
	ret
setWall endp



;	----------------------------------------------------------------------------------------------------------------------------------------
setFoodPosition proc
set_food_pos:
	call generateRandomFood

	; 判断食物是不是和墙壁重叠
	lea eax, dword ptr ds : [globalMapArr]
	mov ecx, dword ptr ds : [globalFoodX]
	imul ecx, ecx, 25
	add eax, ecx
	mov edx, dword ptr ds : [globalFoodY]
	add eax, edx

	mov cl, byte ptr ds : [eax]
	cmp cl, 0bh

	; 如果和墙重叠, 则回到开始位置, 重新生成随机数
	je set_food_pos
	; 如果没有墙, 则设置食物
	mov byte ptr ds : [eax] , 0ch

	ret
setFoodPosition endp



;	----------------------------------------------------------------------------------------------------------------------------------------
generateRandomFood proc
	; 获取时间
	push 0
	call time
	add esp, 4
	; 设置随机数种子
	push eax
	call srand
	add esp, 4
	; 获取 x 坐标值
	call rand
	cdq
	mov ecx, 25
	idiv ecx
	mov dword ptr ds : [globalFoodX] , edx
	; 获取 y 坐标值
	call rand
	cdq
	mov ecx, 25
	idiv ecx
	mov dword ptr ds : [globalFoodY] , edx

	ret
generateRandomFood endp



;	----------------------------------------------------------------------------------------------------------------------------------------
setSnakePosition proc
set_snake_pos:
	call generateRandomSnakeHead

	lea eax, dword ptr ds : [globalMapArr]
	mov ecx, dword ptr ds : [globalInitialSnakeHeadX]
	imul ecx, ecx, 25
	add eax, ecx
	mov edx, dword ptr ds : [globalInitialSnakeHeadY]
	add eax, edx
		
	mov cl, byte ptr ds : [eax]
	cmp cl, 0bh
	;	如果和墙重叠, 则回到开始位置, 重新生成蛇头
	je set_snake_pos

	;	生成的蛇头满足要求, 则写入蛇的结构体
	lea eax, dword ptr ds : [globalSnakeArr]
	mov ecx, dword ptr ds : [globalSnakeLen]
	imul ecx, ecx, 8
	add eax, ecx

	;	将 蛇头的 x 坐标写入结构体
	mov ecx, dword ptr ds : [globalInitialSnakeHeadX] 
	mov dword ptr ds : [eax], ecx
		
	;	将蛇头的 y 坐标写入结构体
	mov ecx, dword ptr ds : [globalInitialSnakeHeadY] 
	mov dword ptr ds : [eax + 4], ecx

	;	设置蛇的长度为 1
	mov dword ptr ds : [globalSnakeLen] , 1

	ret
setSnakePosition endp



;	----------------------------------------------------------------------------------------------------------------------------------------
generateRandomSnakeHead proc
	;	取当前时间
	push 0
	call time
	add esp, 4
	;	设置随机数种子
	;	为了避免和食物的位置重叠，给随机数加上一个固定值
	add eax, 23
	push eax
	call srand
	add esp, 4
	;	取 x 的随机数坐标
	call rand
	cdq
	mov ecx, 25
	idiv ecx
	mov dword ptr ds : [globalInitialSnakeHeadX] , edx
	;	取 y 的随机数坐标
	call rand
	cdq
	mov ecx, 25
	idiv ecx
	mov dword ptr ds : [globalInitialSnakeHeadY] , edx

	ret
generateRandomSnakeHead endp




;	----------------------------------------------------------------------------------------------------------------------------------------
handleIllegalSelection proc
	; 打印错误信息
	invoke printf, dword ptr offset errMsg
	
	; 延时函数
	push 2000
	call dword ptr offset Sleep
	call clearScreenUtil
	ret
handleIllegalSelection endp


;	----------------------------------------------------------------------------------------------------------------------------------------
drawMap proc
	call clearScreenUtil

	mov dword ptr ds:[i], 0
	jmp first_cmp
	; 第一个循环开始
first_inc:
	mov eax, dword ptr ds:[i]
	inc eax
	mov dword ptr ds:[i], eax
first_cmp:	
	mov eax, dword ptr ds:[i]
	cmp eax, 25
	jge first_end

	; 第二个循环开始
	mov dword ptr ds:[j], 0
	jmp second_cmp

second_inc:						
	mov eax, dword ptr ds:[j]
	inc eax
	mov dword ptr ds:[j], eax

second_cmp:					
	mov eax, dword ptr ds:[j]
	cmp eax, 25
	jge second_end

	;	---------------------------------------------------
	lea eax, dword ptr ds:[globalMapArr]
	mov ecx, dword ptr ds:[i]
	imul ecx, ecx, 25
	add eax, ecx
	mov ecx, dword ptr ds:[j]
	add eax, ecx

	mov al, byte ptr ds:[eax]
	cmp al, 0bh
	je draw_wall
	cmp al, 0ch
	je draw_food

	; 打印空格
	invoke printf, dword ptr offset nullCh
	jmp second_inc

	; 打印墙壁
draw_wall:
	invoke printf, dword ptr offset wall
	jmp second_inc

	; 打印食物
draw_food:
	invoke printf, dword ptr offset food
	jmp second_inc
	;	---------------------------------------------------

second_end:
	mov eax, dword ptr offset changeLine
	push eax
	call printf
	add esp, 4
	jmp first_inc

first_end:
	nop

	ret
drawMap endp




;	----------------------------------------------------------------------------------------------------------------------------------------
drawSnake proc
    mov dword ptr ds:[i], 0
	jmp print_snake_cmp

print_snake_inc:
	mov eax, dword ptr ds:[i]
	inc eax
	mov dword ptr ds:[i], eax
print_snake_cmp:
	mov eax, dword ptr ds:[i]
	mov ecx, dword ptr ds:[globalSnakeLen]
	; 如果画好的长度等于储存的贪吃蛇的长度, 则画图结束
	cmp eax, ecx	
	jge print_snake_end

	; 执行代码
	lea eax, dword ptr ds:[globalSnakeArr]
	mov ecx, dword ptr ds:[i]
	imul ecx, ecx, 8
	add eax, ecx

	; 得到蛇的坐标, 拼接后调用 gotoxy
	mov ecx, dword ptr ds:[eax]							; 取出 x
	shl ecx, 16													; 左移 16 位
	mov edx, dword ptr ds:[eax + 4]					; 取出 y
	or ecx, edx													; 拼接完成
	push ecx														; 设置光标位置
	call gotoxyUtil
	add esp, 4

	; 打印蛇的身体
	invoke printf, dword ptr offset snakeBody
	jmp print_snake_inc

print_snake_end:
	nop
	ret

	
drawSnake endp
;	----------------------------------------------------------------------------------------------------------------------------------------
endGame proc
	; 打印提示信息
	invoke printf, dword ptr offset endGameTip

	; 延时 2s
	push 2000
	call Sleep

	; 退出
	push 0
	call ExitProcess
	ret
endGame endp


;	----------------------------------------------------------------------------------------------------------------------------------------
clearScreenUtil proc
	mov eax, dword ptr offset clearScreen
	push eax
	call system
	add esp, 4
	ret
clearScreenUtil endp



;	----------------------------------------------------------------------------------------------------------------------------------------
gotoxyUtil proc C pos:dword
	mov eax, dword ptr ds : [pos]
	push eax

	push -11
	call GetStdHandle
	push eax

	call SetConsoleCursorPosition
	ret
gotoxyUtil endp




;	----------------------------------------------------------------------------------------------------------------------------------------

;	##################################################################################
main proc
	call enterGame
main endp
end main
