#include "types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Helper function to check if a variable is a temporary (starts with 't' followed by digits)
bool is_temporary(const char* name) {
    return name[0] == 't' && isdigit(name[1]);
}

// Функция для обработки escape-последовательностей в строке и генерации байтового массива
void emit_string_data(char* data_section, const char* input, const char* label) {
    size_t len = strlen(input);
    char* buffer = (char*)malloc(len * 2 + 1); // Достаточно места для экранирования
    size_t buf_idx = 0;

    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\' && i + 1 < len) {
            switch (input[i + 1]) {
                case 'n': buffer[buf_idx++] = 10; i++; break; // \n -> 10
                case 't': buffer[buf_idx++] = 9; i++; break;  // \t -> 9
                case 'r': buffer[buf_idx++] = 13; i++; break; // \r -> 13
                case 'b': buffer[buf_idx++] = 8; i++; break;  // \b -> 8
                case 'f': buffer[buf_idx++] = 12; i++; break; // \f -> 12
                case 'v': buffer[buf_idx++] = 11; i++; break; // \v -> 11
                case 'a': buffer[buf_idx++] = 7; i++; break;  // \a -> 7
                case '\\': buffer[buf_idx++] = 92; i++; break; // \\ -> 92
                case '"': buffer[buf_idx++] = 34; i++; break;  // \" -> 34
                case '\'': buffer[buf_idx++] = 39; i++; break; // \' -> 39
                case '0': buffer[buf_idx++] = 0; i++; break;   // \0 -> 0
                default:
                    // Неизвестная escape-последовательность, копируем как есть
                    buffer[buf_idx++] = input[i];
                    break;
            }
        } else {
            buffer[buf_idx++] = input[i];
        }
    }

    // Генерируем db директиву с байтами
    sprintf(data_section + strlen(data_section), "%s db ", label);
    for (size_t i = 0; i < buf_idx; i++) {
        sprintf(data_section + strlen(data_section), "%d", (unsigned char)buffer[i]);
        if (i < buf_idx - 1) {
            sprintf(data_section + strlen(data_section), ", ");
        }
    }
    sprintf(data_section + strlen(data_section), ", 0\n");

    free(buffer);
}

//Выравнивает размер стека так, чтобы после push rbp стек был выровнен по 16 байтам перед вызовами функций.
static int align_stack_size(int size) {
    int aligned = (size + 15) & ~15; // round up to 16
    if ((aligned % 16) == 0) {
        aligned += 8;
    }
    return aligned;
}

void codegen_layout_stack_frame(SymbolTable* locals, int* out_frame_size) {
    int offset = -8;
    for (int i = 0; i < locals->count; i++) {
        // ВСЕ переменные — по 8-байтным слотам
        locals->symbols[i].stack_offset = offset;
        offset -= 8;
    }
    int frame_size = -offset;
    *out_frame_size = align_stack_size(frame_size); // ← остаётся
}


//Генерирует пролог функции: метку, push rbp, mov rbp, rsp, sub rsp для резервирования места.
// Параметры используются напрямую из регистров, без сохранения в стек.
void emit_prologue(CodeGenContext* ctx) {
    sprintf(ctx->out + strlen(ctx->out), "%s:\n", ctx->current_function->name);
    sprintf(ctx->out + strlen(ctx->out), "    push rbp\n");
    sprintf(ctx->out + strlen(ctx->out), "    mov rbp, rsp\n");

    if (ctx->frame_size > 0) {
        sprintf(ctx->out + strlen(ctx->out), "    sub rsp, %d\n", ctx->frame_size);
    }

    // Add main_start label after prologue for main function
    if (strcmp(ctx->current_function->name, "main") == 0) {
        sprintf(ctx->out + strlen(ctx->out), "main_start:\n");
    }

    ctx->string_counter = 0;
}

//Генерирует эпилог: leave, ret.
void emit_epilogue(CodeGenContext* ctx) {
    sprintf(ctx->out + strlen(ctx->out), "%s_before_ret:\n", ctx->current_function->name);
    sprintf(ctx->out + strlen(ctx->out), "; Очистка стека и возврат\n");
    sprintf(ctx->out + strlen(ctx->out), "    leave       ; эквивалент: mov rsp, rbp; pop rbp\n");
    sprintf(ctx->out + strlen(ctx->out), "    ret         ; возвращаем eax как результат\n");
    sprintf(ctx->out + strlen(ctx->out), "%s_end:\n", ctx->current_function->name);
}

//Загружает операнд (константу или переменную) в регистр.
// Для параметров использует регистры напрямую.
void emit_load_operand(CodeGenContext* ctx, Operand* op, const char* reg) {
    if (op->kind == OPERAND_CONST) {
        if (op->data.const_val.type->kind == TYPE_INT || op->data.const_val.type->kind == TYPE_BOOL) {
            sprintf(ctx->out + strlen(ctx->out), "    mov %s, %d\n", reg, op->data.const_val.value.integer);
        } else if (op->data.const_val.type->kind == TYPE_STRING) {
            // For strings, load address of string constant with escape sequence processing
            char label[32];
            sprintf(label, "str_%d", ctx->string_counter++);
            emit_string_data(ctx->data_section, op->data.const_val.value.string, label);
            sprintf(ctx->out + strlen(ctx->out), "    lea %s, [%s]\n", reg, label);
        }
        // Примечание: для строк в lea регистр может быть rax, rcx и т.д. — это нормально
    } else if (op->kind == OPERAND_VAR) {
        // Check if it's a parameter
        for (int i = 0; i < ctx->current_function->params.count && i < 4; i++) {
            if (strcmp(ctx->current_function->params.symbols[i].name, op->data.var.name) == 0) {
                // Для параметров мы всё ещё используем 32-битные регистры из соглашения,
                // но сохраняем результат в переданный регистр `reg`
                const char* param_regs[] = {"ecx", "edx", "r8d", "r9d"};
                sprintf(ctx->out + strlen(ctx->out), "    mov %s, %s\n", reg, param_regs[i]);
                return;
            }
        }
        // It's a local variable — просто копируем из стека в указанный регистр
        int offset = get_var_offset(&ctx->local_vars, op->data.var.name);
        sprintf(ctx->out + strlen(ctx->out), "    mov %s, [rbp + %d]\n", reg, offset);
    }
}

//Сохраняет регистр в переменную по смещению.
void emit_store_to_var(CodeGenContext* ctx, const char* var_name, const char* reg) {
    int offset = get_var_offset(&ctx->local_vars, var_name);
    sprintf(ctx->out + strlen(ctx->out), "    mov [rbp + %d], %s\n", offset, reg);
}

//Находит смещение переменной по имени.
int get_var_offset(SymbolTable* locals, const char* name) {
    for (int i = 0; i < locals->count; i++) {
        if (strcmp(locals->symbols[i].name, name) == 0) {
            return locals->symbols[i].stack_offset;
        }
    }
    fprintf(stderr, "Error: Variable '%s' not found in symbol table\n", name);
    return -999; // Error offset
}

//проходит по блокам CFG, генерирует метки и инструкции для каждого IRInstruction (ASSIGN, ADD, SUB, LT, RET, JUMP, COND_BR).
void asm_build_from_cfg(char* out, FunctionInfo* func_info, SymbolTable* locals, CFG* cfg, FunctionTable* local_funcs)
{
    // Generate extern declarations for used functions
    for (int i = 0; i < local_funcs->count; i++) {
        FunctionInfo* func = local_funcs->functions[i];
        if (func) {
            sprintf(out + strlen(out), "extern %s\n", func->name);
        }
    }
    sprintf(out + strlen(out), "\n");

    int frame_size;

    codegen_layout_stack_frame(locals, &frame_size);

    CodeGenContext ctx = {0}; // обнуляет ВСЮ структуру, включая data_section
    ctx.out = out;
    ctx.current_function = func_info;
    ctx.local_vars = *locals;
    ctx.frame_size = frame_size;
    ctx.string_counter = 0;
    ctx.debug_count = 0;
    memset(ctx.seen_lines, 0, sizeof(ctx.seen_lines));
    // ctx.data_section уже заполнен нулями — strlen() = 0, всё безопасно

    emit_prologue(&ctx);

    printf("DEBUG: cfg->num_blocks = %zu\n", cfg->num_blocks);

    // Traverse blocks
    for (size_t i = 0; i < cfg->num_blocks; i++) {
        BasicBlock* block = &cfg->blocks[i];
        printf("DEBUG: Block %zu: id=%s, num_instructions=%zu\n", i, block->id, block->num_instructions);
        sprintf(ctx.out + strlen(ctx.out), "%s:\n", block->id);

        for (size_t j = 0; j < block->num_instructions; j++) {
            IRInstruction* inst = &block->instructions[j];
            // Emit line label if this is a new line number
            if (inst->line_number > 0 && !ctx.seen_lines[inst->line_number]) {
                sprintf(ctx.out + strlen(ctx.out), "line_%d:\n", inst->line_number);
                ctx.seen_lines[inst->line_number] = true;
                ctx.debug_lines[ctx.debug_count++] = inst->line_number;
            }
            switch (inst->opcode) {
            case IR_ASSIGN:
                Operand* value = &inst->data.assign.value;
                const char* target = inst->data.assign.target;
                bool is_string = false;

                if (value->kind == OPERAND_CONST) {
                    is_string = (value->data.const_val.type->kind == TYPE_STRING);
                } else if (value->kind == OPERAND_VAR) {
                    Symbol* sym = symbol_table_lookup(&ctx.local_vars, value->data.var.name);
                    is_string = (sym && sym->type->kind == TYPE_STRING);
                }

                if (is_string) {
                    emit_load_operand(&ctx, value, "rax");
                    emit_store_to_var(&ctx, target, "rax");
                } else {
                    emit_load_operand(&ctx, value, "eax");
                    emit_store_to_var(&ctx, target, "eax");
                }
                break;
            case IR_ADD:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    add eax, ebx\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_SUB:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    sub eax, ebx\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_MUL:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    imul eax, ebx\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_DIV:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cdq\n");
                sprintf(ctx.out + strlen(ctx.out), "    idiv ebx\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_EQ:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, ebx\n");
                sprintf(ctx.out + strlen(ctx.out), "    sete al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_NE:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, ebx\n");
                sprintf(ctx.out + strlen(ctx.out), "    setne al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_LT:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, ebx\n");
                sprintf(ctx.out + strlen(ctx.out), "    setl al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_LE:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, ebx\n");
                sprintf(ctx.out + strlen(ctx.out), "    setle al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_GT:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, ebx\n");
                sprintf(ctx.out + strlen(ctx.out), "    setg al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_GE:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, ebx\n");
                sprintf(ctx.out + strlen(ctx.out), "    setge al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_AND:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    and eax, ebx\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_OR:
                emit_load_operand(&ctx, &inst->data.compute.operands[0], "eax");
                emit_load_operand(&ctx, &inst->data.compute.operands[1], "ebx");
                sprintf(ctx.out + strlen(ctx.out), "    or eax, ebx\n");
                emit_store_to_var(&ctx, inst->data.compute.result, "eax");
                break;
            case IR_NOT:
                emit_load_operand(&ctx, &inst->data.unary.operand, "eax");
                sprintf(ctx.out + strlen(ctx.out), "    test eax, eax\n");
                sprintf(ctx.out + strlen(ctx.out), "    setz al\n");
                sprintf(ctx.out + strlen(ctx.out), "    movzx eax, al\n");
                emit_store_to_var(&ctx, inst->data.unary.result, "eax");
                break;
            case IR_NEG:
                emit_load_operand(&ctx, &inst->data.unary.operand, "eax");
                sprintf(ctx.out + strlen(ctx.out), "    neg eax\n");
                emit_store_to_var(&ctx, inst->data.unary.result, "eax");
                break;
            case IR_POS:
                // Unary plus does nothing, just assign
                emit_load_operand(&ctx, &inst->data.unary.operand, "eax");
                emit_store_to_var(&ctx, inst->data.unary.result, "eax");
                break;
            case IR_BIT_NOT:
                emit_load_operand(&ctx, &inst->data.unary.operand, "eax");
                sprintf(ctx.out + strlen(ctx.out), "    not eax\n");
                emit_store_to_var(&ctx, inst->data.unary.result, "eax");
                break;
            case IR_CALL:
                // Microsoft x64 calling convention: pass args in rcx, rdx, r8, r9
                // Use 32-bit registers for int/bool types, 64-bit for strings
                const char* arg_regs_32[] = {"ecx", "edx", "r8d", "r9d"};
                const char* arg_regs_64[] = {"rcx", "rdx", "r8", "r9"};
                for (int k = 0; k < inst->data.call.num_args && k < 4; k++) {
                    if (inst->data.call.args[k].kind == OPERAND_VAR &&
                        symbol_table_lookup(&ctx.local_vars, inst->data.call.args[k].data.var.name) &&
                        symbol_table_lookup(&ctx.local_vars, inst->data.call.args[k].data.var.name)->type->kind == TYPE_STRING) {
                        // String variable - load 64-bit address
                        int offset = get_var_offset(&ctx.local_vars, inst->data.call.args[k].data.var.name);
                        sprintf(ctx.out + strlen(ctx.out), "    mov %s, [rbp + %d]\n", arg_regs_64[k], offset);
                        } else if (inst->data.call.args[k].kind == OPERAND_CONST &&
                            inst->data.call.args[k].data.const_val.type->kind == TYPE_STRING) {
                            emit_load_operand(&ctx, &inst->data.call.args[k], arg_regs_64[k]);
                            } else {
                                emit_load_operand(&ctx, &inst->data.call.args[k], arg_regs_32[k]);
                            }
                }
                sprintf(ctx.out + strlen(ctx.out), "    sub rsp, 32\n");
                sprintf(ctx.out + strlen(ctx.out), "    call %s\n", inst->data.call.func_name);
                sprintf(ctx.out + strlen(ctx.out), "    add rsp, 32\n");
                if (strcmp(inst->data.call.result, "") != 0) {
                    emit_store_to_var(&ctx, inst->data.call.result, "eax");
                }
                break;
            case IR_JUMP:
                sprintf(ctx.out + strlen(ctx.out), "    jmp %s\n", inst->data.jump.target);
                break;
            case IR_COND_BR:
                emit_load_operand(&ctx, &inst->data.cond_br.condition, "eax");
                sprintf(ctx.out + strlen(ctx.out), "    cmp eax, 0\n");
                sprintf(ctx.out + strlen(ctx.out), "    jne %s\n", inst->data.cond_br.true_target);
                sprintf(ctx.out + strlen(ctx.out), "    jmp %s\n", inst->data.cond_br.false_target);
                break;
            case IR_RET:
                if (inst->data.ret.has_value) {
                    Operand* value = &inst->data.ret.value;
                    bool is_string = false;

                    if (value->kind == OPERAND_CONST) {
                        is_string = (value->data.const_val.type->kind == TYPE_STRING);
                    } else if (value->kind == OPERAND_VAR) {
                        Symbol* sym = symbol_table_lookup(&ctx.local_vars, value->data.var.name);
                        is_string = (sym && sym->type->kind == TYPE_STRING);
                    }

                    if (is_string) {
                        emit_load_operand(&ctx, value, "rax");
                        // Значение уже в rax — ничего дополнительно делать не нужно
                    } else {
                        emit_load_operand(&ctx, value, "eax");
                        // Значение уже в eax — корректно для возврата int/bool
                    }
                }
                emit_epilogue(&ctx);
                break;
            }
        }
    }
    // No extra epilogue for main, as it's handled in IR_RET

    // Collect unique non-temporary variable names
    char unique_names[100][32];
    int unique_count = 0;
    for (int i = 0; i < locals->count; i++) {
        Symbol* sym = &locals->symbols[i];
        if (!is_temporary(sym->name)) {
            bool found = false;
            for (int j = 0; j < unique_count; j++) {
                if (strcmp(unique_names[j], sym->name) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                strcpy(unique_names[unique_count++], sym->name);
            }
        }
    }

    // Add debug strings to debug_str section
    sprintf(ctx.debug_str_section + strlen(ctx.debug_str_section), "dbg_str_%s db '%s', 0\n", func_info->name, func_info->name);
    for (int i = 0; i < unique_count; i++) {
        sprintf(ctx.debug_str_section + strlen(ctx.debug_str_section), "dbg_str_%s db '%s', 0\n", unique_names[i], unique_names[i]);
    }

    // Append rodata section if there are strings
    if (strlen(ctx.data_section) > 0) {
        sprintf(out + strlen(out), "\nsection .rodata\n");
        sprintf(out + strlen(out), "%s", ctx.data_section);
    }

    // Append debug_str section
    if (strlen(ctx.debug_str_section) > 0) {
        sprintf(out + strlen(out), "\nsection .debug_str\n");
        sprintf(out + strlen(out), "%s", ctx.debug_str_section);
    }

    // Generate debug info
    sprintf(out + strlen(out), "\nsection .debug_info\n");
    sprintf(out + strlen(out), "    ; === Функция %s ===\n", func_info->name);
    sprintf(out + strlen(out), "    dq dbg_str_%s                 ; указатель на имя\n", func_info->name);
    if (strcmp(func_info->name, "main") == 0) {
        sprintf(out + strlen(out), "    dq main_start                   ; Реальный адрес начала кода (для отладчика)\n");
    } else {
        sprintf(out + strlen(out), "    dq %s                         ; старт\n", func_info->name);
    }
    sprintf(out + strlen(out), "    dq %s_end                     ; конец\n", func_info->name);
    //sprintf(out + strlen(out), "    dd 0                          ; параметров: 0\n");
    sprintf(out + strlen(out), "    dd %d                         ; локальных: %d\n", unique_count, unique_count);

    for (int i = 0; i < unique_count; i++) {
        // Find the first symbol with this name
        Symbol* sym = NULL;
        for (int j = 0; j < locals->count; j++) {
            if (strcmp(locals->symbols[j].name, unique_names[i]) == 0) {
                sym = &locals->symbols[j];
                break;
            }
        }
        if (sym) {
            sprintf(out + strlen(out), "    ; Переменная %s\n", sym->name);
            sprintf(out + strlen(out), "    dq dbg_str_%s                    ; имя\n", sym->name);
            int type_code = (sym->type->kind == TYPE_STRING) ? 1 : 0; // 0 - int, 1 - string
            sprintf(out + strlen(out), "    dd %d                            ; тип: %s\n", type_code, (type_code == 1) ? "string" : "int");
            sprintf(out + strlen(out), "    dd %d                           ; смещение\n", sym->stack_offset);
        }
    }

    // Generate debug_line section
    sprintf(out + strlen(out), "\nsection .debug_line\n");
    for (int i = 0; i < ctx.debug_count; i++) {
        sprintf(out + strlen(out), "dq line_%d\n", ctx.debug_lines[i]);
        sprintf(out + strlen(out), "dq %d\n", ctx.debug_lines[i]);
    }
    sprintf(out + strlen(out), "dq %s_before_ret\n", func_info->name);
    sprintf(out + strlen(out), "dq %d\n", ctx.debug_lines[ctx.debug_count-1]+1);
    sprintf(out + strlen(out), "dq 0, 0 ; Конец таблицы\n");
}
