#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_PROGRAM_LENGTH 30000
#define MAX_DEPTH 1000


#define PRINT_IR 1


typedef enum {
    INC              = '+',
    DEC              = '-',
    RIGHT            = '>',
    LEFT             = '<',
    PRINT            = '.',
    READ             = ',',
    JUMP_IF_ZERO     = '[',
    JUMP_IF_NOT_ZERO = ']',
    HALT             = '\0',
} opcode_t;


typedef struct {
    opcode_t op;
    int arg;
} Instruction;

int compress(const char* program, Instruction* out, int max_out)
{
    int count = 0;

    for (int i = 0; program[i] != HALT;)
    {
        opcode_t op = (opcode_t)program[i];

        if (op == INC || op == DEC || op == RIGHT || op == LEFT)
        {
            int arg = 1;
            i++;

            while (program[i] == op)
            {
                arg++;
                i++;
            }

            if (count >= max_out)
            {
                return -1;
            }

            out[count].op = op;
            out[count].arg = arg;
            count++;
        }
        else if (op == PRINT || op == READ || op == JUMP_IF_ZERO || op == JUMP_IF_NOT_ZERO)
        {
            if (count >= max_out)
            {
                return -1;
            }

            out[count].op = op;
            out[count].arg = 1;
            count++;
            i++;
        }
        else
        {
            i++;
        }
    }

    return count;
}

void print_instructions(const Instruction* instructions, int instruction_count)
{
    for (int i = 0; i < instruction_count; i++)
    {
        //if (instructions[i].arg <= 1)
        //{
        //    printf("%c ", instructions[i].op);
        //}
        //else
        //{
        //    printf("%c%d ", instructions[i].op, instructions[i].arg);
        //}

        printf("%c%d\n", instructions[i].op, instructions[i].arg);

    }
}

int build_matches(const char* program, int* match, int program_length)
{
    int opens[MAX_DEPTH];
    int depth = 0;

    for (int i = 0; i < program_length; i++)
    {
        match[i] = -1;
    }

    for (int i = 0; i < program_length; i++)
    {
        if (program[i] == JUMP_IF_ZERO)
        {
            if (depth >= MAX_DEPTH)
            {
                printf("Error: bracket nesting too deep\n");
                return 0;
            }

            opens[depth] = i;
            depth++;
        }
        else if (program[i] == JUMP_IF_NOT_ZERO)
        {
            if (depth == 0)
            {
                printf("Error: unmatched ] at %d\n", i);
                return 0;
            }

            depth--;
            match[i] = opens[depth];
            match[opens[depth]] = i;
        }
    }

    if (depth != 0)
    {
        printf("Error: unmatched [ at %d\n", opens[depth - 1]);
        return 0;
    }

    return 1;
}

void run(const char* program)
{
    uint8_t tape[30000] = {0};
    int match[MAX_PROGRAM_LENGTH];
    int dp = 0;
    int pc = 0;
    int program_length = (int)strlen(program);

    if (program_length >= MAX_PROGRAM_LENGTH)
    {
        printf("Error: program too long\n");
        return;
    }

    if (!build_matches(program, match, program_length))
    {
        return;
    }

    while (program[pc] != HALT)
    {
        switch ((opcode_t)program[pc])
        {
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
            tape[dp] = (uint8_t)getchar();
            pc++;
            break;
        case JUMP_IF_ZERO:
            if (tape[dp] == 0)
            {
                pc = match[pc] + 1;
            }
            else
            {
                pc++;
            }
            break;
        case JUMP_IF_NOT_ZERO:
            if (tape[dp] != 0)
            {
                pc = match[pc] + 1;
            }
            else
            {
                pc++;
            }
            break;
        default:
            pc++;
            break;
        }
    }
}

int main()
{
    const char multiply_loop[] = "++++++++[>+++++++++<-]>.";
    const char hello_world[] = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
    Instruction instructions[MAX_PROGRAM_LENGTH];
    int instruction_count = compress(multiply_loop, instructions, MAX_PROGRAM_LENGTH);

    if (instruction_count < 0)
    {
        printf("Error: compressed program too long\n");
        return 1;
    }

    #if PRINT_IR
        print_instructions(instructions, instruction_count);
        printf("\n");
    #endif

    run(multiply_loop);
    printf("\n");
    run(hello_world);

    return 0;
}
