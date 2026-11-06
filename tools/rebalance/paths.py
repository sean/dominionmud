"""Where things are.  The repository root is two levels up from this file
(override with DOMINION_ROOT); intermediate files go in REBALANCE_WORK
(default: <tmp>/dominion-rebalance)."""
import os, subprocess, tempfile

REPO = os.path.abspath(os.environ.get('DOMINION_ROOT') or
                       os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
LIB = os.path.join(REPO, 'lib')
WORLD = os.path.join(LIB, 'world')
SRC = os.path.join(REPO, 'src')
WORK = os.environ.get('REBALANCE_WORK') or os.path.join(tempfile.gettempdir(), 'dominion-rebalance')
os.makedirs(WORK, exist_ok=True)

ROLES = os.path.join(WORK, 'roles.json')        # vnum -> [role, level, party, zone]

def work(name):
    return os.path.join(WORK, name)

def level_dump(name):
    """Run 'bin/dominion -c' with DOMINION_LEVEL_DUMP: the server's own item
    levels (src/levels.c) and shops.  Returns the dump's path."""
    out = work(name)
    binary = os.path.join(REPO, 'bin', 'dominion')
    if not os.path.exists(binary):
        raise SystemExit(f'{binary} not found: build the server first (make in src/)')
    env = dict(os.environ, DOMINION_LEVEL_DUMP=out)
    r = subprocess.run([binary, '-c'], cwd=REPO, env=env, capture_output=True, text=True)
    if not os.path.exists(out):
        raise SystemExit('bin/dominion -c wrote no level dump:\n' + (r.stdout + r.stderr)[-2000:])
    return out
