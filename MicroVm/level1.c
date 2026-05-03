#include <stdio.h>
#include <stdint.h>


void run(uint8_t* program) {

    uint8_t pc = 0; // Program Counter

    uint8_t opcode = program[pc];

    while (opcode != 0x00) {

        if (opcode == 0x01) {
            printf("%c", program[pc + 1]);
        }
        pc = pc + 2;
        opcode = program[pc];
    }

}

int main() {

    uint8_t program[] = {
    1, 'H',
    1, 'e',
    1, 'l',
    1, 'l',
    1, 'o',
    1, ',',
    1, ' ',
    1, 'W',
    1, 'o',
    1, 'r',
    1, 'l',
    1, 'd',
    1, '!',
    1, '\n',
    0
    };



    run(program);



	return 0;
}
