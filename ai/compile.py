#!/usr/bin/env python3
import os
import zipapp
from stat import S_IEXEC

s = os.path.dirname(os.path.abspath(__file__))
t = os.path.join(s, os.pardir, "zappy_ai")

zipapp.create_archive(
    s,
    target=t,
    interpreter="/usr/bin/env python3",
    main="zappy_ai:main",
)
os.chmod(t, S_IEXEC | os.stat(t).st_mode)
