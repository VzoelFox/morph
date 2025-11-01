"""
Modul ini mendefinisikan kelas-kelas untuk merepresentasikan setiap instruksi
dalam Intermediate Representation (IR) dari bahasa Morph.
"""

from dataclasses import dataclass
from typing import Any, List, Optional

# Base class untuk semua instruksi
class Instruction:
    """Kelas dasar untuk semua instruksi IR."""
    pass

# --- Operasi Memori ---

@dataclass
class LoadConst(Instruction):
    """Memuat nilai konstan ke dalam sebuah temporary."""
    value: Any
    dest: str  # Nama temporary, e.g., 't1'

@dataclass
class LoadVar(Instruction):
    """Memuat nilai dari variabel ke dalam sebuah temporary."""
    name: str
    dest: str

@dataclass
class StoreVar(Instruction):
    """Menyimpan nilai dari temporary/sumber ke dalam sebuah variabel."""
    name: str
    src: str

# --- Operasi Aritmatika & Perbandingan ---

@dataclass
class UnaryOp(Instruction):
    """Kelas dasar untuk operasi unary (seperti NEG)."""
    operand: str
    dest: str

class Negate(UnaryOp): pass

@dataclass
class BinaryOp(Instruction):
    """Kelas dasar untuk operasi biner (seperti ADD, SUB)."""
    left: str
    right: str
    dest: str

class Add(BinaryOp): pass
class Sub(BinaryOp): pass
class Mul(BinaryOp): pass
class Div(BinaryOp): pass
class GreaterThan(BinaryOp): pass
class LessThan(BinaryOp): pass

# --- Operasi Kontrol Alur ---

@dataclass
class Label(Instruction):
    """Mendefinisikan sebuah label sebagai target untuk lompatan."""
    name: str

@dataclass
class Jump(Instruction):
    """Lompatan tanpa syarat ke sebuah label."""
    target: str

@dataclass
class JumpIfFalse(Instruction):
    """Lompatan ke sebuah label jika kondisi bernilai false."""
    condition: str
    target: str

# --- Operasi Fungsi ---

@dataclass
class Call(Instruction):
    """Memanggil sebuah fungsi."""
    func: str
    args: List[str]
    dest: Optional[str] = None # Untuk menyimpan hasil return

@dataclass
class Return(Instruction):
    """Mengembalikan nilai dari sebuah fungsi."""
    value: Optional[str] = None
