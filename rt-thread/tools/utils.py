import os, sys

def splitall(loc):
	parts = []
	while loc != os.curdir and loc != os.pardir:
		prev = loc
		loc, child = os.path.split(prev)
		if loc == prev:
			break
			
		parts.append(child)
		
	parts.append(loc)
	parts.reverse()
	return parts
	
	

def _make_path_relative(origin, dest):
	origin = os.path.abspath(origin)
	dest = os.path.abspath(dest)
	
	origin_list = splitall(os.path.normcase(origin))
	dest_list = splitall(dest)
	
	if origin_list[0] != os.path.normcase(dest_list[0]):
		return dest
	
	i = 0
	for start_seg, dest_seg in zip(origin_list, dest_list):
		if start_seg != os.path.normcase(dest_seg):
			break
		i += 1
		
	segments = [os.pardir] * (len(origin_list) - i)
	segments += dest_list[i:]
	
	if len(segments) == 0:
		return os.curdir
		
	else:
		return os.path.join(*segments)
		
		