// reset stack, plus one for thumb mode
.section .header, ""

.int __end_of_ram
.int reset + 1

.rept 48
.int __unhandled_exception
.endr

.section .header.text, "x"

.text
.thumb_func
.global reset
.balign 4
reset:
  // Set up exception stack
  mrs r1,CONTROL
  mov r0,#2
  orr r0,r1
  msr CONTROL,r0
  isb
  ldr r0,=__end_of_ram - 256
  mov sp,r0
  // msr CONTROL,r1
  // isb

  // Copy the data image from flash to SRAM
  ldr r0,=__image_data_st_vma
  ldr r1,=__image_data_st_lma
  ldr r2,=__image_data_en_vma
  sub r2,r0
  bl memcpy
  ldr r0,=__image_bss_st
  ldr r2,=__image_bss_en
  sub r2,r0
  mov r1,#0
  bl memset
  ldr r4,=__init_array_st
  ldr r5,=__init_array_en
  cmp r4,r5
  beq .Linit_done
.Linit_next:
  ldr r0,[r4]
  tst r0,r0
  beq .Linit_done
  blx r0
  add r4,#4
  cmp r4,r5
  bne .Linit_next
.Linit_done:
  bl main
  bl abort

.thumb_func
.global memset
.balign 4
memset:
  add r0,r2
  mvn r2,r2
  add r2,#1
  beq .Lmemset_done
  mov r3,#3
  tst r0,r3
  bne .Lmemset_more
  tst r2,r3
  bne .Lmemset_more
  ldr r3,=0x01010101
  uxtb r1,r1
  mul r1,r3
  mov r3,#16
.Lmemset_batch:
  cmn r2,r3
  bcs .Lmemset_fast
  str r1,[r0,r2]
  add r2,#4
  str r1,[r0,r2]
  add r2,#4
  str r1,[r0,r2]
  add r2,#4
  str r1,[r0,r2]
  add r2,#4
  beq .Lmemset_done
  b .Lmemset_batch
.Lmemset_fast:
  str r1,[r0,r2]
  add r2,#4
  bne .Lmemset_fast
  bx lr

.Lmemset_more:
  strb r1,[r0,r2]
  add r2,#1
  bne .Lmemset_more
.Lmemset_done:
  bx lr

.balign 4
.thumb_func
.global memcpy
memcpy:
  // Fastpath a bunch of common easy cases
  cmp r2,#4
  beq .Lone_word
  bcs .Lnot_small
  cmp r2,#1
  beq .Lone_byte
  bcc .Ljust_return
  cmp r2,#2
  beq .Lhalf_word
.Lnot_small:
  // Prepare to return the passed destination pointer
  push {r0}

  // Clear copy loop offset
  mov r3,#0

  // Point pointers to ends of buffers
  add r1,r2
  add r0,r2

  // Negate the size and done if zero
  mvn r2,r2
  add r2,#1
  beq .Lmemcpy_done

  // Constant for checking lowest two bits
  mov r3,#3

  // If the size is not a multiple of 32 bits, copy bytes
  tst r2,r3
  bne .Lmemcpy_bytes_loop

  // If the destination is not 32 bit aligned, copy bytes
  tst r0,r3
  bne .Lmemcpy_bytes_loop

  // If the source is not 32 bit aligned, copy bytes
  tst r1,r3
  bne .Lmemcpy_bytes_loop

.Lmemcpy_word_batch_loop:
  mov r3,#16
  cmn r2,r3
  bcc .Lmemcpy_words_loop
  ldr r3,[r1,r2]
  str r3,[r0,r2]
  add r2,#4
  ldr r3,[r1,r2]
  str r3,[r0,r2]
  add r2,#4
  ldr r3,[r1,r2]
  str r3,[r0,r2]
  add r2,#4
  ldr r3,[r1,r2]
  str r3,[r0,r2]
  add r2,#4
  bne .Lmemcpy_word_batch_loop

.Lmemcpy_words_loop:
  ldr r3,[r1,r2]
  str r3,[r0,r2]
  add r2,#4
  bne .Lmemcpy_words_loop
  b .Lmemcpy_done

.balign 4
.Lone_byte:
  ldrb r3,[r1]
  strb r3,[r0]
.Ljust_return:
  bx lr

.Lhalf_word:
  mov r3,#1
  tst r0,r3
  bne .Lnot_small
  tst r1,r3
  bne .Lnot_small
  ldrh r3,[r1]
  strh r3,[r0]
  bx lr

.Lone_word:
  mov r3,#3
  tst r0,r3
  bne .Lnot_small
  tst r1,r3
  bne .Lnot_small
  ldr r3,[r1]
  str r3,[r0]
  bx lr

.balign 4
.Lmemcpy_bytes_loop:
  ldrb r3,[r1,r2]
  strb r3,[r0,r2]
  add r2,#1
  bne .Lmemcpy_bytes_loop

.Lmemcpy_done:
  pop {r0}
  bx lr

.thumb_func
__unhandled_exception:
  b abort
