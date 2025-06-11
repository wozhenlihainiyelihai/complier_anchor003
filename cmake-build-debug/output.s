.MODEL SMALL
.STACK 200h

.DATA
    int_fmt db "%d", 10, 0
    str_fmt db "%s", 10, 0
    int_str_buffer db 12 dup(0)      ; 用于 _itoa 转换整数为字符串
    concat_buffer db 256 dup(0)     ; 用于字符串拼接的结果
    current_student_index dw ?
    num_students dw ?
    student_names dw ?
    test1 dw ?
    student_grades dw ?
    summary_key dw ?

.CODE
EXTERN _printf : NEAR, _itoa : NEAR, _strcpy : NEAR, _strcat : NEAR

main PROC
    mov ax, @data ; 设置数据段寄存器
    mov ds, ax
    call anchor_main ; 调用我们语言的入口函数
    mov ah, 4Ch ; DOS退出程序功能
    int 21h
main ENDP

    ; (=, F, _, grade)
    mov ax, WORD PTR F
    mov WORD PTR grade, ax

    ; (DEC_DYN_ARRAY, student_names, 3, 8)

    ; (STORE_AT, Justin, student_names, 0)
    mov bx, WORD PTR student_names ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 0 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, WORD PTR Justin ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, Anchor, student_names, 1)
    mov bx, WORD PTR student_names ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 1 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, WORD PTR Anchor ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, Alex, student_names, 2)
    mov bx, WORD PTR student_names ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 2 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, WORD PTR Alex ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (DEC_DYN_ARRAY, student_grades, 3, 8)

    ; (DEC_DYN_ARRAY, T0, 3, 8)

    ; (STORE_AT, 95.5, T0, 0)
    mov bx, WORD PTR T0 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 0 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 95.5 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, 89.0, T0, 1)
    mov bx, WORD PTR T0 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 1 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 89.0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, 92.0, T0, 2)
    mov bx, WORD PTR T0 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 2 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 92.0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, T0, student_grades, 0)
    mov bx, WORD PTR student_grades ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 0 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, WORD PTR T0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (DEC_DYN_ARRAY, T1, 3, 8)

    ; (STORE_AT, 78.0, T1, 0)
    mov bx, WORD PTR T1 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 0 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 78.0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, 82.5, T1, 1)
    mov bx, WORD PTR T1 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 1 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 82.5 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, 66.0, T1, 2)
    mov bx, WORD PTR T1 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 2 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 66.0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, T1, student_grades, 1)
    mov bx, WORD PTR student_grades ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 1 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, WORD PTR T1 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (DEC_DYN_ARRAY, T2, 3, 8)

    ; (STORE_AT, 55.0, T2, 0)
    mov bx, WORD PTR T2 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 0 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 55.0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, 61.0, T2, 1)
    mov bx, WORD PTR T2 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 1 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 61.0 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, 59.5, T2, 2)
    mov bx, WORD PTR T2 ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 2 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, 59.5 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (STORE_AT, T2, student_grades, 2)
    mov bx, WORD PTR student_grades ; 将 index 放入 bx
    shl bx, 1 ; index *= 2 (因为是WORD类型)
    mov si, 2 ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, WORD PTR T2 ; 将源值放入 ax
    mov [si], ax ; 存入内存

    ; (FUNC_BEGIN, get_grade_letter, _, _)

get_grade_letter PROC
    push bp ; 保存旧的基址指针
    mov bp, sp ; 设置新的基址指针
    sub sp, 32 ; 为局部变量分配栈空间

    ; (GET_PARAM, score, _, _)

    ; (>=, score, 90.0, T3)
    mov ax, WORD PTR [bp + 4]
    cmp ax, 90.0
    jge L19
    mov WORD PTR [bp-2], 0 ; 结果为 false
    jmp L20
L19:
    mov WORD PTR [bp-2], 1 ; 结果为 true
L20:

    ; (JUMPF, T3, _, L0)
    mov ax, WORD PTR [bp-2]
    cmp ax, 0
    je L0

    ; (=, A, _, grade)
    mov ax, WORD PTR [bp-6]
    mov WORD PTR [bp-8], ax

    ; (JUMP, _, _, L1)
    jmp L1

    ; (LABEL, L0, _, _)
L0:

    ; (>=, score, 80.0, T4)
    mov ax, WORD PTR [bp + 4]
    cmp ax, 80.0
    jge L21
    mov WORD PTR [bp-12], 0 ; 结果为 false
    jmp L22
L21:
    mov WORD PTR [bp-12], 1 ; 结果为 true
L22:

    ; (JUMPF, T4, _, L2)
    mov ax, WORD PTR [bp-12]
    cmp ax, 0
    je L2

    ; (=, B, _, grade)
    mov ax, WORD PTR [bp-16]
    mov WORD PTR [bp-8], ax

    ; (JUMP, _, _, L3)
    jmp L3

    ; (LABEL, L2, _, _)
L2:

    ; (>=, score, 70.0, T5)
    mov ax, WORD PTR [bp + 4]
    cmp ax, 70.0
    jge L23
    mov WORD PTR [bp-20], 0 ; 结果为 false
    jmp L24
L23:
    mov WORD PTR [bp-20], 1 ; 结果为 true
L24:

    ; (JUMPF, T5, _, L4)
    mov ax, WORD PTR [bp-20]
    cmp ax, 0
    je L4

    ; (=, C, _, grade)
    mov ax, WORD PTR [bp-24]
    mov WORD PTR [bp-8], ax

    ; (JUMP, _, _, L5)
    jmp L5

    ; (LABEL, L4, _, _)
L4:

    ; (>=, score, 60.0, T6)
    mov ax, WORD PTR [bp + 4]
    cmp ax, 60.0
    jge L25
    mov WORD PTR [bp-28], 0 ; 结果为 false
    jmp L26
L25:
    mov WORD PTR [bp-28], 1 ; 结果为 true
L26:

    ; (JUMPF, T6, _, L6)
    mov ax, WORD PTR [bp-28]
    cmp ax, 0
    je L6

    ; (=, D, _, grade)
    mov ax, WORD PTR [bp-32]
    mov WORD PTR [bp-8], ax

    ; (LABEL, L6, _, _)
L6:

    ; (LABEL, L5, _, _)
L5:

    ; (LABEL, L3, _, _)
L3:

    ; (LABEL, L1, _, _)
L1:

    ; (RETURN, grade, _, _)
    mov ax, WORD PTR [bp-8] ; 将返回值放入ax
    mov sp, bp
    pop bp
    ret

    ; (FUNC_END, get_grade_letter, _, _)
    mov sp, bp ; 释放局部变量空间
    pop bp ; 恢复旧的基址指针
    ret ; 返回
get_grade_letter ENDP

    ; (=, 3, _, count)
    mov ax, 3
    mov WORD PTR count, ax

    ; (=, 0, _, i)
    mov ax, 0
    mov WORD PTR i, ax

    ; (=, 0.0, _, sum)
    mov ax, 0.0
    mov WORD PTR sum, ax

    ; (FUNC_BEGIN, calculate_average, _, _)

calculate_average PROC
    push bp ; 保存旧的基址指针
    mov bp, sp ; 设置新的基址指针
    sub sp, 16 ; 为局部变量分配栈空间

    ; (GET_PARAM, grades, _, _)

    ; (<, i, count, T7)
    mov ax, WORD PTR [bp-2]
    cmp ax, WORD PTR [bp-4]
    jl L27
    mov WORD PTR [bp-6], 0 ; 结果为 false
    jmp L28
L27:
    mov WORD PTR [bp-6], 1 ; 结果为 true
L28:

    ; (LABEL, L7, _, _)
L7:

    ; (JUMPF, T7, _, L8)
    mov ax, WORD PTR [bp-6]
    cmp ax, 0
    je L8

    ; (+, sum, T8, sum)
    mov ax, WORD PTR [bp-12]
    add ax, WORD PTR [bp-14]
    mov WORD PTR [bp-12], ax

    ; (+, i, 1, i)
    mov ax, WORD PTR [bp-2]
    add ax, 1
    mov WORD PTR [bp-2], ax

    ; (LOAD_AT, T8, grades, i)
    mov bx, WORD PTR [bp-2] ; 将 index 放入 bx
    shl bx, 1 ; index *= 2
    mov si, WORD PTR [bp + 4] ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, [si] ; 从内存取值
    mov WORD PTR [bp-14], ax ; 存入目标

    ; (JUMP, _, _, L7)
    jmp L7

    ; (/, sum, count, T11)
    mov ax, WORD PTR [bp-12]
    cwd ; 将AX的符号扩展到DX
    mov bx, WORD PTR [bp-4]
    idiv bx
    mov WORD PTR [bp-16], ax

    ; (LABEL, L8, _, _)
L8:

    ; (RETURN, T11, _, _)
    mov ax, WORD PTR [bp-16] ; 将返回值放入ax
    mov sp, bp
    pop bp
    ret

    ; (FUNC_END, calculate_average, _, _)
    mov sp, bp ; 释放局部变量空间
    pop bp ; 恢复旧的基址指针
    ret ; 返回
calculate_average ENDP

    ; (+, Name: , name, T12)
    mov ax, WORD PTR Name: 
    add ax, WORD PTR name
    mov WORD PTR T12, ax

    ; (FUNC_BEGIN, print_student_report, _, _)

print_student_report PROC
    push bp ; 保存旧的基址指针
    mov bp, sp ; 设置新的基址指针
    sub sp, 18 ; 为局部变量分配栈空间

    ; (GET_PARAM, name, _, _)

    ; (GET_PARAM, grades, _, _)

    ; (PRINT, --- Student Report ---, _, _)
    push --- Student Report --- ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PRINT, T12, _, _)
    push WORD PTR [bp-4] ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PARAM, grades, _, _)
    push WORD PTR [bp + 4]

    ; (CALL, calculate_average, 1, T13)
    call calculate_average
    add sp, 2 ; 调用者清理参数占用的栈空间
    mov WORD PTR [bp-6], ax ; 保存返回值

    ; (=, T13, _, avg)
    mov ax, WORD PTR [bp-6]
    mov WORD PTR [bp-8], ax

    ; (PARAM, T13, _, _)
    push WORD PTR [bp-6]

    ; (CALL, get_grade_letter, 1, T14)
    call get_grade_letter
    add sp, 2 ; 调用者清理参数占用的栈空间
    mov WORD PTR [bp-10], ax ; 保存返回值

    ; (+, Final Letter Grade: , T14, T15)
    mov ax, WORD PTR [bp-12]
    add ax, WORD PTR [bp-10]
    mov WORD PTR [bp-14], ax

    ; (PRINT, Average Score:, _, _)
    push WORD PTR [bp-16] ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PRINT, avg, _, _)
    push WORD PTR [bp-8] ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PRINT, T15, _, _)
    push WORD PTR [bp-14] ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PRINT, ----------------------, _, _)
    push ---------------------- ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PRINT, , _, _)
    push  ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (FUNC_END, print_student_report, _, _)
    mov sp, bp ; 释放局部变量空间
    pop bp ; 恢复旧的基址指针
    ret ; 返回
print_student_report ENDP

    ; (PRINT, ======= Mini Grade System Starting =======, _, _)
    push WORD PTR ======= Mini Grade System Starting ======= ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (PRINT, , _, _)
    push  ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (=, 3, _, num_students)
    mov ax, 3
    mov WORD PTR num_students, ax

    ; (=, 0, _, current_student_index)
    mov ax, 0
    mov WORD PTR current_student_index, ax

    ; (<, current_student_index, num_students, T16)
    mov ax, WORD PTR current_student_index
    cmp ax, WORD PTR num_students
    jl L29
    mov WORD PTR T16, 0 ; 结果为 false
    jmp L30
L29:
    mov WORD PTR T16, 1 ; 结果为 true
L30:

    ; (LABEL, L9, _, _)
L9:

    ; (JUMPF, T16, _, L10)
    mov ax, WORD PTR T16
    cmp ax, 0
    je L10

    ; (+, current_student_index, 1, T17)
    mov ax, WORD PTR current_student_index
    add ax, 1
    mov WORD PTR T17, ax

    ; (+, Processing report for student #, T17, T18)
    mov ax, WORD PTR Processing report for student #
    add ax, WORD PTR T17
    mov WORD PTR T18, ax

    ; (PRINT, T18, _, _)
    push WORD PTR T18 ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (LOAD_AT, T19, student_grades, current_student_index)
    mov bx, WORD PTR current_student_index ; 将 index 放入 bx
    shl bx, 1 ; index *= 2
    mov si, WORD PTR student_grades ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, [si] ; 从内存取值
    mov WORD PTR T19, ax ; 存入目标

    ; (PARAM, T19, _, _)
    push WORD PTR T19

    ; (LOAD_AT, T20, student_names, current_student_index)
    mov bx, WORD PTR current_student_index ; 将 index 放入 bx
    shl bx, 1 ; index *= 2
    mov si, WORD PTR student_names ; 获取全局或指针数组基地址
    add si, bx ; 计算最终地址
    mov ax, [si] ; 从内存取值
    mov WORD PTR T20, ax ; 存入目标

    ; (PARAM, T20, _, _)
    push WORD PTR T20

    ; (CALL, print_student_report, 2, _)
    call print_student_report
    add sp, 4 ; 调用者清理参数占用的栈空间

    ; (+, current_student_index, 1, current_student_index)
    mov ax, WORD PTR current_student_index
    add ax, 1
    mov WORD PTR current_student_index, ax

    ; (=, current_student_index, _, current_student_index)
    mov ax, WORD PTR current_student_index
    mov WORD PTR current_student_index, ax

    ; (JUMP, _, _, L9)
    jmp L9

    ; (=, 1, _, te)
    mov ax, 1
    mov WORD PTR te, ax

    ; (LABEL, L10, _, _)
L10:

    ; (<, te, 10, T22)
    mov ax, WORD PTR te
    cmp ax, 10
    jl L31
    mov WORD PTR T22, 0 ; 结果为 false
    jmp L32
L31:
    mov WORD PTR T22, 1 ; 结果为 true
L32:

    ; (LABEL, L11, _, _)
L11:

    ; (JUMPF, T22, _, L13)
    mov ax, WORD PTR T22
    cmp ax, 0
    je L13

    ; (+, te, 1, te)
    mov ax, WORD PTR te
    add ax, 1
    mov WORD PTR te, ax

    ; (PRINT, te, _, _)
    push WORD PTR te ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (JUMP, _, _, L11)
    jmp L11

    ; (==, num_students, 1, T24)
    mov ax, WORD PTR num_students
    cmp ax, 1
    je L33
    mov WORD PTR T24, 0 ; 结果为 false
    jmp L34
L33:
    mov WORD PTR T24, 1 ; 结果为 true
L34:

    ; (==, num_students, 2, T25)
    mov ax, WORD PTR num_students
    cmp ax, 2
    je L35
    mov WORD PTR T25, 0 ; 结果为 false
    jmp L36
L35:
    mov WORD PTR T25, 1 ; 结果为 true
L36:

    ; (==, num_students, 3, T26)
    mov ax, WORD PTR num_students
    cmp ax, 3
    je L37
    mov WORD PTR T26, 0 ; 结果为 false
    jmp L38
L37:
    mov WORD PTR T26, 1 ; 结果为 true
L38:

    ; (LABEL, L13, _, _)
L13:

    ; (STORE_MEMBER, 1, test1, 0)
    lea si, WORD PTR test1 ; 获取结构体基地址到 SI
    add si, 0 ; 加上成员偏移量
    mov ax, 1 ; 获取要存储的源值到 AX
    mov [si], ax ; 将值存入计算出的内存地址

    ; (STORE_MEMBER, hello, test1, 2)
    lea si, WORD PTR test1 ; 获取结构体基地址到 SI
    add si, 2 ; 加上成员偏移量
    mov ax, WORD PTR hello ; 获取要存储的源值到 AX
    mov [si], ax ; 将值存入计算出的内存地址

    ; (PRINT, ======= Final Summary =======, _, _)
    push WORD PTR ======= Final Summary ======= ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (=, num_students, _, summary_key)
    mov ax, WORD PTR num_students
    mov WORD PTR summary_key, ax

    ; (JUMPNZ, T24, _, L15)
    ; 未处理的操作: JUMPNZ

    ; (JUMPNZ, T25, _, L16)
    ; 未处理的操作: JUMPNZ

    ; (JUMPNZ, T26, _, L17)
    ; 未处理的操作: JUMPNZ

    ; (JUMP, _, _, L18)
    jmp L18

    ; (LABEL, L15, _, _)
L15:

    ; (PRINT, Processed 1 student., _, _)
    push WORD PTR Processed 1 student. ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (JUMP, _, _, L14)
    jmp L14

    ; (LABEL, L16, _, _)
L16:

    ; (PRINT, Processed 2 students., _, _)
    push WORD PTR Processed 2 students. ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (JUMP, _, _, L14)
    jmp L14

    ; (LABEL, L17, _, _)
L17:

    ; (PRINT, Processed 3 students., _, _)
    push WORD PTR Processed 3 students. ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (JUMP, _, _, L14)
    jmp L14

    ; (LABEL, L18, _, _)
L18:

    ; (PRINT, Processed multiple students., _, _)
    push WORD PTR Processed multiple students. ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

    ; (LABEL, L14, _, _)
L14:

    ; (PRINT, ======= Grade System Finished =======, _, _)
    push WORD PTR ======= Grade System Finished ======= ; 压入整数值
    push OFFSET int_fmt ; 压入整数格式化符
    call _printf
    add sp, 4 ; 清理printf的参数栈

END main
