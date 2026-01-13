; ==============================================================================
; INTENT STRUCTURE DEFINITIONS (NASM)
; ==============================================================================
; Parity with corelib/core/intent.fox and intent.s
; ==============================================================================

; ------------------------------------------------------------------------------
; NODE HEADER OFFSETS
; ------------------------------------------------------------------------------
INTENT_OFFSET_TYPE      equ 0x00 ; i64
INTENT_OFFSET_NEXT      equ 0x08 ; ptr
INTENT_OFFSET_CHILD     equ 0x10 ; ptr
INTENT_OFFSET_HINT      equ 0x18 ; ptr
INTENT_OFFSET_DATA_A    equ 0x20 ; i64/ptr
INTENT_OFFSET_DATA_B    equ 0x28 ; i64/ptr
INTENT_NODE_SIZE        equ 48   ; Total Size

; ------------------------------------------------------------------------------
; NODE TYPES (Enum)
; ------------------------------------------------------------------------------
; Level 1: UNIT
INTENT_UNIT_MODULE      equ 0x1001

; Level 2: SHARD
INTENT_SHARD_FUNC       equ 0x2001
INTENT_SHARD_BLOCK      equ 0x2002

; Level 3: FRAGMENT
INTENT_FRAG_LITERAL     equ 0x3001
INTENT_FRAG_BINARY      equ 0x3002
INTENT_FRAG_UNARY       equ 0x3003
INTENT_FRAG_CALL        equ 0x3004
INTENT_FRAG_VAR         equ 0x3005 ; Read
INTENT_FRAG_ASSIGN      equ 0x3006 ; Write
INTENT_FRAG_IF          equ 0x3007
INTENT_FRAG_WHILE       equ 0x3008
INTENT_FRAG_RETURN      equ 0x3009
INTENT_FRAG_SWITCH      equ 0x300A
INTENT_FRAG_CASE        equ 0x300B
INTENT_FRAG_STRING      equ 0x4002
INTENT_FRAG_SYSCALL     equ 0x4003
