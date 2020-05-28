  .include "macros.inc"
  .include "mpu.inc"
  .include "mmu.inc"

  start

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; check the MPU_BUILD
test00:
  .equ VERSION   , 0x03
  .equ NR_REGIONS, 0x08
  .equ MPU_BCR_REF, (NR_REGIONS << 8) | VERSION
  lr r0, [mpu_build]
  assert_eq MPU_BCR_REF, r0, 0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; All of the registers should be accessible in kernel mode
; this test (check_mpu) is based on 8 regions.
test01:
  ; mpuen : momenterally enabled with full access
  ; when read, only relevant bits must be set.
  auxreg_write_read mpuen   , 0xffffffff, 0x400001f8, 1
  ; disable mpu at once
  mpu_disable
  auxreg_write_read mpurdb0 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp0 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb1 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp1 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb2 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp2 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb3 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp3 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb4 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp4 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb5 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp5 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb6 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp6 , 0xffffffff, 0x00000ffb, 0x01
  auxreg_write_read mpurdb7 , 0xffffffff, 0xffffffe1, 0x01
  auxreg_write_read mpurdp7 , 0xffffffff, 0x00000ffb, 0x01


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; None of the registers should be accessible in user mode
test02:
  mpu_reset
  ; prep the exception for the end
  lr r0, [mpuic]    ; don't care for mpu_ecr value
  mpu_set_excep_params mpu_ecr  = r0                  , \
                       ecr      = PRIVILEGE_VIOLATION , \
                       efa      = @test02_user_space+4, \
                       eret     = @test02_user_space+4, \
                       continue = @test03             , \
                       test_num = 0x02
  enter_user_mode @test02_user_space
test02_user_space:
  add r0, r0, r0             ; some filler to make a basic block
  ; accessing MPU registers in user mode is not allowed
  lr r0, [mpu_build]
  assert_eq 0, 1, 2          ; exception must have been raised


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; running with -global cpu.has-mpu=false or reading a region register
; which is higher than configured number of regions causes an
; instuction error: ecr=0x020000
test03:
  mpu_reset
  ; prep the exception for 'lr'ing a region that does not exist
  lr r0, [mpuic]    ; don't care for mpu_ecr value
  mpu_set_excep_params mpu_ecr  = r0                    , \
                       ecr      = ILLEGAL_INSTRUCTION   , \
                       efa      = @test03_illegal_lr_rgn, \
                       eret     = @test03_illegal_lr_rgn, \
                       continue = @test03_cont          , \
                       test_num = 3
test03_illegal_lr_rgn:
  lr r1, [mpurdb15]
  assert_eq 0, 1, 3
test03_cont:
  ; prep the exception for 'sr'ing a region that does not exist
  lr r0, [mpuic]    ; don't care for mpu_ecr value
  mpu_set_excep_params mpu_ecr  = r0                    , \
                       ecr      = ILLEGAL_INSTRUCTION   , \
                       efa      = @test03_illegal_sr_rgn, \
                       eret     = @test03_illegal_sr_rgn, \
                       continue = @test04               , \
                       test_num = 3
test03_illegal_sr_rgn:
  sr r1, [mpurdp8]
  assert_eq 0, 1, 3


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; giving KR to region 1
; checking read is ok and write raise an exception
test04:
  .equ MEM_ADDR04  , 0x4000
  .equ DATA04      , 0x1337
  .equ MPU_ECR_W_R1, MPU_ECR_WRITE | 1
  mpu_reset
  mpu_add_base    mpurdb1, MEM_ADDR04
  mpu_add_region  mpurdp1, REG_MPU_EN_KR, MPU_SIZE_32B
  mpu_write_data  DATA04, MEM_ADDR04
  mpu_enable
  ; read permission check
  mpu_verify_data DATA04, MEM_ADDR04, 4
  ; write permission check
  mpu_set_excep_params mpu_ecr  = MPU_ECR_W_R1           , \
                       ecr      = PROTV_WRITE_MPU        , \
                       efa      = MEM_ADDR04             , \
                       eret     = @test04_illegal_store+4, \
                       continue = @test05                , \
                       test_num = 4
test04_illegal_store:
  add r0, r0, r0            ; filler; so exception happens in...
  st  r1, [MEM_ADDR04]      ; ...the middle of a translation block
  assert_eq 0, 1, 4         ; an exception must have been raised


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; having 2 small region next to each other: one with write permission
; and the other one with read permission. checking if permissions are
; respected accordingly. this tests how MPU sets QEmu's internal TLB
; and if it is able to set the TLB's entry size correctly.
test05:
  .equ MEM_ADDR05, 0x4024     ; 4 bytes above the multiple of 32
  .equ DATA05    , 0xbabe
  mpu_reset
  mpu_add_base    mpurdb0, MEM_ADDR05   ; effective address would be 0x4020
  mpu_add_region  mpurdp0, REG_MPU_EN_KW, MPU_SIZE_32B
  mpu_add_base    mpurdb1, MEM_ADDR05+32; effective address would be 0x4040
  mpu_add_region  mpurdp1, REG_MPU_EN_KR, MPU_SIZE_32B
  mpu_write_data  DATA05, MEM_ADDR05+32 ; write to 0x4044 (region1)
  ; let the fun begin
  mpu_enable
  mpu_verify_data DATA05, MEM_ADDR05+32, 5
  st r7, [MEM_ADDR05]                   ; write bogus data (region 0)
  ; now time for some exception
  mpu_set_excep_params mpu_ecr  = MPU_ECR_W_R1         , \
                       ecr      = PROTV_WRITE_MPU      , \
                       efa      = MEM_ADDR05+32        , \
                       eret     = @test05_illegal_store, \
                       continue = @test06              , \
                       test_num = 5
test05_illegal_store:
  st r7, [MEM_ADDR05+32]    ; this shouldn't be allowed
  assert_eq 0, 1, 5         ; an exception must have been raised


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; update a region's permission and size to see if they are taken
; into account.
test06:
  .equ MEM_ADDR06,   0x30000
  .equ MPU_ECR_R_R3, MPU_ECR_READ | 3
  mpu_reset
  mpu_add_base    mpurdb3, MEM_ADDR06
  mpu_add_region  mpurdp3, REG_MPU_EN_KR, MPU_SIZE_64B
  mpu_enable
  ld r7, [MEM_ADDR06+32]    ; this should be allowed
  ; changing permission (deliberately mpu is not disabled)
  mpu_add_region  mpurdp3, REG_MPU_EN_KE, MPU_SIZE_64B ; update (KR -> KE)
  ; prep for exception
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_R3        , \
                       ecr      = PROTV_READ_MPU      , \
                       efa      = MEM_ADDR06+32       , \
                       eret     = @test06_illegal_read, \
                       continue = @test06_change_size , \
                       test_num = 6
test06_illegal_read:
  ld r7, [MEM_ADDR06+32]    ; this is not allowed anymore
  assert_eq 0, 1, 6         ; an exception must have been raised
test06_change_size:
  ; changing size (deliberately mpu is not disabled)
  mpu_add_region  mpurdp3, REG_MPU_EN_KE, MPU_SIZE_32B ; update (64 -> 32)
  mpu_enable
  ld r7, [MEM_ADDR06+32]    ; this is allowed again (+32 is in def. region)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; check a permission that has only execute permission.
; the read should not be possible.
test07:
  .equ NOP_OPCODE, 0x7000264a
  .equ JR1_OPCODE, 0x00402020
  .equ CODE_CAVE07, 0x40000
  .equ MPU_ECR_R_R0, MPU_ECR_READ | 0
  mpu_reset
  mpu_add_base    mpurdb0, CODE_CAVE07
  mpu_add_region  mpurdp0, REG_MPU_EN_KE, MPU_SIZE_32B
  mov r0, NOP_OPCODE
  mov r1, @test07_rest
  mov r2, JR1_OPCODE
  st  r0, [CODE_CAVE07]    ; nop
  st  r2, [CODE_CAVE07+4]  ; j [r1]
  st  r0, [CODE_CAVE07+8]  ; nop
  mpu_enable
  ; let's take a leap of faith
  j CODE_CAVE07

test07_rest:
  ; wow, if we just came back, let's raise hell
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_R0        , \
                       ecr      = PROTV_READ_MPU      , \
                       efa      = CODE_CAVE07+4       , \
                       eret     = @test07_illegal_read, \
                       continue = @test08             , \
                       test_num = 7
test07_illegal_read:
  ld r7, [CODE_CAVE07+4]    ; this shouldn't be allowed
  assert_eq 0, 1, 7         ; an exception must have been raised


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; one region to rule them all
; 1) we are testing a very big region here.
; 2) moreover we change its permission and size in the middle
test08:
  .equ MEM_ADDR08   , 0x00000000
  .equ BIG_ADDR08   , 0x7FFFFFE0
  .equ MPU_ECR_W_R7 , MPU_ECR_WRITE | 7
  .equ MPU_ECR_R_DEF, MPU_ECR_READ | 0xFF
  .equ DATA08_1     , 0x34fa                ; random magic
  .equ DATA08_2     , 0x987afb              ; random magic
  mpu_reset
  ; planting data
  mpu_write_data  DATA08_1, BIG_ADDR08
  ; a 4 gigabyte region with read and execute permissions
  mpu_add_base    mpurdb7, MEM_ADDR08
  mpu_add_region  mpurdp7, REG_MPU_EN_KR | REG_MPU_EN_KE , MPU_SIZE_4G
  ; prepping exception (must be before enable, otherwise no write access)
  mpu_set_excep_params mpu_ecr  = MPU_ECR_W_R7             , \
                       ecr      = PROTV_WRITE_MPU          , \
                       efa      = BIG_ADDR08               , \
                       eret     = @test08_illegal_write    , \
                       continue = @test08_change_permission, \
                       test_num = 8
  ; default region with only write permission
  mpu_enable REG_MPU_EN_KW
  ; checking read (BIG_ADDR08) and exec (current instruction) permissions
  mpu_verify_data DATA08_1, BIG_ADDR08, 8
test08_illegal_write:
  st r7, [BIG_ADDR08]       ; no write is allowed
  assert_eq 0, 1, 8         ; an exception must have been raised
test08_change_permission:
  ; change permission _and_ size
  mpu_add_region  mpurdp7, REG_MPU_EN_FULL_ACCESS , MPU_SIZE_2G
  ; now there should be no problem in writing either
  mpu_write_data  DATA08_2, BIG_ADDR08
  mpu_verify_data DATA08_2, BIG_ADDR08, 8
  ; prepping second exception: default region has no read access
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_DEF           , \
                       ecr      = PROTV_READ_MPU          , \
                       efa      = BIG_ADDR08+0xF0         , \
                       eret     = @test08_illegal_def_read, \
                       continue = @test09                 , \
                       test_num = 8
test08_illegal_def_read:
  ld r7, [BIG_ADDR08+0xF0]  ; this is default region now and not sanctioned
  assert_eq 0, 1, 8         ; an exception must have been raised


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; a user cannot have kernel permissions, but a kernel inherits granted
; user permissions as well.
test09:
  .equ MEM_ADDR09_1, 0x60000
  .equ MEM_ADDR09_2, 0x62000              ; 8k after
  .equ MPU_ECR_W_R6, MPU_ECR_WRITE | 6
  .equ DATA09      , 0x89091              ; another random data from beyond
  mpu_reset
  ; a region for user to write
  mpu_add_base   mpurdb5, MEM_ADDR09_1
  mpu_add_region mpurdp5, REG_MPU_EN_UW, MPU_SIZE_8K
  ; a region only for kernel
  mpu_add_base   mpurdb6, MEM_ADDR09_2
  mpu_add_region mpurdp6, REG_MPU_EN_KR | REG_MPU_EN_KW, MPU_SIZE_8K
  ; prep the exception for the end
  mpu_set_excep_params mpu_ecr  = MPU_ECR_W_R6            , \
                       ecr      = PROTV_WRITE_MPU         , \
                       efa      = MEM_ADDR09_2            , \
                       eret     = @test09_user_space+8    , \
                       continue = @test09_rest_kernel_mode, \
                       test_num = 9
  ; let's have at it
  mpu_enable REG_MPU_EN_UE | REG_MPU_EN_KR
  enter_user_mode @test09_user_space
test09_user_space:
  st r7, [MEM_ADDR09_2-4]   ; write to the end of user region
  st r7, [MEM_ADDR09_2]     ; uh-oh: causing trouble
  assert_eq 0, 1, 9         ; we're looking for an exception; not this.
test09_rest_kernel_mode:
  ; a simple write and verify chore in kernel mode
  mpu_write_data  DATA09, MEM_ADDR09_2+64
  mpu_verify_data DATA09, MEM_ADDR09_2+64, 9
  ; also writing to user region because of implied write access
  mpu_write_data  DATA09, MEM_ADDR09_1+64
  mpu_disable       ; else we cannot verify (no read access)
  mpu_verify_data DATA09, MEM_ADDR09_1+64, 9


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; A region with only user read access should not be fetchable.
test10:
  .equ CODE_CAVE10  , 0x100000
  .equ DATA10      , 0x010101
  .equ MPU_ECR_E_R4, MPU_ECR_FETCH | 4
  mpu_reset
  mpu_add_base   mpurdb4, CODE_CAVE10
  mpu_add_region mpurdp4, REG_MPU_EN_UR, MPU_SIZE_64K
  ; plant data
  mpu_write_data DATA10, CODE_CAVE10
  ; prep the exception for the region being not executable
  mpu_set_excep_params mpu_ecr  = MPU_ECR_E_R4   , \
                       ecr      = PROTV_FETCH_MPU, \
                       efa      = CODE_CAVE10    , \
                       eret     = CODE_CAVE10    , \
                       continue = @test11        , \
                       test_num = 10
  mpu_enable
  enter_user_mode @test10_user_space
test10_user_space:
  mpu_verify_data DATA10, CODE_CAVE10, 10     ; read must be OK
  j @CODE_CAVE10                              ; this one not


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; KE must be enough for raising exception
; the tricky thing about this test is that it is allowing the
; parameters for the exceptions to be readable. as a result,
; the test assumes that there is 32 byte region that these
; parameters fit in AND it does not overlap with the exception
; routine itself.
test11:
  .equ MEM_ADDR11, 0x900
  mpu_reset
  ; allowing exception parameters to be read
  mpu_add_base   mpurdb0, @mpu_ecr_ref
  mpu_add_region mpurdp0, REG_MPU_EN_KR, MPU_SIZE_32B
  ; prep for the excepiton
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_DEF       , \
                       ecr      = PROTV_READ_MPU      , \
                       efa      = MEM_ADDR11          , \
                       eret     = @test11_illegal_read, \
                       continue = @test12             , \
                       test_num = 11
  mpu_enable REG_MPU_EN_KE
  add r0, r0, r0              ; just a random guy making a difference
test11_illegal_read:
  ld  r0, [MEM_ADDR11]
  assert_eq 0, 1, 11          ; an exception better be raised


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Double exception must raise Machine Check with memory management disabled.
; This test hangs in nSIM if MMU exists. Apparently, nSIM sets the halt flag
; if a MachineCheck is raised and there is MMU in the system. The  presence
; of MMU is necessary for test14.
test12:
  .equ MPU_ECR_E_DEF, MPU_ECR_FETCH | 0xFF
  mpu_reset
  ; enable MPU with no access whatsoever
  mpu_enable 0x0
test12_doomed:
  add r0, r0, r0
  lr  r0, [mpuen]
  assert_eq 0, r0, 12
  j @test13
  ; the machine check routine to be executed eventually
	.global	 EV_MachineCheck
	.type	   EV_MachineCheck, @function
	.align 4
EV_MachineCheck:
  lr   r0, [mpuen]
  brne r0, REG_MPU_EN_EN  , @test12_mc_fail
  lr   r0, [mpuic]
  brne r0, MPU_ECR_E_DEF  , @test12_mc_fail
  lr   r0, [ecr]
  brne r0, MACHINE_CHECK  , @test12_mc_fail
  lr   r0, [eret]
  brne r0, @test12_doomed , @test12_mc_fail
  lr   r1, [efa]
  breq r0, r1             , @test12_mc_success
test12_mc_fail:
  print "MachineCheck exception is not sane!\n"
  end
test12_mc_success:
  mpu_disable         ; disable MPU in a civilized way
  lr   r0, [erstatus] ; undo the mess:
  and  r0, r0, ~32    ; clear AE bit
  sr   r0, [erstatus] ; and
  rtie                ; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Page size for default region: best effort 8K, else 1 byte. You need
; to look into tracing to see if it is doing the right thing.
test13:
  .equ TWO_PAGES_BEFORE , 0x7C000
  .equ ONE_PAGE_BEFORE  , 0x7E000
  .equ MEM_ADDR13_1     , 0x80000
  .equ SAME_PAGE_BETWEEN, 0x80050
  .equ MEM_ADDR13_2     , 0x80100
  .equ SAME_PAGE_AFTER  , 0x81000
  .equ ONE_PAGE_AFTER   , 0x82000
  .equ MPU_ECR_R_R1     , MPU_ECR_READ | 1
  mpu_reset
  mpu_add_base   mpurdb3, MEM_ADDR13_1        ; \
  mpu_add_region mpurdp3, 0x0, MPU_SIZE_32B   ;  | two black holes
  mpu_add_base   mpurdb1, MEM_ADDR13_2        ;  | alike regions
  mpu_add_region mpurdp1, 0x0, MPU_SIZE_32B   ; /
  ; your exception shall be your salvation
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_R1        , \
                       ecr      = PROTV_READ_MPU      , \
                       efa      = MEM_ADDR13_2        , \
                       eret     = @test13_illegal_read, \
                       continue = @test14             , \
                       test_num = 13
  mpu_enable
  ld r0, [TWO_PAGES_BEFORE+0x1000]    ; must cache the page
  ld r0, [TWO_PAGES_BEFORE+0x1100]    ; reuse same information
  ld r0, [ONE_PAGE_BEFORE +0x1FFC]    ; oooh, just before the black hole
  ld r0, [ONE_PAGE_BEFORE +0x0500]    ; reuse from above
  ld r0, [SAME_PAGE_BETWEEN      ]    ; too narrow to cache the page
  ld r0, [SAME_PAGE_BETWEEN+0x10 ]    ; permissions must be totally checked
  ld r0, [SAME_PAGE_AFTER        ]    ; same page as the black holes
  ld r0, [SAME_PAGE_AFTER+0x10   ]    ; no caching must be used
  ld r0, [ONE_PAGE_AFTER         ]    ; this area is safe and ...
  ld r0, [ONE_PAGE_AFTER+0x04    ]    ; ...can be cached
test13_illegal_read:
  ld r0, [MEM_ADDR13_2           ]    ; oops!
  assert_eq 0, 1, 13


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; MMU and MPU may coexist but do not overlap.
; this test assumes an "rwe" access for exception routine checks and an "re"
; access for the page this test case is loaded in. if these two pages happen
; to be the same, e.g. previous tests are commented out, then things will
; get nasty, because the last attribute will be used for both.
test14:
  .equ MMU_KRNL_RE , REG_PD1_KRNL_E | REG_PD1_KRNL_R
  .equ MMU_KRNL_RWE, REG_PD1_KRNL_E | REG_PD1_KRNL_W | REG_PD1_KRNL_R
  .equ MMU_VPN_GV  , REG_PD0_GLOBAL | REG_PD0_VALID
  .equ MEM_ADDR14  , 0x80000100          ; an address in MPU's interest
  ; creates an entry in TLB with given permissions.
  ; the translation is identical (virt = physical)
  .macro add_mmu_entry addr, permission
    mov r0, \addr
    and r0, r0, PAGE_NUMBER_MSK
    or  r1, r0, \permission     ; r1 holds physical address and permissoins
    or  r0, r0, MMU_VPN_GV      ; r0 is a global valid virtual address
    mmu_tlb_insert r0, r1       ; add entry for MMU
  .endm
  mpu_reset
  ; add a read/write/execute permission for exception part page
  add_mmu_entry @mpu_ecr_ref, MMU_KRNL_RWE
  ; add a read/execute permission for current page
  lr r1, [pc]
  add_mmu_entry r1, MMU_KRNL_RE
  ; exception for writing to (2nd) MMU page
  lr r0, [mpuic]                ; don't care for mpu_ecr value
  mpu_set_excep_params mpu_ecr  = r0                   , \
                       ecr      = PROTV_WRITE_MMU      , \
                       efa      = r1                   , \
                       eret     = @test14_illegal_write, \
                       continue = @test14_mpu          , \
                       test_num = 14
  ; enable the guys
  mmu_enable                  ; enable MMU
  mpu_enable REG_MPU_EN_KW    ; enable MPU with kernel write access
  ; this is happening in MMU's territory
test14_illegal_write:
  st r0, [r1]       ; no write for this entry in TLB
  assert_eq 0, 1, 14

test14_mpu:
  add r0, r0, r0            ; a happy camper
  st  r0, [MEM_ADDR14]      ; in MPU realm
  ; MPU exception now
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_DEF       , \
                       ecr      = PROTV_READ_MPU      , \
                       efa      = MEM_ADDR14          , \
                       eret     = @test14_illegal_read, \
                       continue = @test14_wrapup      , \
                       test_num = 14
test14_illegal_read:
  ld  r0, [MEM_ADDR14]      ; uh-oh...
  assert_eq 0, 1, 14
test14_wrapup:
  mpu_disable
  mmu_disable


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; two overlapping regions test. one is 2 pages long and the other is inside
; the second page of the first region:
;        ,----------.
;        | R2:rw-   | region nr 2 with read/write permission.
; page1  |          |
;        |          |
; .......|..........|.......
;        |          |
; page2  |,________.|
;        ||R1:r--  || region nr 1 with read only permission.
;        |`--------'| this region is inside region nr 2.
;        `----------'
; setup: R2 is 16kb with rw-
;        R1 is  4kb with r--
; write to the first page           --> must go ok.
; write to the first half of page 2 --> must go ok.
; write to R1                       --> expect an exception.
; in the end read from R1           --> must go ok.
test15:
  .equ MEM_ADDR15_R2   , 0x150000
  .equ MEM_ADDR15_R2_P2, MEM_ADDR15_R2 + PAGE_SIZE
  .equ MEM_ADDR15_R1   , MEM_ADDR15_R2_P2 + PAGE_SIZE/2
  .equ DATA15_1        , 0x3ff0293f    ; random magic
  .equ DATA15_2        , DATA15_1+1
  .equ DATA15_3        , DATA15_1+2
  .equ MPU_ECR_W_R1, MPU_ECR_WRITE | 1
  mpu_reset
  mpu_add_base    mpurdb1, MEM_ADDR15_R1
  mpu_add_region  mpurdp1, REG_MPU_EN_KR, MPU_SIZE_4K
  mpu_add_base    mpurdb2, MEM_ADDR15_R2
  mpu_add_region  mpurdp2, REG_MPU_EN_KR|REG_MPU_EN_KW, MPU_SIZE_16K
  ; planting some data (for later read)
  mpu_write_data  DATA15_1, MEM_ADDR15_R1+24
  ; let the fun begin
  mpu_enable
  mpu_write_data  DATA15_2, MEM_ADDR15_R2+20
  mpu_verify_data DATA15_2, MEM_ADDR15_R2+20, 15
  mpu_write_data  DATA15_3, MEM_ADDR15_R2+20+PAGE_SIZE
  mpu_verify_data DATA15_3, MEM_ADDR15_R2+20+PAGE_SIZE, 15
  ; now time for some exception
  mpu_set_excep_params mpu_ecr  = MPU_ECR_W_R1         , \
                       ecr      = PROTV_WRITE_MPU      , \
                       efa      = MEM_ADDR15_R1+24     , \
                       eret     = @test15_illegal_store, \
                       continue = @test15_cont         , \
                       test_num = 15
  st r7, [MEM_ADDR15_R2_P2+32] ; write bogus data (region 2, page 2)
test15_illegal_store:
  st r7, [MEM_ADDR15_R1+24]    ; this shouldn't be allowed
  assert_eq 0, 1, 15           ; an exception must have been raised
test15_cont:
  mpu_verify_data DATA15_1, MEM_ADDR15_R1+24, 15 ; this is allowed


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; another overlapping regions test. in previous one, a page (nr=2) was split
; among two regions.  in  this  test, the page  is contained inside  another
; region, which in return is inside yet another region:
;        ,----------.
; page1  | R5:r---  | region nr 5 with read only permission.
; .......|..........|.......
; page2  |          |
; .......|..........|.......
; page3  |          |
; .......|..........|.......
; page4  |          |
; .......|,________.|.......
; page5  ||R3:-w-  || region nr 3 with write only permission.
; .......||........||.......
; page6  ||        || this region is inside region nr 5.
; .......|`--------'|.......
; page7  |          |
; .......|..........|.......
; page8  |          |
;        `----------'
; setup: R3 is 16kb with -w-
;        R5 is 64kb with r--
; read from the fourth page  --> must go ok.
; read from page 7           --> must go ok.
; write to page 4            --> expect an exception.
; write to page 5            --> must go ok.
; read from page 6           --> expect an exception.
test16:
  .equ MEM_ADDR16_R5   , 0x160000
  .equ MEM_ADDR16_R5_P4, MEM_ADDR16_R5 + 3*PAGE_SIZE
  .equ MEM_ADDR16_R5_P7, MEM_ADDR16_R5 + 6*PAGE_SIZE
  .equ MEM_ADDR16_R3   , MEM_ADDR16_R5 + 4*PAGE_SIZE
  .equ MEM_ADDR16_R3_P5, MEM_ADDR16_R3
  .equ MEM_ADDR16_R3_P6, MEM_ADDR16_R5 + 5*PAGE_SIZE
  .equ DATA16_1        , 0x93822093    ; random magic
  .equ DATA16_2        , DATA16_1+1
  .equ DATA16_3        , DATA16_1+2
  .equ MPU_ECR_R_R3, MPU_ECR_READ  | 3
  .equ MPU_ECR_W_R5, MPU_ECR_WRITE | 5
  mpu_reset
  mpu_add_base    mpurdb3, MEM_ADDR16_R3
  mpu_add_region  mpurdp3, REG_MPU_EN_KW, MPU_SIZE_16K
  mpu_add_base    mpurdb5, MEM_ADDR16_R5
  mpu_add_region  mpurdp5, REG_MPU_EN_KR, MPU_SIZE_64K
  ; planting some data (for later read)
  mpu_write_data  DATA16_1, MEM_ADDR16_R5_P4+24
  mpu_write_data  DATA16_3, MEM_ADDR16_R5_P7+24
  ; let the fun begin
  mpu_enable
  mpu_verify_data DATA16_1, MEM_ADDR16_R5_P4+24, 16
  mpu_verify_data DATA16_3, MEM_ADDR16_R5_P7+24, 16
  ; first exception because of writing in region 5
  mpu_set_excep_params mpu_ecr  = MPU_ECR_W_R5         , \
                       ecr      = PROTV_WRITE_MPU      , \
                       efa      = MEM_ADDR16_R5_P4+24  , \
                       eret     = @test16_illegal_store, \
                       continue = @test16_cont         , \
                       test_num = 16
test16_illegal_store:
  st r7, [MEM_ADDR16_R5_P4+24] ; this shouldn't be allowed
  assert_eq 0, 1, 16           ; an exception must have been raised
test16_cont:
  mpu_write_data  DATA16_2, MEM_ADDR16_R3_P5+24 ;will be checked later
  ; second exception while reading in region 3
  mpu_set_excep_params mpu_ecr  = MPU_ECR_R_R3        , \
                       ecr      = PROTV_READ_MPU      , \
                       efa      = MEM_ADDR16_R3_P6+24 , \
                       eret     = @test16_illegal_read, \
                       continue = @test16_end         , \
                       test_num = 16
test16_illegal_read:
  ld r7, [MEM_ADDR16_R3_P6+24] ; this shouldn't be allowed
  assert_eq 0, 1, 16           ; an exception must have been raised
test16_end:
  mpu_disable
  mpu_verify_data DATA16_2, MEM_ADDR16_R3_P5+24, 16 ; check if written


valhalla:
  print "[OK] Memory protection unit.\n"
  end
