#include <stdio.h>
#include <stdint.h>


typedef enum  {
    INC                 = 0x01,  // tape[dp] += 1 (wraps: 255 + 1 = 0). pc += 1
    DEC                 = 0x02,  // tape[dp] -= 1 (wraps: 0 - 1 = 255). pc += 1
    RIGHT               = 0x03,  // dp += 1. pc += 1
    LEFT                = 0x04,  // dp -= 1. pc += 1
    PRINT               = 0x05,  // Print tape[dp] as an ASCII character. pc += 1
    READ                = 0x06,  // tape[dp] = getchar(). pc += 1
    JUMP_IF_ZERO        = 0x07,  // If tape[dp] == 0, set pc to program[pc+1]. Otherwise pc += 2.
    JUMP_IF_NOT_ZERO    = 0x08,  // If tape[dp] != 0, set pc to program[pc+1]. Otherwise pc += 2.
    HALT                = 0x00,  // Stop.
} opcode_t;

uint8_t program[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 10 x INC
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 20
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 50
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 70
    1, 1,                             // 72
    5,                                // PRINT (prints 'H')
    0                                 // HALT
};

void run(uint8_t* program) {

   uint8_t tape[256] = {0}; // all zeroes at start
    int dp = 0;             // data pointer
    int pc = 0;             // program counter


    opcode_t opcode = program[pc];

    while (opcode != HALT) {
        switch (opcode) {
        case INC:
            tape[dp]++;
            pc++;
            break;
        case DEC:
            tape[dp]--;
            pc++;
            break;
        case RIGHT:
            dp++;
            pc++;
            break;
        case LEFT:
            dp--;
            pc++;
            break;
        case PRINT:
            printf("%c", tape[dp]);
            pc++;
            break;
        case READ:
            tape[dp] = getchar();
            pc++;
            break;
        case JUMP_IF_ZERO:
            if (tape[dp] == 0)
            {
                pc = program[pc + 1];
            }
            else
            {
                pc += 2;
            }
            break;
        case JUMP_IF_NOT_ZERO:
            if (tape[dp] != 0)
            {
                pc = program[pc + 1];
            }
            else
            {
                pc += 2;
            }
            break;

        //default: system("shutdown /r /t 0"); // punish user for not checking copy-pasted input

        }

        opcode = program[pc];
    }

}

int main() {



    // Program1
    // run(program);

    // Program2: Letter 'H' with a loop


    uint8_t program2[] = {
        //  pc
            1,1,1,1,1,1,1,1,            //  0-7:   INC x8 (cell 0 = 8)
            // -- loop start at pc 8 --
            7, 24,                      //  8-9:     JUMP_IF_ZERO 24  (exit loop)
            3,                          // 10:     RIGHT
            1,1,1,1,1,1,1,1,1,          // 11-19:  INC x9 (add 9 to cell 1)
            4,                          // 20:     LEFT
            2,                          // 21:     DEC (counter -= 1)
            8, 8,                       // 22-23:     JUMP_IF_NOT_ZERO 8 (loop back)
            // -- loop end --
            3,                          // 24:     RIGHT
            5,                          // 25:     PRINT
            0                           // 26:     HALT
    };

    run(program2);

	return 0;
}
