#include <stdio.h>
#include <stdint.h>


typedef enum  {
    HALT        = 0x00,  // Stops execution
    PRINT_CHAR  = 0x01,  // Print program[pc+1] as ASCII. pc += 2
    PUSH        = 0x02,  // Push program[pc+1] onto the stack. pc += 2
    ADD         = 0x03,  // Pop two values, push their sum.pc += 1
    PRINT_TOP   = 0x04,  // Pop the top of the stack and print it as ASCII. pc += 1

} opcode_t;


uint8_t program1[] = {
    2, 'H',       // PUSH 'H'
    4,             // PRINT_TOP
    2, 'i',       // PUSH 'i'
    4,             // PRINT_TOP
    0              // HALT
};

uint8_t program2[] = {
    2, 65,         // PUSH 65  ('A')
    2, 2,          // PUSH 2
    3,             // ADD       (stack top = 67)
    4,             // PRINT_TOP (prints 'C')
    0              // HALT
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

        }

        opcode = program[pc];
    }

}

int main() {


    printf("Program 1: \n");
    run(program1);
    printf("\n----------------------------------\n");
    printf("Program 2: \n");
    run(program2);


	return 0;
}
