import os, shutil

from shutil import ignore_patterns

def do_copy_file(src,dst):
	if not os.path.exists(src):
		return
	
	path = os.path.dirname(dst)
	if not os.path.exists(path):
		os.makedirs(path)
		
	shutil.copy2(src, dst)
	
def do_copy_folder(src_dir, dst_dir, ignore=None):
	if not os.path.exists(src_dir):
		return
		
	try:
		if os.path.exists(dst_dir):
			shutil.rmtree(dst_dir)
	except:
		print("delete folder:%s failed" % dst_dir)
		return
	shutil.copytree(src_dir, dst_dir, ignore=ignore)
	
source_ext = ["c", "h", "s", "S", "cpp", "xpm"]
source_list = []

def walk_children(child):
	global source_ext
	global source_list
	
	
# def MakeCopy(program, BSP_ROOT, RTT_ROOT, Env):
	# global source_list
	# target_path = os.path.join(BSP_ROOT, "rt-thread")
	
	# for item in program:
		