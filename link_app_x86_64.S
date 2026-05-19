	.p2align 4
 	.section .data.app
 	.global _num_app
 _num_app:
	.quad 16
	.quad test_mremap_simple_start
	.quad test_mremap_simple_end
	.quad oscomp_munmap_start
	.quad oscomp_munmap_end
	.quad test_signal_delivery_start
	.quad test_signal_delivery_end
	.quad test_phase2b_signal_basic_start
	.quad test_phase2b_signal_basic_end
	.quad test_brk_init_start
	.quad test_brk_init_end
	.quad test_sigaltstack_start
	.quad test_sigaltstack_end
	.quad ch2b_exit_start
	.quad ch2b_exit_end
	.quad test_mprotect_simple_start
	.quad test_mprotect_simple_end
	.quad test_mmap_simple_start
	.quad test_mmap_simple_end
	.quad oscomp_brk_start
	.quad oscomp_brk_end
	.quad test_phase2a_clone_start
	.quad test_phase2a_clone_end
	.quad oscomp_mmap_start
	.quad oscomp_mmap_end
	.quad test_phase1_start
	.quad test_phase1_end
	.quad oscomp_getpid_start
	.quad oscomp_getpid_end
	.quad test_sig_dfl_start
	.quad test_sig_dfl_end
	.quad test_fork_wait_start
	.quad test_fork_wait_end
	.p2align 4
	.section .user
 	.global test_mremap_simple_start
 	.global test_mremap_simple_end
test_mremap_simple_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_mremap_simple"
test_mremap_simple_end:
	.p2align 4
	.section .user
 	.global oscomp_munmap_start
 	.global oscomp_munmap_end
oscomp_munmap_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/oscomp_munmap"
oscomp_munmap_end:
	.p2align 4
	.section .user
 	.global test_signal_delivery_start
 	.global test_signal_delivery_end
test_signal_delivery_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_signal_delivery"
test_signal_delivery_end:
	.p2align 4
	.section .user
 	.global test_phase2b_signal_basic_start
 	.global test_phase2b_signal_basic_end
test_phase2b_signal_basic_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_phase2b_signal_basic"
test_phase2b_signal_basic_end:
	.p2align 4
	.section .user
 	.global test_brk_init_start
 	.global test_brk_init_end
test_brk_init_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_brk_init"
test_brk_init_end:
	.p2align 4
	.section .user
 	.global test_sigaltstack_start
 	.global test_sigaltstack_end
test_sigaltstack_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_sigaltstack"
test_sigaltstack_end:
	.p2align 4
	.section .user
 	.global ch2b_exit_start
 	.global ch2b_exit_end
ch2b_exit_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/ch2b_exit"
ch2b_exit_end:
	.p2align 4
	.section .user
 	.global test_mprotect_simple_start
 	.global test_mprotect_simple_end
test_mprotect_simple_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_mprotect_simple"
test_mprotect_simple_end:
	.p2align 4
	.section .user
 	.global test_mmap_simple_start
 	.global test_mmap_simple_end
test_mmap_simple_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_mmap_simple"
test_mmap_simple_end:
	.p2align 4
	.section .user
 	.global oscomp_brk_start
 	.global oscomp_brk_end
oscomp_brk_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/oscomp_brk"
oscomp_brk_end:
	.p2align 4
	.section .user
 	.global test_phase2a_clone_start
 	.global test_phase2a_clone_end
test_phase2a_clone_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_phase2a_clone"
test_phase2a_clone_end:
	.p2align 4
	.section .user
 	.global oscomp_mmap_start
 	.global oscomp_mmap_end
oscomp_mmap_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/oscomp_mmap"
oscomp_mmap_end:
	.p2align 4
	.section .user
 	.global test_phase1_start
 	.global test_phase1_end
test_phase1_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_phase1"
test_phase1_end:
	.p2align 4
	.section .user
 	.global oscomp_getpid_start
 	.global oscomp_getpid_end
oscomp_getpid_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/oscomp_getpid"
oscomp_getpid_end:
	.p2align 4
	.section .user
 	.global test_sig_dfl_start
 	.global test_sig_dfl_end
test_sig_dfl_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_sig_dfl"
test_sig_dfl_end:
	.p2align 4
	.section .user
 	.global test_fork_wait_start
 	.global test_fork_wait_end
test_fork_wait_start:
	.incbin "/Users/zhiyuansue/Desktop/RendezvOS_Linux/user_payload/user/build/aarch64/test_fork_wait"
test_fork_wait_end:
