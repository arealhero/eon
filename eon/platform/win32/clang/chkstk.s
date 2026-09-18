// NOTE(vlad): Since we are running in CRT-free environment we need to provide __chkstk ourselves.

        .intel_syntax   noprefix

        .globl          __chkstk

        .text

        PAGESIZE = 4096

__chkstk:
        sub     rsp, 16
        mov     qword ptr [rsp], r10
        mov     qword ptr [rsp + 8], r11
        xor     r11, r11
        lea     r10, [rsp + 24]
        sub     r10, rax
        cmovb   r10, r11
        mov     r11, qword ptr gs:[16]
        cmp     r10, r11
        jae     done
        and     r10w, 0x0F000

force_commit_page:
        lea     r11, [r11 - PAGESIZE]
        mov     byte ptr [r11], 0
        cmp     r10, r11
        jne     force_commit_page

done:
        mov     r10, qword ptr [rsp]
        mov     r11, qword ptr [rsp + 8]
        add     rsp, 16
        ret
