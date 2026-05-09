# MicroVm: Brainfuck Optimizations 🚀

This branch (`optimizations`) transforms our naive Brainfuck interpreter into a highly optimized, bytecode-driven Virtual Machine. By adding an Intermediate Representation (IR) and a Peephole Optimizer, execution time for complex programs (like a Mandelbrot fractal generator) is significantly reduced.

## What Was Done

1. **Intermediate Representation (IR):** The raw Brainfuck characters are no longer interpreted at runtime. The source is pre-compiled into a flat array of structures (`instruction_t`) called IR or bytecodes. 
2. **Infinite Tape:** The memory tape was expanded to a 1 Megabyte heap allocation. More importantly, we removed bounds checking (`% TAPE_SIZE`) during pointer movement (`IR_MOVE`). If a program misbehaves, it crashes, but correct programs run at maximum CPU speed.
3. **Peephole Optimizer:** A secondary pass over the compiled IR detects common, slow Brainfuck patterns and replaces them with dedicated, lightning-fast instructions.

---

## How It Works (With Examples)

### 1. Run-Length Encoding (RLE)
In naive Brainfuck, moving the pointer 5 times to the right (`>>>>>`) requires 5 iterations of the execution loop. 
During our IR compilation, contiguous identical instructions are folded into a single instruction with an operand.

* **Raw:** `>>>>>`
* **IR:** `IR_MOVE (operand: 5)`

### 2. Loop Clearing (`IR_CLEAR`)
Brainfuck programs frequently clear a cell (set it to 0) using a simple loop: `[-]`. Naively, this decrements the cell by 1 until it hits 0, which is extremely slow for large numbers.
Our compiler recognizes this exact character sequence and translates it into a direct memory zeroing operation.

* **Raw:** `[-]`
* **IR:** `IR_CLEAR`
* **C execution:** `tape[data_pointer] = 0;`

### 3. Multiplication Loops (`IR_MUL`)
The most significant optimization. Brainfuck lacks native math, so adding 3 to an adjacent cell looks like this: `[->+++<]`. This means: *subtract 1 here, move right, add 3, move left, repeat until 0.*

Our Peephole Optimizer simulates loops. If it finds a loop that only moves the pointer, adds/subtracts values, always returns the pointer to its starting position (`offset: 0`), and decrements the original cell by exactly 1... it converts it into native multiplications!

* **Raw:** `[->+++<]`
* **IR:** 
  * `IR_MUL (operand: 3, offset: 1)`
  * `IR_CLEAR`
  * `IR_NOP` (padding)

By embedding the relative distance (`offset`) into the `IR_MUL` instruction, the VM can use native base+offset addressing without actually moving its internal pointer back and forth. 
* **C execution:** `tape[data_pointer + inst.offset] += tape[data_pointer] * inst.operand;`