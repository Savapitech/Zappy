#!/usr/bin/env python3
import py_compile
import os
from stat import S_IEXEC

file_path = py_compile.compile("ia/zappy_ai.py", "zappy_ai", "../")
os.chmod(file_path, S_IEXEC | os.stat(file_path).st_mode)