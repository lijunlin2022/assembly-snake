TITLE Program Snake

.386
.model flat, stdcall
option casemap:none
includelib msvcrt.lib

ExitProcess proto, dwExitCode:dword
Sleep proto, timeSpan:dword
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

; 存储食物的位置
globalFoodX dword ?
globalFoodY dword ?

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
	call drawMap
	ret
startGame endp

;	----------------------------------------------------------------------------------------------------------------------------------------
showMainMenu proc
	; 打印分割字符
	mov eax, dword ptr offset dividingLine
	push eax
	call printf
	add esp, 4

	; 打印作者信息
	mov eax, dword ptr offset authorInfo
	push eax
	call printf
	add esp, 4

	; 打印提示信息
	mov eax, dword ptr offset operationGuide
	push eax
	call printf
	add esp, 4

	; 打印分割字符
	mov eax, dword ptr offset dividingLine
	push eax
	call printf
	add esp, 4

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
handleIllegalSelection proc
	; 打印错误信息
	mov eax, dword ptr offset errMsg
	push eax
	call printf
	add esp, 4
	
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
	mov eax, dword ptr offset nullCh
	push eax
	call printf
	add esp, 4
	jmp second_inc

	; 打印墙壁
draw_wall:
	mov eax, dword ptr offset wall
	push eax
	call printf
	add esp, 4
	jmp second_inc

	; 打印食物
draw_food:
	mov eax, dword ptr offset food
	push eax
	call printf
	add esp, 4
	jmp second_inc

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
endGame proc
	; 打印提示信息
	mov eax, dword ptr offset endGameTip
	push eax
	call printf
	add esp, 4

	; 延时 2s
	push 2000
	call Sleep
	add esp, 4

	; 退出
	push 0
	call ExitProcess
	add esp, 4
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


;	##################################################################################
main proc
	call enterGame
main endp
end main
