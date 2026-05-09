#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define TAPE_SIZE (1024 * 1024)
#define DEBUG_TRACE_ENABLED 0 // Disabled by default for normal CLI usage

#define MAX_OFFSETS 2048
#define OFFSET_CENTER 1024

typedef enum {
    IR_ADD,
    IR_MOVE,
    IR_PUT,
    IR_GET,
    IR_JZ,
    IR_JNZ,
    IR_CLEAR,
    IR_MUL,
    IR_NOP,
    IR_HALT
} ir_opcode_t;

typedef struct {
    ir_opcode_t opcode;
    int operand;
    int offset;
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
        "MUL",
        "NOP",
        "HALT"
    };

    int i = 0;
    while (instructions[i].opcode != IR_HALT) {
        printf("%04d: %-5s %d (offset: %d)\n", i, opcode_names[instructions[i].opcode], instructions[i].operand, instructions[i].offset);
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
                instructions[inst_index].offset = 0;
                inst_index++;
                break;
            }

            case '.':
            case ',':
                instructions[inst_index].opcode = (current_char == '.') ? IR_PUT : IR_GET;
                instructions[inst_index].operand = 0;
                instructions[inst_index].offset = 0;
                inst_index++;
                i++;
                break;

            case '[':
                // Optimization: Simple loop clear '[-]'
                if (source_code[i + 1] == '-' && source_code[i + 2] == ']') {
                    instructions[inst_index].opcode = IR_CLEAR;
                    instructions[inst_index].operand = 0;
                    instructions[inst_index].offset = 0;
                    inst_index++;
                    i += 3;
                    break;
                }

                bracket_stack[stack_depth++] = inst_index;
                instructions[inst_index].opcode = IR_JZ;
                instructions[inst_index].operand = -1; // Placeholder for destination
                instructions[inst_index].offset = 0;
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
                instructions[inst_index].offset = 0;
                inst_index++;
                i++;
                break;
            }

            default:
                // Ignore other unknown characters
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
    instructions[inst_index].offset = 0;

    return instructions;
}

void optimize_muls(instruction_t* prog) {
    int i = 0;
    
    while (prog[i].opcode != IR_HALT) {
        if (prog[i].opcode == IR_JZ) {
            int loop_start = i;
            int jnz_index = prog[i].operand - 1; 
            
            if (jnz_index >= 0 && prog[jnz_index].opcode == IR_JNZ) {
                int is_simple_loop = 1;
                int current_offset = 0;
                int changes[MAX_OFFSETS] = {0};

                for (int j = loop_start + 1; j < jnz_index; j++) {
                    if (prog[j].opcode == IR_MOVE) {
                        current_offset += prog[j].operand;
                    } 
                    else if (prog[j].opcode == IR_ADD) {
                        int pos = OFFSET_CENTER + current_offset;
                        if (pos >= 0 && pos < MAX_OFFSETS) {
                            changes[pos] += prog[j].operand;
                        } else {
                            is_simple_loop = 0; break; 
                        }
                    } 
                    else if (prog[j].opcode == IR_NOP) {
                        continue;
                    } 
                    else {
                        is_simple_loop = 0; break; 
                    }
                }

                if (is_simple_loop && current_offset == 0 && changes[OFFSET_CENTER] == -1) {
                    int rewrite_index = loop_start;
                    
                    for (int j = 0; j < MAX_OFFSETS; j++) {
                        if (j != OFFSET_CENTER && changes[j] != 0) {
                            prog[rewrite_index].opcode = IR_MUL;
                            prog[rewrite_index].operand = changes[j];
                            prog[rewrite_index].offset = j - OFFSET_CENTER;
                            rewrite_index++;
                        }
                    }
                    
                    prog[rewrite_index].opcode = IR_CLEAR;
                    prog[rewrite_index].operand = 0;
                    prog[rewrite_index].offset = 0;
                    rewrite_index++;

                    for (int j = rewrite_index; j <= jnz_index; j++) {
                        prog[j].opcode = IR_NOP;
                    }
                }
            }
        }
        i++;
    }
}

void run_vm(const instruction_t* instructions)
{
    uint8_t* tape = calloc(TAPE_SIZE, 1);
    if (!tape) {
        fprintf(stderr, "Failed to allocate tape\n");
        return;
    }
    int data_pointer = TAPE_SIZE / 2; // Start in the middle to prevent easy underflow
    int program_counter = 0;

    while (1) {
        instruction_t inst = instructions[program_counter];

        switch (inst.opcode) {
            case IR_ADD:
                tape[data_pointer] += (uint8_t)inst.operand;
                program_counter++;
                break;

            case IR_MOVE:
                data_pointer += inst.operand;
                program_counter++;
                break;

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

            case IR_MUL:
                tape[data_pointer + inst.offset] += tape[data_pointer] * inst.operand;
                program_counter++;
                break;

            case IR_NOP:
                program_counter++;
                break;

            case IR_HALT:
                free(tape);
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
        return EXIT_FAILURE;
    }

    optimize_muls(prog);

#if DEBUG_TRACE_ENABLED
    print_instructions(prog);
    printf("\n");
#endif

    run_vm(prog);

    free(prog);

    return EXIT_SUCCESS;
}