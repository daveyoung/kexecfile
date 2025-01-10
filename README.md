# kexecfile
simple kexec utility to use kexec_file_load syscall

Please feel free to integrate this or rewrite a similar one in kexec-tools.
It is simple because we leave all other logics to kernel, eg. arch specific
kernel image sanity checking, memory infomations collecting etc.

The other advantage is for new architectures which support kexec_file_load
this will be much easier to have a userspace support code.

For example for riscv elf kernel load with kexec_file_load, you can simply
use this one to load with a minor kernel patch
[valid by now Jan 10, 2025, probably someone will fix it later]:

diff --git a/arch/riscv/kernel/elf_kexec.c b/arch/riscv/kernel/elf_kexec.c
index 3c37661801f9..e783a72d051f 100644
--- a/arch/riscv/kernel/elf_kexec.c
+++ b/arch/riscv/kernel/elf_kexec.c
@@ -468,6 +468,9 @@ int arch_kexec_apply_relocations_add(struct purgatory_info *pi,
 		case R_RISCV_ALIGN:
 		case R_RISCV_RELAX:
 			break;
+		case R_RISCV_64:
+			*(u64 *)loc = val;
+			break;
 		default:
 			pr_err("Unknown rela relocation: %d\n", r_type);
 			return -ENOEXEC;
