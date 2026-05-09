#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define TAPE_SIZE 30000
#define DEBUG_TRACE_ENABLED 0 // Disabled by default for normal CLI usage

typedef enum {
    IR_ADD,
    IR_MOVE,
    IR_PUT,
    IR_GET,
    IR_JZ,
    IR_JNZ,
    IR_CLEAR,
    IR_HALT
} ir_opcode_t;

typedef struct {
    ir_opcode_t opcode;
    int operand;
} instruction_t;

void print_instructions(const instruction_t* instructions)
{
    const char* opcode_names[] = {
        "ADD",
        "MOVE",
        "PUT",
        "GET",
        "JZ",
        "JNZ",
        "CLEAR",
        "HALT"
    };

    int i = 0;
    while (instructions[i].opcode != IR_HALT) {
        printf("%04d: %-5s %d\n", i, opcode_names[instructions[i].opcode], instructions[i].operand);
        i++;
    }
    printf("%04d: HALT\n", i);
}

instruction_t* compile_ir(const char* source_code)
{
    size_t source_length = strlen(source_code);
    instruction_t* instructions = malloc((source_length + 1) * sizeof(instruction_t));
    int* bracket_stack = malloc(source_length * sizeof(int));

    if (!instructions || !bracket_stack) {
        free(instructions);
        free(bracket_stack);
        return NULL;
    }

    int inst_index = 0;
    int stack_depth = 0;
    int i = 0;

    while (source_code[i] != '\0') {
        char current_char = source_code[i];

        switch (current_char) {
            case '+':
            case '-':
            case '>':
            case '<': {
                int amount = 0;
                while (source_code[i] == current_char) {
                    amount += (current_char == '+' || current_char == '>') ? 1 : -1;
                    i++;
                }

                instructions[inst_index].opcode = (current_char == '+' || current_char == '-') ? IR_ADD : IR_MOVE;
                instructions[inst_index].operand = amount;
                inst_index++;
                break;
            }

            case '.':
            case ',':
                instructions[inst_index].opcode = (current_char == '.') ? IR_PUT : IR_GET;
                instructions[inst_index].operand = 0;
                inst_index++;
                i++;
                break;

            case '[':
                // Optimization: Simple loop clear '[-]'
                if (source_code[i + 1] == '-' && source_code[i + 2] == ']') {
                    instructions[inst_index].opcode = IR_CLEAR;
                    instructions[inst_index].operand = 0;
                    inst_index++;
                    i += 3;
                    break;
                }

                bracket_stack[stack_depth++] = inst_index;
                instructions[inst_index].opcode = IR_JZ;
                instructions[inst_index].operand = -1; // Placeholder for destination
                inst_index++;
                i++;
                break;

            case ']': {
                if (stack_depth == 0) {
                    fprintf(stderr, "Error: Unmatched ']' at index %d\n", i);
                    free(instructions);
                    free(bracket_stack);
                    return NULL;
                }

                int open_index = bracket_stack[--stack_depth];
                instructions[open_index].operand = inst_index + 1;

                instructions[inst_index].opcode = IR_JNZ;
                instructions[inst_index].operand = open_index + 1;
                inst_index++;
                i++;
                break;
            }

            default:
                // Ignore comments and other unknown characters
                i++;
                break;
        }
    }

    free(bracket_stack);

    if (stack_depth != 0) {
        fprintf(stderr, "Error: Unmatched '[' at end of source\n");
        free(instructions);
        return NULL;
    }

    instructions[inst_index].opcode = IR_HALT;
    instructions[inst_index].operand = 0;

    return instructions;
}

void run_vm(const instruction_t* instructions)
{
    uint8_t tape[TAPE_SIZE] = { 0 };
    int data_pointer = 0;
    int program_counter = 0;

    while (1) {
        instruction_t inst = instructions[program_counter];

        switch (inst.opcode) {
            case IR_ADD:
                tape[data_pointer] = (uint8_t)(tape[data_pointer] + inst.operand);
                program_counter++;
                break;

            case IR_MOVE: {
                int new_dp = (data_pointer + inst.operand) % TAPE_SIZE;
                if (new_dp < 0) {
                    new_dp += TAPE_SIZE;
                }
                data_pointer = new_dp;
                program_counter++;
                break;
            }

            case IR_PUT:
                putchar(tape[data_pointer]);
                program_counter++;
                break;

            case IR_GET: {
                int ch = getchar();
                if (ch != EOF) {
                    tape[data_pointer] = (uint8_t)ch;
                }
                program_counter++;
                break;
            }

            case IR_JZ:
                program_counter = (tape[data_pointer] == 0) ? inst.operand : program_counter + 1;
                break;

            case IR_JNZ:
                program_counter = (tape[data_pointer] != 0) ? inst.operand : program_counter + 1;
                break;

            case IR_CLEAR:
                tape[data_pointer] = 0;
                program_counter++;
                break;

            case IR_HALT:
                return;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename.bf>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* source_code = malloc(file_size + 1);
    if (!source_code) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t read_size = fread(source_code, 1, file_size, file);
    source_code[read_size] = '\0';
    fclose(file);

    instruction_t* prog = compile_ir(source_code);
    free(source_code);

    if (!prog) {
        return 1;
    }

#if DEBUG_TRACE_ENABLED
    print_instructions(prog);
    printf("\n");
#endif

    run_vm(prog);

    free(prog);

    return 0;
}