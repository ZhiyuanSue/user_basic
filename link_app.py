# generate an asm file and use that to incbin all the bin files

import sys
import os

link_app_file_name="link_app.S"

def generate_link_app(pwd,elf_dir,elf_file_list):
	link_app_s_file = os.path.join(pwd,link_app_file_name)
	manager_str="\t.p2align 4\n \
\t.section .data.app\n \
\t.global _num_app\n \
_num_app:\n"

	section_str=""
	manager_str += "\t.quad " + str(len(elf_file_list)) + "\n"
	for file in elf_file_list:
		inc_bin_file = os.path.join(pwd,elf_dir,file)
		manager_str += "\t.quad " + file + "_start\n" + "\t.quad " + file + "_end\n"
		section_str += "\t.p2align 4\n\t.section .user\n \
\t.global " + file + "_start\n \
\t.global " + file + "_end\n" \
+ file + "_start:\n" \
+ "\t.incbin \""+ inc_bin_file +"\"\n" \
+ file + "_end:\n"
	
	with open(link_app_s_file,"w") as file:
		file.write(manager_str)
		file.write(section_str)

	pass

if __name__ =='__main__':
	elf_dir = sys.argv[1]
	pwd = os.getcwd()
	elf_files = []
	if os.path.exists(elf_dir):
		all_items = os.listdir(elf_dir)
		elf_files = [f for f in all_items if os.path.isfile(os.path.join(elf_dir, f))]
	generate_link_app(pwd,elf_dir,elf_files)
	pass