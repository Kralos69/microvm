#include <stdio.h>
#include <stdint.h>


typedef enum  {
    HALT            = 0x00,  // Stops execution
    PRINT_CHAR      = 0x01,  // Print program[pc+1] as ASCII. pc += 2
    PUSH            = 0x02,  // Push program[pc+1] onto the stack. pc += 2
    ADD             = 0x03,  // Pop two values, push their sum.pc += 1
    PRINT_TOP       = 0x04,  // Pop the top of the stack and print it as ASCII. pc += 1
    JUMP            = 0x05,  // Set pc to program[pc+1]. (Don't add 2 — you're replacing pc entirely.)
    JUMP_IF_ZERO    = 0x06,  // Peek the top of the stack. If it was 0, set pc to program[pc+1]. If it wasn't 0, just do pc += 2 and keep going.

} opcode_t;


uint8_t program1[] = {
    2, 65,         // 0: PUSH 65 ('A')
    4,             // 2: PRINT_TOP
    5, 0,          // 3: JUMP 0  (go back to the start)
    0              // 5: HALT    (never reached)
};

//uint8_t program2[] = {
//    2, 3,          // 0: PUSH 3         (loop counter)
//    2, 65,         // 2: PUSH 65 ('A')
//    4,             // 4: PRINT_TOP      (prints 'A', counter is now on top)
//    2, 1,          // 5: PUSH 1
//    3,             // 7: ADD            (wait — this is counter + 1, not counter - 1!)
//};

uint8_t program3_orig[] = {
    2, 3,          //  0: PUSH 3           <- loop counter
    2, 65,         //  2: PUSH 65 ('A')
    4,             //  4: PRINT_TOP        <- prints 'A', counter back on top
    2, 255,        //  5: PUSH 255
    3,             //  7: ADD              <- counter = counter + 255 (wraps: subtract 1)
    6, 2,          //  8: JUMP_IF_ZERO 2   <- if counter hit 0, skip to pc=2... wait
    0              // 10: HALT
};

uint8_t program3_corrected[] = {
    2, 3,          //  0: PUSH 3           <- loop counter
    2, 65,         //  2: PUSH 65 ('A')
    4,             //  4: PRINT_TOP        <- prints 'A', counter back on top
    2, 255,        //  5: PUSH 255
    3,             //  7: ADD              <- counter = counter + 255 (wraps: subtract 1)
    6, 12,         //  8: JUMP_IF_ZERO 12  <- Jump to the exit.
    5, 2,          //  10: JUMP 2          <- Jump to the start of the loop.
    0              //  12: HALT
};


void run(uint8_t* program) {

    uint8_t pc = 0; // Program Counter

    opcode_t opcode = program[pc];

    uint8_t stack[256];
    uint8_t sp = 0;

    while (opcode != 0x00) {
        opcode = program[pc];
        switch (opcode) {
        case PRINT_CHAR:
            printf("%c", program[pc + 1]);
            pc += 2;
            break;
        case PUSH:
            stack[sp] = program[pc + 1];
            sp++;
            pc += 2;
            break;
        case ADD:
            if (sp >= 2)
            {
                sp--;
                stack[sp - 1] = stack[sp - 1] + stack[sp];
            }
            pc += 1;
            break;
        case PRINT_TOP:
            if (sp > 0)
            {
                sp--;
                printf("%c", stack[sp]);
            }
            pc += 1;
            break;
        case JUMP:
            pc = program[pc + 1];
            break;
        case JUMP_IF_ZERO:
            if (sp > 0 && stack[sp - 1] == 0)
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


    //printf("Program 1: \n");
    //run(program1); // Infinite loop
    //printf("\n----------------------------------\n");
    //printf("Program 2: \n");
    //run(program2);
    printf("\n----------------------------------\n");
    printf("Program 3: \n");
    run(program3_corrected);


	return 0;
}
