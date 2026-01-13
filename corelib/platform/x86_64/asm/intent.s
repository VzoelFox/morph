# ==============================================================================
# INTENT STRUCTURE DEFINITIONS (Assembly)
# ==============================================================================
# Parity with corelib/core/intent.fox
# Defines offsets and constants for the IntentTree (Unit -> Shard -> Fragment)
# ==============================================================================

# ------------------------------------------------------------------------------
# NODE HEADER OFFSETS (Common for all nodes)
# ------------------------------------------------------------------------------
.equ INTENT_OFFSET_TYPE,        0x00 # i64
.equ INTENT_OFFSET_NEXT,        0x08 # ptr (Next Sibling)
.equ INTENT_OFFSET_CHILD,       0x10 # ptr (First Child / Content)
.equ INTENT_OFFSET_HINT,        0x18 # ptr (Metadata)
.equ INTENT_OFFSET_DATA_A,      0x20 # i64/ptr (Payload A)
.equ INTENT_OFFSET_DATA_B,      0x28 # i64/ptr (Payload B)
.equ INTENT_NODE_SIZE,          48   # Total Size

# ------------------------------------------------------------------------------
# NODE TYPES (Enum)
# ------------------------------------------------------------------------------

# Level 1: UNIT (Global Scope / Module)
.equ INTENT_UNIT_MODULE,        0x1001

# Level 2: SHARD (Local Scope / Container)
.equ INTENT_SHARD_FUNC,         0x2001 # Function Definition
.equ INTENT_SHARD_BLOCK,        0x2002 # Generic Block (Scope Boundary)

# Level 3: FRAGMENT (Logic / Instruction)
.equ INTENT_FRAG_LITERAL,       0x3001 # Literal Value
.equ INTENT_FRAG_BINARY,        0x3002 # Binary Operation
.equ INTENT_FRAG_UNARY,         0x3003 # Unary Operation
.equ INTENT_FRAG_CALL,          0x3004 # Function Call
.equ INTENT_FRAG_VAR,           0x3005 # Variable Access (Read)
.equ INTENT_FRAG_ASSIGN,        0x3006 # Variable Assignment (Write)
.equ INTENT_FRAG_IF,            0x3007 # Control Flow: If
.equ INTENT_FRAG_WHILE,         0x3008 # Control Flow: While
.equ INTENT_FRAG_RETURN,        0x3009 # Control Flow: Return
.equ INTENT_FRAG_SWITCH,        0x300A # Control Flow: Switch (Pilih)
.equ INTENT_FRAG_CASE,          0x300B # Control Flow: Case (Kasus)

# ------------------------------------------------------------------------------
# PAYLOAD MAPPING
# ------------------------------------------------------------------------------
# UNIT_MODULE
#   Child  -> First Shard (Func/GlobalStatement)
#   Data A -> Filename (String Ptr)

# SHARD_FUNC
#   Child  -> First Fragment (Body)
#   Data A -> Function Name (String Ptr)
#   Data B -> Args List (Ptr)

# SHARD_BLOCK
#   Child  -> First Fragment (Body)

# FRAG_LITERAL
#   Data A -> Value (i64)

# FRAG_BINARY
#   Data A -> Operator (i64/char)
#   Child  -> Left Operand (Fragment)
#       Child->Next -> Right Operand (Fragment)

# FRAG_VAR
#   Data A -> Name (String Ptr)

# FRAG_ASSIGN (VAR_WRITE)
#   Data A -> Name (String Ptr)
#   Child  -> Value Expr (Fragment)

# FRAG_IF
#   Child  -> Condition (Fragment)
#       Child->Next -> True Block (Shard)
#       Child->Next->Next -> False Block (Shard)

# FRAG_SWITCH
#   Child -> Condition/Value (Fragment)
#   Data A -> List of CASE Fragments (Linked List)

# FRAG_CASE
#   Child -> Value to Match (Literal/Expr)
#   Data A -> Block (Shard) containing case body
