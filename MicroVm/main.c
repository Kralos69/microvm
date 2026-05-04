#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_PROGRAM_LENGTH 30000
#define MAX_DEPTH 1000

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

    run(multiply_loop);
    printf("\n");
    run(hello_world);

    return 0;
}
