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

;##################################################################################
.data
;----------------------------------------------setWall()
; 循环设置墙壁时所用的变量
i dword ?	
;----------------------------------------------globalMapArr
; 开辟 625 个字节的数组来设置地图
globalMapArr byte 625 dup(?)

; 存储食物的位置
globalFoodX dword ?
globalFoodY dword ?

;----------------------------------------------showMainMenu()
dividingLine byte "-------------------------------------------------------------------", 0ah, 0
authorInfo byte "author: LiJunLin", 0ah, 0
operationGuide byte "按 1 开始游戏", 0ah, "按 2 结束游戏", 0ah, "按 W 向上移动",  0ah, "按 S 向下移动", 0ah, "按 A 向左移动", 0ah, "按 D 向右移动", 0ah, 0

;---------------------------------------------enterGame()
paramater byte "%d", 0
selection dword ?

;----------------------------------------------handleIllegalSelection()
errMsg byte "输入的编号不正确，请在 2s 之后重新输入", 0ah, 0

;----------------------------------------------clearScreenUtil()
clearScreen byte "cls", 0

;##################################################################################
.code

;------------------------------------------------------
clearScreenUtil proc
	mov eax, dword ptr offset clearScreen
	push eax
	call system
	add esp, 4
	ret
clearScreenUtil endp
;------------------------------------------------------

;------------------------------------------------------
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
;------------------------------------------------------


;------------------------------------------------------
setWall proc
	mov dword ptr ds:[i], 0
	mov ecx, 25
set_wall:
	; 顶墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, dword ptr ds:[i]
	mov byte ptr ds:[eax + ebx], 0Bh
	; 底墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, 24
	imul ebx, ebx, 25
	add eax, ebx
	mov ebx, dword ptr ds:[i]
	mov byte ptr ds : [eax + ebx] , 0Bh
	; 左墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, dword ptr ds:[i]
	imul ebx, ebx, 25
	mov byte ptr ds : [eax + ebx] , 0Bh
	; 右墙
	lea eax, dword ptr ds:[globalMapArr]
	mov ebx, dword ptr ds:[i]
	imul ebx, ebx, 25
	add ebx, 24
	mov byte ptr ds : [eax + ebx] , 0Bh
	; i 自减
	mov ebx, dword ptr ds:[i]
	inc ebx
	mov dword ptr  ds:[i] , ebx
	loop set_wall
	ret
setWall endp

;------------------------------------------------------
initMapData proc
	push 625
	push 0
	lea eax, dword ptr offset[globalMapArr]
	push eax
	call memset
	add esp, 12
	ret
initMapData endp

;------------------------------------------------------
startGame proc
	call initMapData
	call setWall
	ret
startGame endp
;------------------------------------------------------


;------------------------------------------------------
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
;------------------------------------------------------

;------------------------------------------------------
enterGame proc
	call showMainMenu
	
	; 获取用户选择
	lea eax, dword ptr ds:[selection]
	push eax
	mov ecx, dword ptr offset paramater
	push ecx
	call scanf
	add esp, 8

	; 判断用户的选择
	mov eax, dword ptr offset selection
	cmp eax, 1
	je start_game
	cmp eax, 2
	je end_game

	; 处理非法输入
	call handleIllegalSelection

start_game:
	call startGame
end_game:
	invoke printf, offset dividingLine
	invoke printf, offset dividingLine
	invoke ExitProcess, 0
	ret
enterGame endp
;------------------------------------------------------

main proc
	call enterGame
main endp
end main
