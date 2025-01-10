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
https://people.redhat.com/~ruyang/riscv/diff
