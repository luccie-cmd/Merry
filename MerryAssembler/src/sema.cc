#include "../includes/sema.hh"
#include <algorithm>

std::vector<std::string> valid_registers = {
    "Ma",
    "Mb",
    "Mc",
    "Md",
    "Me",
    "Mf",
    "M1",
    "M2",
    "M3",
    "M4",
    "M5",
    "Mm1",
    "Mm2",
    "Mm3",
    "Mm4",
    "Mm5"
};

std::vector<merry::front_end::AstInstType> first_op_has_register = {
    merry::front_end::AstInstType::ADD,
    merry::front_end::AstInstType::SUB,
    merry::front_end::AstInstType::MUL,
    merry::front_end::AstInstType::DIV,
    merry::front_end::AstInstType::MOD,
};

std::vector<std::pair<merry::front_end::AstInstType, std::size_t>> N_operands_in_inst = {
    {merry::front_end::AstInstType::NOP, 0},
    {merry::front_end::AstInstType::HLT, 0},
    {merry::front_end::AstInstType::ADD, 2},
    {merry::front_end::AstInstType::SUB, 2},
    {merry::front_end::AstInstType::MUL, 2},
    {merry::front_end::AstInstType::DIV, 2},
    {merry::front_end::AstInstType::MOD, 2},
    {merry::front_end::AstInstType::JMP, 1},
    {merry::front_end::AstInstType::CALL, 1},
    {merry::front_end::AstInstType::RET, 0},
    {merry::front_end::AstInstType::OUTR, 0},
    {merry::front_end::AstInstType::UOUTR, 0},
};

void merry::front_end::Sema::check_operand_size(merry::front_end::AstNodeInst inst){
    for(std::pair<merry::front_end::AstInstType, std::size_t> type : N_operands_in_inst){
        if(type.first == inst.get_inst_type()){
            if(type.second != inst.get_operands().size()){
                std::cout << "ERROR: Invalid number of operands expected " << std::to_string(type.second) << " But got " << std::to_string(inst.get_operands().size()) << std::endl;
            }
        }
    }
}

static int label_count = 0;
merry::front_end::AstNodeInst merry::front_end::Sema::resolve_inst_imm_or_reg(AstNodeInst inst){
    AstNodeInst copy_inst = inst;
    // Check if it's not a jump or call instruction
    if(inst.get_inst_type() != AstInstType::JMP && inst.get_inst_type() != AstInstType::CALL){
        if(copy_inst.get_operands().size() == 0){
            return inst;
        }
        if(std::find(first_op_has_register.begin(), first_op_has_register.end(), copy_inst.get_inst_type()) != first_op_has_register.end()){
            if(std::find(valid_registers.begin(), valid_registers.end(), copy_inst.get_operands().at(0)) == valid_registers.end()){
                std::cout << "Encountered an invalid register name `" << copy_inst.get_operands().at(0) << "`\n";
                exit(1);
            }
        }
        if(std::find(valid_registers.begin(), valid_registers.end(), copy_inst.get_operands().at(1)) != valid_registers.end()){
            copy_inst.is_second_reg = true;
        }
        if(copy_inst.is_second_reg){
            if(std::find(valid_registers.begin(), valid_registers.end(), copy_inst.get_operands().at(1)) == valid_registers.end()){
                std::cout << "Encountered an invalid register name `" << copy_inst.get_operands().at(1) << "`\n";
                exit(1);
            }
        }
    } else{
        unresolved_jmps.push_back({copy_ast.get_insts().size(), inst.get_operands().at(0)});
        label_count++;
    }
    insts_size++;
    return copy_inst;
}

merry::front_end::AstNodeLabel merry::front_end::Sema::resolve_label_addr(AstNodeLabel label){
    if(find_dub_label(label.get_name())){
        std::cout << "Cannot redifine label " << label.get_name() << std::endl;
        exit(1);
    }
    // Do a wierd but functioning hack to stop it from jumping not far enough
    label.set_addr((insts_size+label_count)-1);
    insts_size++;
    return label;
}